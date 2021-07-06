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
#include <cstddef>
#include <variant>
#include <unordered_map>
#include <type_traits>
#include <sol/sol.hpp>
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
    // define an distinct nil type
    // cereal doesn't support std::nullptr_t
    struct nil
    {
        char unused = 0;
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(unused);
        }

        bool operator == (const luaf::nil &) const
        {
            return true;
        }

        bool operator < (const luaf::nil &) const
        {
            return false;
        }
    };

    // in lua the key can also be int, bool double etc
    // but sol::object doesn't define less operator which makes it can't be key of std::map

    using variable = std::variant<luaf::nil, int, bool, double, std::string>;
    using table    = std::unordered_map<std::string, luaf::variable>;

    template<typename T> constexpr char getBlobType()
    {
        if constexpr(std::is_same_v<T, int>){
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
        else if constexpr(std::is_same_v<T, luaf::table>){
            return 't';
        }
        else if constexpr(std::is_same_v<T, luaf::nil>){
            return 'n';
        }
        else{
            throw fflerror("no blob type char defined for given type");
        }
    }

    template<typename T> std::string buildBlob(T t)
    {
        auto s = cerealf::serialize<T>(std::move(t));
        s.push_back(getBlobType<T>());
        return s;
    }

    template<> inline std::string buildBlob<luaf::nil>(luaf::nil)
    {
        return std::string("nn");
    }

    template<> inline std::string buildBlob<sol::object>(sol::object obj)
    {
        if(obj == sol::lua_nil){
            return std::string("nn");
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
            case getBlobType<luaf::nil  >(): return sol::make_object(sv, sol::nil);
            case getBlobType<int        >(): return sol::object(sv, sol::in_place_type<int        >, cerealf::deserialize<int        >(std::move(s)));
            case getBlobType<bool       >(): return sol::object(sv, sol::in_place_type<bool       >, cerealf::deserialize<bool       >(std::move(s)));
            case getBlobType<double     >(): return sol::object(sv, sol::in_place_type<double     >, cerealf::deserialize<double     >(std::move(s)));
            case getBlobType<std::string>(): return sol::object(sv, sol::in_place_type<std::string>, cerealf::deserialize<std::string>(std::move(s)));
            default: throw fflerror("invalid blob type: %c", ch);
        }
    }

    inline auto buildLuaTable(sol::state_view sv, std::string s)
    {
        const auto fnConv = [&sv](const auto &var) -> sol::object
        {
            /**/ if(std::get_if<luaf::nil     >(&var)) return sol::make_object(sv, sol::nil);
            else if(std::get_if<int           >(&var)) return sol::object(sv, sol::in_place_type<int        >, std::get<int        >(var));
            else if(std::get_if<bool          >(&var)) return sol::object(sv, sol::in_place_type<bool       >, std::get<bool       >(var));
            else if(std::get_if<double        >(&var)) return sol::object(sv, sol::in_place_type<double     >, std::get<double     >(var));
            else if(std::get_if<std::string   >(&var)) return sol::object(sv, sol::in_place_type<std::string>, std::get<std::string>(var));
            else throw fflerror("invalid blob type");
        };

        std::map<std::string, sol::object> luaTable;
        const auto cppTable = cerealf::deserialize<luaf::table>(s);

        for(const auto &[key, value]: cppTable){
            luaTable[key] = fnConv(value);
        }
        return sol::as_table_t<decltype(luaTable)>(std::move(luaTable));
    }
}
