#pragma once
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
