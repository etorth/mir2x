#pragma once
#include <memory>
#include <string>
#include <ostream>
#include <cstddef>
#include <variant>
#include <tuple>
#include <optional>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <array>
#include <vector>
#include <deque>
#include <list>
#include <type_traits>
#include <stdexcept>
#include <sol/sol.hpp>
#include "stdf.hpp"

// c++ internal types <----> luaVar <----> lua types as sol::object
//    lua_Integer         std::variant     sol::object
//    bool                                 sol::as_table_t<...>
//    double
//    std::string
//    std::unordered_map

namespace luaf
{
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
    using luaArray = std::vector<luaVarWrapper>;
    using luaTable = std::unordered_map<luaVarWrapper, luaVarWrapper, _details::_luaVarWrapperHash>; // TODO using incomplete type, is it UB ?

    using luaVar = std::variant<
        // default initialized as nil
        luaNil,

        luaArray, // optional
        luaTable,

        // try integer then decimal
        // lua integer is always a number
        lua_Integer,
        double,
        bool,
        std::string>;
}

namespace luaf
{
    bool isArray(const  sol::table    &);
    bool isArray(const luaf::luaTable &);
}

namespace luaf
{
    // luaVarWrapper behaves exactly same as luaVar
    // use it to avoid type dependency, C++ can not recursively define types
    class luaVarWrapper final
    {
        private:
            friend struct _details::_luaVarWrapperHash;

        private:
            std::unique_ptr<luaVar> m_ptr; // don't share underlaying variable

        public:
            /**/  luaVarWrapper() = default; // default initialized as luaNil
            /**/ ~luaVarWrapper() = default;

        public:
            template<typename T> luaVarWrapper(T t)
                : m_ptr(std::make_unique<luaVar>(std::move(t)))
            {}

        public:
            luaVarWrapper(luaNil) // no need to allocate memory if holding luaNil
                : luaVarWrapper()
            {}

        public:
            luaVarWrapper(luaVar v): m_ptr(std::visit(stdf::VarDispatcher
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
            template<typename Archive> void serialize(Archive & ar)
            {
                ar(m_ptr);
            }
    };
}

namespace luaf
{
    std::ostream &operator << (std::ostream &, const luaNil &);
    std::ostream &operator << (std::ostream &, const luaVarWrapper &);
}

std::ostream & operator << (std::ostream &, const sol::object &);
std::ostream & operator << (std::ostream &, const sol::stack_proxy &);
std::ostream & operator << (std::ostream &, const sol::variadic_args &);
std::ostream & operator << (std::ostream &, const sol::protected_function_result &);

#include "fflerror.hpp"

namespace luaf
{
    std::string quotedLuaString(const std::string &);
    std::string luaObjTypeString(const sol::object &);
}

namespace luaf
{
    // sol is overly flexible to create sol::object
    // don't use generic template
    //
    //   template<typename T> sol::object buildLuaObj(sol::state_view, T)
    //
    // implement all possible types explicitly instead

    sol::object buildLuaObj(sol::state_view, luaNil);
    sol::object buildLuaObj(sol::state_view, luaVar);
    sol::object buildLuaObj(sol::state_view, luaVarWrapper);

    sol::object buildLuaObj(sol::state_view sv, lua_Integer);
    sol::object buildLuaObj(sol::state_view sv, double);
    sol::object buildLuaObj(sol::state_view sv, bool);
    sol::object buildLuaObj(sol::state_view sv, std::string);
}

namespace luaf
{
    template<typename T> luaVar buildLuaVar(T);

    luaVar buildLuaVar(luaVarWrapper);
    luaVar buildLuaVar(const sol::object &);

    template<typename K, typename V, typename... Args> luaVar buildLuaVar(std::unordered_map<K, V, Args...>);
    template<typename K, typename V, typename... Args> luaVar buildLuaVar(std::          map<K, V, Args...>);

    template<typename T, typename... Args> luaVar buildLuaVar(std::         list<T, Args...>);
    template<typename T, typename... Args> luaVar buildLuaVar(std::       vector<T, Args...>);
    template<typename T, typename... Args> luaVar buildLuaVar(std::          set<T, Args...>);
    template<typename T, typename... Args> luaVar buildLuaVar(std::unordered_set<T, Args...>);

    template<typename T, size_t N> luaVar buildLuaVar(T (&)[N]);
    template<typename T, size_t N> luaVar buildLuaVar(std::array<T, N>);
    template<typename T          > luaVar buildLuaVar(std::initializer_list<T>);

    template<typename T> luaVar buildLuaVar(std::optional<T>);
    template<typename... Ts> luaVar buildLuaVar(const std::tuple<Ts...> &);

    template<typename C> luaArray buildLuaArray(C varList)
    {
        luaArray array;
        array.reserve(std::size(varList));

        for(auto &v: varList){
            array.emplace_back(luaVarWrapper(buildLuaVar(std::move(v))));
        }
        return array;
    }

    template<typename C> luaTable buildLuaTable(C varTable)
    {
        luaTable table;
        for(auto &[k, v]: varTable){
            table.emplace(luaVarWrapper(buildLuaVar(std::move(k))), luaVarWrapper(buildLuaVar(std::move(v))));
        }
        return table;
    }

    template<typename T> luaVar buildLuaVar(T t)
    {
        return luaVar(std::move(t));
    }

    template<typename K, typename V, typename... Args> luaVar buildLuaVar(std::unordered_map<K, V, Args...> varTable) { return buildLuaTable(varTable); }
    template<typename K, typename V, typename... Args> luaVar buildLuaVar(std::          map<K, V, Args...> varTable) { return buildLuaTable(varTable); }

    template<typename T, typename... Args> luaVar buildLuaVar(std::         list<T, Args...> varList) { return buildLuaArray(varList); }
    template<typename T, typename... Args> luaVar buildLuaVar(std::       vector<T, Args...> varList) { return buildLuaArray(varList); }
    template<typename T, typename... Args> luaVar buildLuaVar(std::        deque<T, Args...> varList) { return buildLuaArray(varList); }
    template<typename T, typename... Args> luaVar buildLuaVar(std::          set<T, Args...> varList) { return buildLuaArray(varList); }
    template<typename T, typename... Args> luaVar buildLuaVar(std::unordered_set<T, Args...> varList) { return buildLuaArray(varList); }

    template<typename T, size_t N> luaVar buildLuaVar(T (&varList)[N])                  { return buildLuaArray(varList); }
    template<typename T, size_t N> luaVar buildLuaVar(std::array<T, N> varList)         { return buildLuaArray(varList); }
    template<typename T          > luaVar buildLuaVar(std::initializer_list<T> varList) { return buildLuaArray(varList); }

    template<typename T> luaVar buildLuaVar(std::optional<T> varOpt)
    {
        if(varOpt.has_value()){
            return buildLuaVar(std::move(varOpt.value()));
        }
        else{
            return luaNil{};
        }
    }

    template<typename... Ts> luaVar buildLuaVar(const std::tuple<Ts...> &t)
    {
        return std::apply([](const auto &... elem) -> luaArray
        {
            luaArray array;
            array.reserve(sizeof...(Ts));
            (array.emplace_back(luaVarWrapper(buildLuaVar(elem))), ...);
            return array;
        }, t);
    }
}

namespace luaf
{
    std::vector<luaVar> vargBuildLuaVarList(const sol::variadic_args             &, size_t = 0, std::optional<size_t> = std::nullopt);
    std::vector<luaVar>  pfrBuildLuaVarList(const sol::protected_function_result &, size_t = 0, std::optional<size_t> = std::nullopt);
}

namespace luaf
{
    namespace _details
    {
        template<typename T> struct _luaVarAsImpl
        {
            static T call(const luaVar &var)
            {
                if constexpr(std::is_integral_v<T>){
                    return static_cast<T>(std::get<lua_Integer>(var));
                }
                else if constexpr(std::is_floating_point_v<T>){
                    return static_cast<T>(std::get<double>(var));
                }
                else if constexpr(std::is_same_v<T, const char *>){
                    return std::get<std::string>(var).c_str();
                }
                else if constexpr(std::is_same_v<T, std::string_view>){
                    return std::get<std::string>(var);
                }
                else if constexpr(std::is_same_v<T, luaf::luaVar>){
                    static_assert(stdf::always_false<T>::value, "only convert to C/C++ types");
                }
                else{
                    static_assert(stdf::always_false<T>::value, "invalid type for conversion");
                }
            }
        };
    }

    // decode a luaVar back into a concrete c++ type T
    // caller is assumed to know the exact shape T that the luaVar holds, invalid shape/type throws

    template<typename T> T luaVarAs(const luaVar &var)
    {
        return _details::_luaVarAsImpl<T>::call(var);
    }

    namespace _details
    {
        template<typename C> C _luaVarAsSequence(const luaVar &var)
        {
            const auto &arr = std::get<luaArray>(var);

            C result;
            for(const auto &elem: arr){
                result.emplace_back(luaVarAs<typename C::value_type>(elem.get()));
            }
            return result;
        }

        template<typename M> M _luaVarAsMap(const luaVar &var)
        {
            M result;
            if(const auto arr = std::get_if<luaArray>(std::addressof(var))){
                for(size_t i = 0; const auto &elem: *arr){
                    using KeyType = typename M::key_type;
                    using ValType = typename M::mapped_type;

                    if(!result.emplace(luaVarAs<KeyType>(luaVar(static_cast<lua_Integer>(++i))), luaVarAs<ValType>(elem.get())).second){
                        throw fflerror("luaVarAs<std::map>: duplicated key");
                    }
                }
                return result;
            }

            const auto &table = std::get<luaTable>(var);
            for(const auto &[key, value]: table){
                using KeyType = typename M::key_type;
                using ValType = typename M::mapped_type;
                if(!result.emplace(luaVarAs<KeyType>(key.get()), luaVarAs<ValType>(value.get())).second){
                    throw fflerror("luaVarAs<std::map>: duplicated key");
                }
            }
            return result;
        }

        template<> struct _luaVarAsImpl<lua_Integer>
        {
            static lua_Integer call(const luaVar &);
        };

        template<> struct _luaVarAsImpl<double>
        {
            static double call(const luaVar &);
        };

        template<> struct _luaVarAsImpl<bool>
        {
            static bool call(const luaVar &);
        };

        template<> struct _luaVarAsImpl<std::string>
        {
            static std::string call(const luaVar &);
        };

        template<typename T, typename... Args> struct _luaVarAsImpl<std::list<T, Args...>>
        {
            static std::list<T, Args...> call(const luaVar &var)
            {
                return _luaVarAsSequence<std::list<T, Args...>>(var);
            }
        };

        template<typename T, typename... Args> struct _luaVarAsImpl<std::vector<T, Args...>>
        {
            static std::vector<T, Args...> call(const luaVar &var)
            {
                return _luaVarAsSequence<std::vector<T, Args...>>(var);
            }
        };

        template<typename K, typename V, typename... Args> struct _luaVarAsImpl<std::map<K, V, Args...>>
        {
            static std::map<K, V, Args...> call(const luaVar &var)
            {
                return _luaVarAsMap<std::map<K, V, Args...>>(var);
            }
        };

        template<typename K, typename V, typename... Args> struct _luaVarAsImpl<std::unordered_map<K, V, Args...>>
        {
            static std::unordered_map<K, V, Args...> call(const luaVar &var)
            {
                return _luaVarAsMap<std::unordered_map<K, V, Args...>>(var);
            }
        };

        template<typename... Ts> struct _luaVarAsImpl<std::tuple<Ts...>>
        {
            static std::tuple<Ts...> call(const luaVar &var)
            {
                using TupleType = std::tuple<Ts...>;
                const auto &arr = std::get<luaArray>(var);

                if(arr.size() != sizeof...(Ts)){
                    throw fflerror("luaVarAs<std::tuple>: size mismatch");
                }

                return [&]<size_t... Is>(std::index_sequence<Is...>) -> TupleType
                {
                    return TupleType{luaVarAs<std::tuple_element_t<Is, TupleType>>(arr[Is].get())...};
                }
                (std::make_index_sequence<sizeof...(Ts)>{});
            }
        };

        template<typename T, size_t N> struct _luaVarAsImpl<std::array<T, N>>
        {
            static std::array<T, N> call(const luaVar &var)
            {
                const auto &arr = std::get<luaArray>(var);
                if(arr.size() != N){
                    throw fflerror("luaVarAs<std::array>: size mismatch");
                }

                std::array<T, N> result;
                for(size_t i = 0; i < N; ++i){
                    result[i] = luaVarAs<T>(arr[i].get());
                }
                return result;
            }
        };
    }
}
