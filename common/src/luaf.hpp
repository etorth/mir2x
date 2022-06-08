#pragma once
#include <string>
#include <cstddef>
#include <variant>
#include <unordered_map>
#include <type_traits>
#include <sol/sol.hpp>
#include "totype.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"

// c++ internal types <----> blob <----> lua types as sol::object
//    int                std::string       sol::object
//    bool                                 sol::as_table_t<...>
//    double
//    std::string
//    std::unordered_map

namespace luaf
{
    namespace _details
    {
        using conv_table = std::unordered_map<std::string, std::string>;
    }

    template<typename T> constexpr char getBlobType()
    {
        if constexpr(std::is_same_v<T, sol::nil_t>){
            return 'n';
        }
        else if constexpr(std::is_same_v<T, int>){
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
        else if(obj.is<int>()){
            return buildBlob<int>(obj.as<int>());
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
            _details::conv_table convtbl;
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
            case getBlobType<int>():
                {
                    return sol::object(sv, sol::in_place_type<int>, cerealf::deserialize<int>(std::move(s)));
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
                    for(auto &[kstr, vstr]: cerealf::deserialize<_details::conv_table>(s)){
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
}
