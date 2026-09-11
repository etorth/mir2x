#pragma once
#include <functional>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>

namespace stdf
{
    template<typename T> struct always_false: std::false_type {};

    template<typename T> concept TriviallyCopyable = std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>;

    // overload pattern: build a callable from a set of lambdas/functors, used with std::visit
    template<typename... Ts> struct VarDispatcher: Ts...
    {
        using Ts::operator()...;
    };

    // scoped-assignment RAII helper: sets keep = k on construction, restores the old value on destruction
    template<typename T> class ValueKeeper final
    {
        private:
            T & m_ref;
            T   m_oldValue;

        public:
            template<typename K> ValueKeeper(T& keep, K&& k)
                : m_ref(keep)
                , m_oldValue(keep)
            {
                m_ref = std::forward<K>(k);
            }

            ~ValueKeeper()
            {
                m_ref = std::move(m_oldValue);
            }

        public:
            ValueKeeper            (const ValueKeeper &) = delete;
            ValueKeeper & operator=(const ValueKeeper &) = delete;
    };
}

namespace stdf
{
    template<typename... U> constexpr bool hasInt(int var, int candidate, U&&... u)
    {
        return (var == candidate) || hasInt(var, std::forward<U>(u)...);
    }

    template<> constexpr bool hasInt(int var, int candidate)
    {
        return var == candidate;
    }

    // map2Int(szStr, D_NONE,
    //      u8"攻击", D_ATTACK,
    //      u8"挨打", D_HITTEF,
    //      u8"行走", D_WALK)
    template<typename... U> constexpr int map2Int(const char8_t *s, int defVar, const char8_t *optStr, int optVar, U&&... u)
    {
        return (std::u8string_view(s) == optStr) ? optVar : map2Int(s, defVar, std::forward<U>(u)...);
    }

    template<> constexpr int map2Int(const char8_t *s, int defVar, const char8_t *optStr, int optVar)
    {
        return (std::u8string_view(s) == optStr) ? optVar : defVar;
    }
}

namespace stdf
{
    namespace _scope_guard_details
    {
        class scope_guard_base
        {
            protected:
                bool m_dismissed;

            protected:
                explicit scope_guard_base(bool dismissed = false) noexcept
                    : m_dismissed(dismissed)
                {}

                static scope_guard_base make_empty_scope_guard() noexcept
                {
                    return scope_guard_base{};
                }

            public:
                void dismiss() noexcept { m_dismissed =  true; }
                void  rehire() noexcept { m_dismissed = false; }
        };

        template<typename FunctionType> class scope_guard: public scope_guard_base
        {
            private:
                FunctionType m_function;

            public:
                explicit scope_guard(      FunctionType  & fn) noexcept(std::is_nothrow_copy_constructible_v<FunctionType>) : scope_guard(std::as_const(fn)        , make_failsafe(std::is_nothrow_copy_constructible<FunctionType>{}, &fn)) {}
                explicit scope_guard(const FunctionType  & fn) noexcept(std::is_nothrow_copy_constructible_v<FunctionType>) : scope_guard(fn                       , make_failsafe(std::is_nothrow_copy_constructible<FunctionType>{}, &fn)) {}
                explicit scope_guard(      FunctionType && fn) noexcept(std::is_nothrow_move_constructible_v<FunctionType>) : scope_guard(std::move_if_noexcept(fn), make_failsafe(std::is_nothrow_move_constructible<FunctionType>{}, &fn)) {}

            public:
                scope_guard            (const scope_guard  &) = delete;
                scope_guard & operator=(const scope_guard  &) = delete;
                scope_guard & operator=(      scope_guard &&) = delete; // do I need this ?

            public:
                scope_guard(scope_guard&& other) noexcept(std::is_nothrow_move_constructible_v<FunctionType>)
                    : scope_guard_base()
                    , m_function(std::move_if_noexcept(other.m_function)) // other.m_function does the cleanup if this line throws
                {
                    m_dismissed = std::exchange(other.m_dismissed, true);
                }

                ~scope_guard() noexcept
                {
                    if (!m_dismissed) {
                        try{
                            m_function();
                        }
                        catch(const std::exception &e){
                            std::cerr << "Caught exception in scope_guard: " << e.what() << std::endl;
                        }
                        catch(...){
                            std::cerr << "Caught exception in scope_guard: unknown error" << std::endl;
                        }
                    }
                }

            private:
                static scope_guard_base make_failsafe(std::true_type, const void *) noexcept
                {
                    return make_empty_scope_guard();
                }

                template <typename Fn> static auto make_failsafe(std::false_type, Fn* fn) noexcept -> scope_guard<decltype(std::ref(*fn))>
                {
                    return scope_guard<decltype(std::ref(*fn))>{std::ref(*fn)};
                }

            private:
                template <typename Fn> explicit scope_guard(Fn && fn, scope_guard_base && failsafe)
                    : scope_guard_base{}
                    , m_function(std::forward<Fn>(fn))
                {
                    failsafe.dismiss();
                }

            public:
                void* operator new(std::size_t) = delete;
        };
    }

    template<typename F> [[nodiscard]] auto guard(F&& f) noexcept(noexcept(_scope_guard_details::scope_guard<std::decay_t<F>>(static_cast<F &&>(f))))
    {
        return _scope_guard_details::scope_guard<std::decay_t<F>>(static_cast<F &&>(f));
    }
}
