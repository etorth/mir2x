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
}

namespace luaf
{
    class luaVarWrapper;
    namespace _details
    {
        struct _luaVarWrapperHash
        {
            size_t operator() (const luaVarWrapper &) const noexcept;
        };
    }
}

namespace luaf
{
    class luaVarWrapper;
    using luaTable = std::unordered_map<luaVarWrapper, luaVarWrapper, _details::_luaVarWrapperHash>; // TODO using incomplete type, is it UB ?

    using luaVar = std::variant<
        luaNil, // default initialized as nil
        luaTable,

        bool,
        double,
        lua_Integer,
        std::string>;

    // luaVarWrapper should behave exactly same as luaVar
    // but it helps to get rid of type dependency, C++ can not recursively define types
}

namespace luaf
{
    class luaVarWrapper
    {
        private:
            friend struct _details::_luaVarWrapperHash;

        private:
            std::unique_ptr<luaVar> m_ptr; // don't share underlaying variable

        public:
            /**/  luaVarWrapper() = default;  // default initialized as luaNil
            /**/ ~luaVarWrapper() = default;

        public:
            template<typename T> luaVarWrapper(T t)
                : m_ptr(std::make_unique<luaVar>(std::move(t)))
            {}

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
            luaVarWrapper              (const luaVarWrapper & );
            luaVarWrapper              (      luaVarWrapper &&);
            luaVarWrapper & operator = (      luaVarWrapper   );

        public:
            operator luaVar () const
            {
                if(m_ptr){
                    return *m_ptr; // coping
                }
                else{
                    return luaNil{};
                }
            }

        public:
            /* */ luaVar &get()       { return *m_ptr; }
            const luaVar &get() const { return *m_ptr; }

        public:
            bool operator == (const luaVarWrapper &) const;
            bool operator == (const luaVar        &) const;
            bool operator == (const luaNil        &) const;

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
}

namespace luaf
{
    template<typename T> sol::object buildLuaObj(sol::state_view &sv, T t)
    {
        return sol::object(sv, sol::in_place_type<std::remove_cvref_t<decltype(t)>>, std::move(t));
    }

    sol::object buildLuaObj(sol::state_view &, luaNil);
    sol::object buildLuaObj(sol::state_view &, luaVar);


    template<typename T> luaVar buildLuaVar(T t)
    {
        return luaVar(std::move(t));
    }

    luaVar buildLuaVar(luaVarWrapper);
    luaVar buildLuaVar(const sol::object &);

    std::vector<luaVar> pfrBuildLuaVarList(const sol::protected_function_result &pfr);
}

inline std::ostream & operator << (std::ostream &os, const luaf::luaVarWrapper &wrapper)
{
    return os << wrapper.str();
}

inline std::ostream & operator << (std::ostream &os, const luaf::luaNil &)
{
    return os << "(luanil)";
}
