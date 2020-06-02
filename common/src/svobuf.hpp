/*
 * =====================================================================================
 *
 *       Filename: svobuf.hpp
 *        Created: 03/29/2019 04:54:20
 *    Description: 
 *                should use std::align_storage not std::array
 *                std::array calls T()
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

template<typename T, size_t N> class svobuf
{
    private:
        size_t m_size = 0;

    private:
        std::vector<T> m_vec;

    private:
        std::array<T, N> m_arr;

    public:
        T &at(size_t n)
        {
            return const_cast<T &>(static_cast<const svobuf *>(this)->at(n));
        }

        const T &at(size_t n) const
        {
            if(n >= m_size){
                throw std::out_of_range(__PRETTY_FUNCTION__);
            }

            if(m_size <= N){
                return m_arr[n];
            }

            if(n < N){
                return m_arr[n];
            }
            return m_vec[n - N];
        }

    public:
        void push_back(const T &val)
        {
            if(m_size < N){
                m_arr[m_size] = val;
                m_size++;
                return;
            }

            m_vec.push_back(val);
            m_size++;
        }

    public:
        size_t size() const
        {
            return m_size;
        }
};
