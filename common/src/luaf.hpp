#pragma once
#include <memory>
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

// c++ internal types <----> luaVar <----> lua types as sol::object
//    lua_Integer         std::variant     sol::object
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

    struct luaNil
    {
        char placeholder = 0;

        bool operator == (const luaNil &) const
        {
            return true;
        }

        template<typename Archive> void serialize(Archive & ar)
        {
            ar(placeholder);
        }
    };

    class luaVarWrapper;
    struct _luaVarWrapperHash
    {
        inline size_t operator() (const luaVarWrapper &) const noexcept;
    };

    using luaTable = std::unordered_map<luaVarWrapper, luaVarWrapper, _luaVarWrapperHash>;

    using luaVar = std::variant<
        luaNil, // default initialized as nil
        luaTable,

        bool,
        double,
        lua_Integer,
        std::string>;

    // luaVarWrapper should behave exactly same as luaVar
    // but it helps to get rid of type dependency, C++ can not recursively define types

    class luaVarWrapper
    {
        private:
            friend struct _luaVarWrapperHash;

        private:
            std::unique_ptr<luaVar> m_ptr; // underlaying variable never share

        public:
            template<typename T> luaVarWrapper(T t)
                : m_ptr(std::make_unique<luaVar>(std::move(t)))
            {}

        public:
            luaVarWrapper() = default;  // default initialized as luaNil

        public:
            luaVarWrapper(luaNil)       // no need to allocate memory if holding luaNil
                : luaVarWrapper()
            {}

        public:
            luaVarWrapper(luaVar v): m_ptr(std::visit(luaVarDispatcher
            {
                [](luaNil) -> std::unique_ptr<luaVar>
                {
                    return nullptr;
                },

                [](auto &&arg) -> std::unique_ptr<luaVar>
                {
                    return std::make_unique<luaVar>(std::move(arg));
                },
            }, std::move(v))){}

        public:
            luaVarWrapper(const luaVarWrapper &w)
                : m_ptr(w.m_ptr ? std::make_unique<luaVar>(*w.m_ptr) : nullptr)
                {}

            luaVarWrapper(luaVarWrapper &&w)
                : m_ptr(std::move(w.m_ptr))
            {}

            luaVarWrapper &operator = (luaVarWrapper w)
            {
                std::swap(m_ptr, w.m_ptr);
                return *this;
            }

        public:
            operator luaVar () const
            {
                if(m_ptr){
                    return *m_ptr;
                }
                else{
                    return luaNil{};
                }
            }

        public:
            /* */ luaVar &get()       { return *m_ptr; }
            const luaVar &get() const { return *m_ptr; }

        public:
            bool operator == (const luaVarWrapper &parm) const
            {
                if(m_ptr){
                    return get() == parm.get();
                }
                else{
                    return parm.m_ptr == nullptr; // luaNil
                }
            }

            bool operator == (const luaVar &parm) const
            {
                if(m_ptr){
                    return get() == parm;
                }
                else{
                    return parm.index() == 0; // luaNil
                }
            }

        public:
            std::string str() const
            {
                return std::visit([](const auto &v) -> std::string { return str_any(v); }, *m_ptr);
            }

        public:
            template<typename Archive> void serialize(Archive & ar)
            {
                ar(m_ptr);
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

    template<typename T> sol::object buildLuaObj(sol::state_view &sv, T t)
    {
        return sol::object(sv, sol::in_place_type<std::remove_cvref_t<decltype(t)>>, std::move(t));
    }

    inline sol::object buildLuaObj(sol::state_view &sv, luaNil)
    {
        return sol::make_object(sv, sol::nil);
    }

    inline sol::object buildLuaObj(sol::state_view &sv, luaVar v)
    {
        return std::visit(luaVarDispatcher
        {
            [&sv](const luaTable &t) -> sol::object
            {
                sol::table tbl(sv.lua_state(), sol::create);
                for(const auto &[k, v]: t){
                    tbl[buildLuaObj(sv, k)] = buildLuaObj(sv, v);
                }
                return tbl; // sol::table can be used as sol::object
            },

            [&sv](const auto &v) -> sol::object
            {
                return buildLuaObj(sv, v);
            }
        }, v);
    }

    template<typename T> luaVar buildLuaVar(T t)
    {
        return luaVar(std::move(t));
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

    inline std::vector<luaVar> pfrBuildLuaVarList(const sol::protected_function_result &pfr)
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
