#pragma once
#include <cstdint>
#include <bit>

namespace randf
{
    template<float LOW = 0.0f, float HIGH = 1.0f> float uniform(uint32_t &seed)
    {
        static_assert(LOW < HIGH);
        seed = seed * 134775813 + 1;

        const uint32_t u = (seed >> 9) | 0x3f800000;
        const float f = std::bit_cast<float>(u);

        return LOW + (f - 1.0f) * (HIGH - LOW);
    }

    class unigen final
    {
        private:
            uint32_t m_seed;

        public:
            unigen(uint32_t seed) noexcept
                : m_seed(seed)
            {}

        public:
            template<float LOW=0.0f, float HIGH=1.0f> float gen() noexcept
            {
                return uniform<LOW, HIGH>(m_seed);
            }
    };
}
