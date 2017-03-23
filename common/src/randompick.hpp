/*
 * =====================================================================================
 *
 *       Filename: randompick.hpp
 *        Created: 06/18/2016 21:53:32
 *  Last Modified: 03/21/2017 11:09:52
 *
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
#include <vector>
#include <utility>
#include <cassert>

template<typename Record>
class RandomPick
{
    private:
        std::vector<uint32_t> m_ProbV;
        std::vector<Record>   m_RecordV;
        std::vector<double>   m_CProbV;

    public:
        RandomPick() = default;
       ~RandomPick() = default;

    public:
        template<typename... T> void Add(uint32_t nProb, T&&... stT)
        {
            // 1. push this record back
            m_ProbV.push_back(nProb);
            m_RecordV.emplace_back(std::forward<T>(stT)...);

            // 2. re-calculate the cumulative probability
            double fSum = 1.0 * std::accumulate(m_ProbV.begin(), m_ProbV.end(), 0);

            // 3. clear last cumulative probability distribution
            m_CProbV.clear();

            // 4. push one by one
            std::for_each(m_ProbV.begin(), m_ProbV.end(), [this, fSum](uint32_t nProb){
                m_CProbV.push_back(m_CProbV.size() ? (m_CProbV.back() + 1.0 * nProb / fSum) : (1.0 * nProb / fSum));
            });
        }

        // TODO: here we return a copy
        //       since mostly we use only for number picking, if for object picking
        //       we should define move constructor to take advantage of this return object
        Record Pick()
        {
            // 1. check size, we can't pick record from null v
            assert(Size() > 0);

            // 2. get random number
            double fPick = (std::rand() % 99991) * 1.0 / 99991.0;

            // 3. do pick
            for(size_t nIndex = 0; nIndex < m_CProbV.size(); ++nIndex){
                if(fPick < m_CProbV[nIndex]){ return m_RecordV[nIndex]; }
            }

            return m_RecordV.back();
        }

        size_t Size()
        {
            return m_CProbV.size();
        }
};
