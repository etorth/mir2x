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

    using variable   = std::variant<luaf::nil, int, bool, double, std::string>;
    using table      = std::map<luaf::variable, luaf::variable>;
    using conv_table = std::map<std::string, std::string>; // string-to-string-table used to serialize/deserialize general lua table

    template<typename T> constexpr char getBlobType()
    {
        if constexpr(std::is_same_v<T, luaf::nil>){
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
        else if constexpr(std::is_same_v<T, std::map<std::string, std::string>>){
            return 't';
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

    // TODO
    // remove when sol2 fixed the issue
    inline auto buildLuaConvTable(std::string s)
    {
        fflassert(s.length() >= 2);
        fflassert(s.back() == getBlobType<luaf::conv_table>());

        s.pop_back();
        auto convTable = cerealf::deserialize<luaf::conv_table>(s);
        luaf::conv_table convTableTransf;

        for(auto &p: convTable){
            std::string ts;
            ts.reserve(p.first.size() * 2);

            for(const char ch: p.first){
                ts.push_back(((ch & 0XF0) >> 4) + '0');
                ts.push_back(((ch & 0X0F) >> 0) + '0');
            }
            convTableTransf[ts] = p.second;
        }

        return sol::as_table_t<luaf::conv_table>(std::move(convTableTransf));
    }

    // inline auto buildLuaConvTable(std::string s)
    // {
    //     fflassert(s.length() >= 2);
    //     fflassert(s.back() == getBlobType<luaf::conv_table>());
    //
    //     s.pop_back();
    //     auto convTable = cerealf::deserialize<luaf::conv_table>(s);
    //     return sol::as_table_t<luaf::conv_table>(std::move(convTable));
    // }
}
