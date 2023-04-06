#include "luaf.hpp"

size_t luaf::_details::_luaVarWrapperHash::operator () (const luaVarWrapper &wrapper) const noexcept
{
    return std::visit(luaVarDispatcher
    {
        [](const luaNil &) -> size_t
        {
            return 2918357;
        },

        [](const luaTable &table) -> size_t
        {
            size_t h = 3679231;
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

sol::object luaf::buildLuaObj(sol::state_view &sv, luaf::luaNil)
{
    return sol::make_object(sv, sol::nil);
}

sol::object luaf::buildLuaObj(sol::state_view &sv, luaf::luaVar v)
{
    return std::visit(luaf::luaVarDispatcher
    {
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
        luaf::luaTable table;
        for(const auto &[k, v]: obj.as<sol::table>()){
            table.insert_or_assign(luaf::buildLuaVar(sol::object(k)), luaf::buildLuaVar(sol::object(v)));
        }
        return table;
    }
    else{
        throw fflerror("unsupported type: %s", to_cstr(sol::type_name(obj.lua_state(), obj.get_type())));
    }
}

std::vector<luaf::luaVar> luaf::pfrBuildLuaVarList(const sol::protected_function_result &pfr)
{
    fflassert(pfr.valid());
    if(pfr.return_count() == 0){
        return {};
    }

    std::vector<luaf::luaVar> result;
    result.reserve(pfr.return_count());

    for(const auto &r: pfr){
        result.push_back(luaf::buildLuaVar(sol::object(r)));
    }
    return result;
}
