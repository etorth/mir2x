/*
 * =====================================================================================
 *
 *       Filename: luaf.hpp
 *        Created: 04/22/2021 20:24:34
 *    Description:
 *
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <string>
#include <type_traits>
#include <sol/sol.hpp>
#include "cerealf.hpp"
#include "fflerror.hpp"

namespace luaf
{
    template<typename T> constexpr char getBlobTypeChar()
    {
        if constexpr(std::is_same_v<T, int>){
            return 'i';
        }
        else if constexpr(std::is_same_v<T, double>){
            return 'f';
        }
        else if constexpr(std::is_same_v<T, std::string>){
            return 't';
        }
        else if constexpr(std::is_same_v<T, sol::lua_nil_t>){
            return 'n';
        }
        else{
            throw fflerror("no blob type char defined for given type");
        }
    }

    template<typename T> std::string buildSQLBlob(T t)
    {
        auto s = cerealf::serialize<T>(std::move(t));
        s.push_back(getBlobTypeChar<T>());
        return s;
    }

    template<> std::string buildSQLBlob<sol::lua_nil_t>(sol::lua_nil_t)
    {
        return std::string("nn");
    }

    template<> std::string buildSQLBlob<sol::object>(sol::object obj)
    {
        if(obj == sol::lua_nil){
            return std::string("nn");
        }
        else if(obj.is<int>()){
            return buildSQLBlob<int>(obj.as<int>());
        }
        else if(obj.is<double>()){
            return buildSQLBlob<double>(obj.as<double>());
        }
        else if(obj.is<std::string>()){
            return buildSQLBlob<std::string>(obj.as<std::string>());
        }
        else{
            throw fflerror("invalid object type");
        }
    }

    inline sol::object buildLuaObj(sol::state_view sv, std::string s)
    {
        // string created by cerealf::serialize()
        // [0, n-2] data
        // [   n-1] compression
        // [   n  ] type information
        fflassert(s.length() >= 2);
        const char ch = s.back();
        s.pop_back();

        switch(ch){
            case getBlobTypeChar<int           >(): return sol::object(sv, sol::in_place_type<int        >, cerealf::deserialize<int        >(std::move(s)));
            case getBlobTypeChar<double        >(): return sol::object(sv, sol::in_place_type<double     >, cerealf::deserialize<double     >(std::move(s)));
            case getBlobTypeChar<std::string   >(): return sol::object(sv, sol::in_place_type<std::string>, cerealf::deserialize<std::string>(std::move(s)));
            case getBlobTypeChar<sol::lua_nil_t>(): return sol::make_object(sv, sol::nil);
            default: throw fflerror("invalid type char: %c", ch);
        }
    }
}
