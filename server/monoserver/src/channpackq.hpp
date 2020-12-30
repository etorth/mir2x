/*
 * =====================================================================================
 *
 *       Filename: channpackq.hpp
 *        Created: 01/27/2018 01:16:08
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
#include <deque>
#include <cstdint>

struct ChannPack
{
    const uint8_t *Data;
    size_t         DataLen;

    const std::function<void()> &DoneCB;

    ChannPack(const uint8_t *pData, size_t nDataLen, const std::function<void()> &rstDoneCB)
        : Data(pData)
        , DataLen(nDataLen)
        , DoneCB(rstDoneCB)
    {}

    operator bool () const
    {
        return Data != nullptr;
    }

    static ChannPack GetEmptyPack()
    {
        static std::function<void()> stEmptyCB;
        return {nullptr, 0, stEmptyCB};
    }
};

struct PackMark
{
    size_t Loc;
    size_t Length;

    std::function<void()> DoneCB;

    PackMark(size_t nLoc, size_t nLength, std::function<void()> &&rstDoneCB)
        : Loc(nLoc)
        , Length(nLength)
        , DoneCB(std::move(rstDoneCB))
    {}
};

class ChannPackQ
{
    private:
        std::vector<uint8_t> m_packBuf;
        std::deque<PackMark> m_packMarkQ;

    public:
        ChannPackQ()
            : m_packBuf()
            , m_packMarkQ()
        {
            m_packBuf.reserve(1024 * 1024 * 1024);
        }

    public:
        ~ChannPackQ() = default;

    public:
        bool Empty() const
        {
            return m_packMarkQ.empty();
        }

    public:
        void Clear()
        {
            m_packBuf.clear();
            m_packMarkQ.clear();
        }

    public:
        ChannPack GetChannPack()
        {
            auto &rstHead = m_packMarkQ.front();
            return ChannPack(&(m_packBuf[rstHead.Loc]), rstHead.Length, rstHead.DoneCB);
        }

        void RemoveChannPack()
        {
            if(!Empty()){
                m_packMarkQ.pop_front();
            }
        }

    public:
        uint8_t *GetPostBuf(size_t);

    public:
        bool AddChannPack(uint8_t, const uint8_t *, size_t, std::function<void()> &&);

    private:
        bool AddPackMark(size_t, size_t, std::function<void()> &&);

    private:
        size_t GetBufOff(const uint8_t *pDst) const
        {
            return pDst - &(m_packBuf[0]);
        }
};
