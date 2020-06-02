/*
 * =====================================================================================
 *
 *       Filename: svobuf.hpp
 *        Created: 03/29/2019 04:54:20
 *    Description: 
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
#include <array>
#include <vector>
#include <cstddef>
#include "fflerror.hpp"

template<typename T, size_t N> class svobuf
{
    private:
        size_t m_size = 0;

    private:
        std::vector<T> m_vec;

    private:
        std::array<T, N> m_arr;

    public:
        size_t size() const
        {
            return m_size;
        }

    public:
        T &at(size_t n)
        {
            return const_cast<T &>(static_cast<const svobuf *>(this)->at(n));
        }

        const T &at(size_t pos) const
        {
            if(pos >= m_size){
                throw fflerror("[%zu] access out of range: %zu", pos, size());
            }

            if(pos < N){
                return m_arr[pos];
            }
            return m_vec[pos - N];
        }

    public:
        auto & operator [] (size_t pos)
        {
            return const_cast<T &>(static_cast<const svobuf *>(this)->operator[](pos));
        }

        const auto & operator [] (size_t pos) const
        {
            if(pos < N){
                return m_arr[pos];
            }
            return m_vec[pos - N];
        }

    public:
        void push_back(const T &val)
        {
            if(size() < N){
                m_arr[m_size++] = val;
                return;
            }

            m_vec.push_back(val);
            m_size++;
        }
};
