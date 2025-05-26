#include <limits>
#include "luaf.hpp"
// put strf.h after luaf.h
// this guarantees all defs of operator << () functions seen before str_any(const T &)
#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

size_t luaf::_details::_luaVarWrapperHash::operator () (const luaVarWrapper &wrapper) const noexcept
{
    return std::visit(luaVarDispatcher
    {
        [](const luaNil &) -> size_t
        {
            return 2918357;
        },

        [](const luaArray &array) -> size_t // has order dependency
        {
            size_t h = array.size();
            for(const auto &v: array) {
                h ^= _details::_luaVarWrapperHash{}(v) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        },

        [](const luaTable &table) -> size_t // requires order-invariant
        {
            size_t h = table.size();
            for(const auto &[k, v]: table){
                h ^= _details::_luaVarWrapperHash{}(k);
                h ^= _details::_luaVarWrapperHash{}(v);
            }
            return h;
        },

        [](const auto &t) -> size_t
        {
            return std::hash<std::remove_cvref_t<decltype(t)>>{}(t);
        },
    }, *wrapper.m_ptr);
}

bool luaf::_details::isArray(const sol::table &table)
{
    lua_Integer cnt = 0;
    lua_Integer min = std::numeric_limits<lua_Integer>::max();
    lua_Integer max = std::numeric_limits<lua_Integer>::min();

    for(const auto &[k, v]: table){
        if(k.is<lua_Integer>()){
            min = std::min(min, k.as<lua_Integer>());
            max = std::max(max, k.as<lua_Integer>());
        }
        else{
            return false;
        }
        cnt++;
    }

    return (min == 1) && (max == cnt);
}

bool luaf::_details::isArray(const luaf::luaTable &table)
{
    if(table.empty()){
        return true;
    }

    lua_Integer min = std::numeric_limits<lua_Integer>::max();
    lua_Integer max = std::numeric_limits<lua_Integer>::min();

    for(const auto &[k, v]: table){
        if(k == luaf::luaNil()){
            throw fflerror("invalid luaTable key: nil");
        }

        else if(const auto p = std::get_if<lua_Integer>(std::addressof(k.get()))){
            min = std::min(min, *p);
            max = std::max(max, *p);
        }

        else{
            return false;
        }
    }

    return (min == 1) && (to_uz(max) == table.size());
}

luaf::luaVarWrapper::luaVarWrapper(const luaf::luaVarWrapper &w)
    : m_ptr(w.m_ptr ? std::make_unique<luaVar>(*w.m_ptr) : nullptr)
{}

luaf::luaVarWrapper::luaVarWrapper(luaf::luaVarWrapper &&w)
    : m_ptr(std::move(w.m_ptr))
{}

luaf::luaVarWrapper & luaf::luaVarWrapper::operator = (luaf::luaVarWrapper w)
{
    std::swap(m_ptr, w.m_ptr);
    return *this;
}
bool luaf::luaVarWrapper::operator == (const luaf::luaVarWrapper &parm) const
{
    if(m_ptr){
        return get() == parm.get();
    }
    else{
        return parm.m_ptr == nullptr; // luaNil
    }
}

bool luaf::luaVarWrapper::operator == (const luaf::luaVar &parm) const
{
    if(m_ptr){
        return get() == parm;
    }
    else{
        return parm.index() == 0; // luaNil
    }
}

bool luaf::luaVarWrapper::operator == (const luaf::luaNil &) const
{
    return m_ptr == nullptr || m_ptr->index() == 0;
}

std::string luaf::quotedLuaString(const std::string &s)
{
    // following two shall print identical result:
    //
    //    in c++: std::puts(s)
    //    in lua: print(luaf::quotedLuaString(s))
    //
    // use this to pass compound structures as string to lua

    // don't use double square bracket
    // because this function is used for asInitString() which supports table
    //
    //     local u = any_func()
    //     local s = string.format("[%s]=%s", asInitString(u))
    //
    // this messes up because the key-value pair also uses square bracket

    // if(s.find('[') == std::string::npos && s.find(']') == std::string::npos){
    //     return str_printf("[[%s]]", s.c_str());
    // }

    std::string result;
    result.reserve(s.size() + 32);

    result += "\'";
    for(const auto si: s){
        switch(si){
            case '\'': result += "\\\'"    ; break;
            case '\\': result += "\\\\"    ; break;
            default  : result.push_back(si); break;
        }
    }

    result += "\'";
    return result;
}

std::string luaf::luaObjTypeString(const sol::object &obj)
{
    return sol::type_name(obj.lua_state(), obj.get_type());
}

sol::object luaf::buildLuaObj(sol::state_view sv, luaf::luaNil)
{
    return sol::make_object(sv, sol::nil);
}

sol::object luaf::buildLuaObj(sol::state_view sv, luaf::luaVar v)
{
    return std::visit(luaf::luaVarDispatcher
    {
        [&sv](const luaf::luaArray &a) -> sol::object
        {
            sol::table tbl(sv.lua_state(), sol::create);
            for(lua_Integer i = 1; const auto &v: a){
                tbl[i++] = luaf::buildLuaObj(sv, v);
            }
            return tbl; // sol::table can be used as sol::object
        },

        [&sv](const luaf::luaTable &t) -> sol::object
        {
            sol::table tbl(sv.lua_state(), sol::create);
            for(const auto &[k, v]: t){
                tbl[luaf::buildLuaObj(sv, k)] = luaf::buildLuaObj(sv, v);
            }
            return tbl; // sol::table can be used as sol::object
        },

        [&sv](const auto &v) -> sol::object
        {
            return luaf::buildLuaObj(sv, v);
        }
    }, v);
}

sol::object luaf::buildLuaObj(sol::state_view sv, luaf::luaVarWrapper w)
{
    return luaf::buildLuaObj(sv, luaf::luaVar(std::move(w)));
}

sol::object luaf::buildLuaObj(sol::state_view sv, lua_Integer v)
{
    return sol::object(sv, sol::in_place_type<lua_Integer>, v);
}

sol::object luaf::buildLuaObj(sol::state_view sv, double v)
{
    return sol::object(sv, sol::in_place_type<double>, v);
}

sol::object luaf::buildLuaObj(sol::state_view sv, bool v)
{
    return sol::object(sv, sol::in_place_type<bool>, v);
}

sol::object luaf::buildLuaObj(sol::state_view sv, std::string v)
{
    return sol::object(sv, sol::in_place_type<std::string>, std::move(v));
}

luaf::luaVar luaf::buildLuaVar(luaf::luaVarWrapper w)
{
    if(w == luaf::luaNil()){
        return luaf::luaNil();
    }
    else{
        return luaf::luaVar(std::move(w.get()));
    }
}

luaf::luaVar luaf::buildLuaVar(const sol::object &obj)
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
        if(obj.as<sol::table>()[sol::metatable_key] != sol::nil){
            throw fflerror("can't build from table with metatable");
        }

        if(_details::isArray(obj.as<sol::table>())){
            luaf::luaArray array;
            for(const auto &[k, v]: obj.as<sol::table>()){
                const auto i = to_uz(k.as<lua_Integer>());
                if(array.size() < i){
                    array.resize(i);
                }
                array[i - 1] = luaf::buildLuaVar(sol::object(v));
            }
            return array;
        }
        else{
            luaf::luaTable table;
            for(const auto &[k, v]: obj.as<sol::table>()){
                if(const auto [p, added] = table.insert_or_assign(luaf::buildLuaVar(sol::object(k)), luaf::buildLuaVar(sol::object(v))); !added){
                    // this is possible, lua table doesn't have equal-by-value defined
                    // following table is legal in lua:
                    //
                    //     local t = {
                    //         [{}] = 1,
                    //         [{}] = 1,
                    //     }
                    //
                    // which creates table t with two table-keys with same value, but different address
                    // I can use sol::table::pointer() to differ them by adding an extra key to LuaTable to store the address, but looks doesn't make much sense
                    throw fflerror("duplicated key detected: %s", str_any(luaf::buildLuaVar(sol::object(k))).c_str());
                }
            }
            return table;
        }
    }
    else{
        throw fflerror("unsupported type: %s", to_cstr(sol::type_name(obj.lua_state(), obj.get_type())));
    }
}

template<typename T> static std::vector<luaf::luaVar> _buildLuaVarFromLuaObjContainer(const T &args, size_t varSize, size_t begin, const std::optional<size_t> &endOpt)
{
    fflassert(begin <= varSize, begin, varSize);
    if(endOpt.has_value()){
        fflassert(endOpt.value() <= varSize, endOpt.value(), varSize);
    }

    if(begin == varSize){
        return {};
    }

    std::vector<luaf::luaVar> result;
    result.reserve(endOpt.value_or(varSize) - begin);

    for(auto i = begin; i < endOpt.value_or(varSize); ++i){
        result.push_back(luaf::buildLuaVar(sol::object(args[i])));
    }
    return result;
}

std::vector<luaf::luaVar> luaf::vargBuildLuaVarList(const sol::variadic_args &args, size_t begin, std::optional<size_t> end)
{
    fflassert(args.lua_state());
    return _buildLuaVarFromLuaObjContainer(args, args.size(), begin, end);
}

std::vector<luaf::luaVar> luaf::pfrBuildLuaVarList(const sol::protected_function_result &pfr, size_t begin, std::optional<size_t> end)
{
    fflassert(pfr.valid());
    return _buildLuaVarFromLuaObjContainer(pfr, pfr.return_count(), begin, end);
}

std::ostream & operator << (std::ostream &os, const sol::object &obj)
{
    return os << str_any(luaf::buildLuaVar(obj));
}

std::ostream & operator << (std::ostream &os, const sol::stack_proxy &proxy)
{
    return os << str_any(luaf::buildLuaVar(sol::object(proxy)));
}

std::ostream & operator << (std::ostream &os, const sol::variadic_args &args)
{
    return os << str_any(luaf::vargBuildLuaVarList(args));
}

std::ostream & operator << (std::ostream &os, const sol::protected_function_result &args)
{
    return os << str_any(luaf::pfrBuildLuaVarList(args));
}

std::ostream & operator << (std::ostream &os, const luaf::luaNil &)
{
    return os << "(luanil)";
}

std::ostream & operator << (std::ostream &os, const luaf::luaVarWrapper &wrapper)
{
    return os << str_any(luaf::luaVar(wrapper));
}
