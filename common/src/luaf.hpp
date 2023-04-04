#pragma once
#include <string>
#include <ostream>
#include <cstddef>
#include <variant>
#include <unordered_map>
#include <type_traits>
#include <sol/sol.hpp>
#include "strf.hpp"
#include "totype.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"

// c++ internal types <----> blob <----> lua types as sol::object
//    lua_Integer        std::string       sol::object
//    bool                                 sol::as_table_t<...>
//    double
//    std::string
//    std::unordered_map

namespace luaf
{
    template<typename... Ts> struct luaVarDispatcher: Ts...
    {
        using Ts::operator()...;
    };

    class luaVarWrapper;
    struct _luaVarWrapperHash
    {
        inline size_t operator() (const luaVarWrapper &) const noexcept;
    };

    struct luaNil
    {
        bool operator == (const luaNil &) const
        {
            return true;
        }
    };

    using luaTable = std::unordered_map<luaVarWrapper, luaVarWrapper, _luaVarWrapperHash>;

    using luaVar = std::variant<
        bool,
        double,
        lua_Integer,
        std::string,

        luaNil,
        luaTable
    >;

    class luaVarWrapper
    {
        private:
            friend struct _luaVarWrapperHash;

        private:
            std::shared_ptr<luaVar> m_ptr;

        public:
            template<typename T> luaVarWrapper(T t)
                : m_ptr(std::make_shared<luaVar>(std::move(t)))
            {}

        public:
            bool operator == (const luaVarWrapper &parm) const
            {
                if(m_ptr && parm.m_ptr){
                    return *m_ptr == *parm.m_ptr;
                }
                else{
                    return !m_ptr && !parm.m_ptr;
                }
            }

        public:
            std::string str() const
            {
                return std::visit([](const auto &v) -> std::string { return str_any(v); }, *m_ptr);
            }
    };

    size_t _luaVarWrapperHash::operator() (const luaVarWrapper &wrapper) const noexcept
    {
        return std::visit(luaVarDispatcher
        {
            [](const luaNil &) -> size_t
            {
                return 291835;
            },

            [](const luaTable &table) -> size_t
            {
                if(table.empty()){
                    return 0;
                }
                else{
                    size_t h = 0;
                    for(const auto &[k, v]: table){
                        h ^= _luaVarWrapperHash{}(k);
                        h ^= _luaVarWrapperHash{}(v);
                    }
                    return h;
                }
            },

            [](const auto &t) -> size_t
            {
                return std::hash<std::remove_cvref_t<decltype(t)>>{}(t);
            },
        }, *wrapper.m_ptr);
    }

    template<typename T> constexpr char getBlobType()
    {
        if constexpr(std::is_same_v<T, sol::nil_t>){
            return 'n';
        }
        else if constexpr(std::is_same_v<T, lua_Integer>){
            return 'i';
        }
        if constexpr(std::is_same_v<T, bool>){
            return 'b';
        }
        else if constexpr(std::is_same_v<T, double>){
            return 'f';
        }
        else if constexpr(std::is_same_v<T, std::string>){
            return 's';
        }
        else if constexpr(std::is_same_v<T, sol::table>){
            return 't';
        }
        else{
            throw fflerror("unsupported type");
        }
    }

    template<typename T> std::string buildBlob(const T &t)
    {
        auto s = cerealf::serialize<T>(t);
        s.push_back(getBlobType<T>());
        return s;
    }

    template<> inline std::string buildBlob<sol::nil_t>(const sol::nil_t &)
    {
        return std::string("nn");
    }

    template<> inline std::string buildBlob<sol::object>(const sol::object &obj)
    {
        if(obj == sol::nil){
            return buildBlob<sol::nil_t>(sol::nil);
        }
        else if(obj.is<lua_Integer>()){
            return buildBlob<lua_Integer>(obj.as<lua_Integer>());
        }
        else if(obj.is<bool>()){
            return buildBlob<bool>(obj.as<bool>());
        }
        else if(obj.is<double>()){
            return buildBlob<double>(obj.as<double>());
        }
        else if(obj.is<std::string>()){
            return buildBlob<std::string>(obj.as<std::string>());
        }
        else if(obj.is<sol::table>()){
            std::unordered_map<std::string, std::string> convtbl;
            for(const auto &[k, v]: obj.as<sol::table>()){
                convtbl[buildBlob<sol::object>(k)] = buildBlob<sol::object>(v);
            }

            auto s = cerealf::serialize(convtbl);
            s.push_back(getBlobType<sol::table>());
            return s;
        }
        else{
            throw fflerror("unsupported type: %s", to_cstr(sol::type_name(obj.lua_state(), obj.get_type())));
        }
    }

    // pfr is not a table nor any lua type object
    // it's a list contains several type object to support multiple-return syntax
    inline std::vector<std::string> pfrBuildBlobList(const sol::protected_function_result &pfr)
    {
        fflassert(pfr.valid());
        if(pfr.return_count() == 0){
            return {};
        }

        std::vector<std::string> result;
        result.reserve(pfr.return_count());

        for(const auto &r: pfr){
            result.push_back(buildBlob<sol::object>(r));
        }
        return result;
    }

    inline sol::object buildLuaObj(sol::state_view &sv, std::string s)
    {
        // string created by cerealf::serialize()
        // [0, n-2] data
        // [   n-1] compression
        // [   n  ] type information
        fflassert(s.length() >= 2, s, s.length());
        const char ch = s.back();
        s.pop_back();

        switch(ch){
            case getBlobType<sol::nil_t>():
                {
                    return sol::make_object(sv, sol::nil);
                }
            case getBlobType<lua_Integer>():
                {
                    return sol::object(sv, sol::in_place_type<lua_Integer>, cerealf::deserialize<lua_Integer>(std::move(s)));
                }
            case getBlobType<bool>():
                {
                    return sol::object(sv, sol::in_place_type<bool>, cerealf::deserialize<bool>(std::move(s)));
                }
            case getBlobType<double>():
                {
                    return sol::object(sv, sol::in_place_type<double>, cerealf::deserialize<double>(std::move(s)));
                }
            case getBlobType<std::string>():
                {
                    return sol::object(sv, sol::in_place_type<std::string>, cerealf::deserialize<std::string>(std::move(s)));
                }
            case getBlobType<sol::table>():
                {
                    sol::table tbl(sv.lua_state(), sol::create);
                    for(auto &[kstr, vstr]: cerealf::deserialize<std::unordered_map<std::string, std::string>>(s)){
                        tbl[buildLuaObj(sv, std::move(kstr))] = buildLuaObj(sv, std::move(vstr));
                    }
                    return tbl; // sol::table can be used as sol::object
                }
            default:
                {
                    throw fflerror("unsupported blob type: %c", ch);
                }
        }
    }

    inline luaVar buildLuaVar(const sol::object &obj)
    {
        if(obj == sol::nil){
            return luaNil();
        }
        else if(obj.is<lua_Integer>()){
            return obj.as<lua_Integer>();
        }
        else if(obj.is<bool>()){
            return obj.as<bool>();
        }
        else if(obj.is<double>()){
            return obj.as<double>();
        }
        else if(obj.is<std::string>()){
            return obj.as<std::string>();
        }
        else if(obj.is<sol::table>()){
            luaTable table;
            for(const auto &[k, v]: obj.as<sol::table>()){
                table.insert_or_assign(buildLuaVar(k), buildLuaVar(v));
            }
            return table;
        }
        else{
            throw fflerror("unsupported type: %s", to_cstr(sol::type_name(obj.lua_state(), obj.get_type())));
        }
    }

    inline std::vector<luaVar> pfrBuildVarList(const sol::protected_function_result &pfr)
    {
        fflassert(pfr.valid());
        if(pfr.return_count() == 0){
            return {};
        }

        std::vector<luaVar> result;
        result.reserve(pfr.return_count());

        for(const auto &r: pfr){
            result.push_back(buildLuaVar(r));
        }
        return result;
    }
}

inline std::ostream & operator << (std::ostream &os, const luaf::luaVarWrapper &wrapper)
{
    return os << wrapper.str();
}

inline std::ostream & operator << (std::ostream &os, const luaf::luaNil &)
{
    return os << "(luanil)";
}
