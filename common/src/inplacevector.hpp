#pragma once
#include <cstddef>
#include <version>

#if __has_include(<inplace_vector>)
#include <inplace_vector>
#endif

#if defined(__cpp_lib_inplace_vector)
namespace stdf
{
    template<typename T, size_t N> using inplace_vector = std::inplace_vector<T, N>;
}
#else
#include <memory>
#include <new>
#include <utility>
#include "fflerror.hpp"

// libc++ (Android NDK) has no std::inplace_vector yet
// minimal replacement, only covers what mir2x uses
namespace stdf
{
    template<typename T, size_t N> class inplace_vector
    {
        private:
            alignas(T) unsigned char m_storage[sizeof(T) * (N ? N : 1)];
            size_t m_size = 0;

        public:
            inplace_vector() = default;

            inplace_vector(const inplace_vector &other)
            {
                for(const auto &t: other){
                    push_back(t);
                }
            }

            inplace_vector(inplace_vector &&other)
            {
                for(auto &t: other){
                    push_back(std::move(t));
                }
            }

            inplace_vector & operator = (const inplace_vector &other)
            {
                if(this != &other){
                    clear();
                    for(const auto &t: other){
                        push_back(t);
                    }
                }
                return *this;
            }

            inplace_vector & operator = (inplace_vector &&other)
            {
                if(this != &other){
                    clear();
                    for(auto &t: other){
                        push_back(std::move(t));
                    }
                }
                return *this;
            }

            ~inplace_vector()
            {
                clear();
            }

        public:
            constexpr static size_t capacity()
            {
                return N;
            }

            size_t size() const
            {
                return m_size;
            }

            bool empty() const
            {
                return m_size == 0;
            }

        public:
            /**/  T *data()       { return std::launder(reinterpret_cast<      T *>(m_storage)); }
            const T *data() const { return std::launder(reinterpret_cast<const T *>(m_storage)); }

            /**/  T *begin()       { return data(); }
            const T *begin() const { return data(); }
            /**/  T *  end()       { return data() + m_size; }
            const T *  end() const { return data() + m_size; }

            /**/  T &operator[](size_t i)       { return data()[i]; }
            const T &operator[](size_t i) const { return data()[i]; }

            /**/  T &at(size_t i)       { if(i >= m_size){ throw fflpanic("index {} out of range, size {}", i, m_size); } return data()[i]; }
            const T &at(size_t i) const { if(i >= m_size){ throw fflpanic("index {} out of range, size {}", i, m_size); } return data()[i]; }

            /**/  T &front()       { return at(0); }
            const T &front() const { return at(0); }
            /**/  T & back()       { return at(m_size - 1); }
            const T & back() const { return at(m_size - 1); }

        public:
            template<typename... Args> T &emplace_back(Args && ... args)
            {
                if(m_size >= N){
                    throw fflpanic("inplace_vector capacity {} exceeded", N);
                }

                T *p = std::construct_at(reinterpret_cast<T *>(m_storage) + m_size, std::forward<Args>(args)...);
                m_size++;
                return *p;
            }

            T &push_back(const T &t) { return emplace_back(t); }
            T &push_back(T &&t)      { return emplace_back(std::move(t)); }

            void pop_back()
            {
                fflassert(m_size > 0);
                m_size--;
                std::destroy_at(data() + m_size);
            }

            void clear()
            {
                std::destroy(begin(), end());
                m_size = 0;
            }
    };
}
#endif
