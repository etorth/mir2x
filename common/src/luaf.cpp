#include "luaf.hpp"

size_t luaf::_details::_luaVarWrapperHash::operator () (const luaVarWrapper &wrapper) const noexcept
{
    return std::visit(luaVarDispatcher
    {
        [](const luaNil &) -> size_t
        {
            return 2918357;
        },

        [](const luaTable &table) -> size_t // requires order-invariant
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

std::string luaf::quotedLuaString(const std::string &s)
{
    // following two shall print identical result:
    //
    //    in c++: std::puts(s)
    //    in lua: print(luaf::quotedLuaString(s))
    //
    // use this to pass compound structures as string to lua

    if(s.find('[') == std::string::npos && s.find(']') == std::string::npos){
        return str_printf("[[%s]]", s.c_str());
    }

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

sol::object luaf::buildLuaObj(sol::state_view sv, luaf::luaNil)
{
    return sol::make_object(sv, sol::nil);
}

sol::object luaf::buildLuaObj(sol::state_view sv, luaf::luaVar v)
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
