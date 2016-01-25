#pragma once
#include <algorithm>
#include <cstdint>

class Message final
{
    //  Message header structure
    //
    //    32 --- 23   22   21   20   19   18   17    16 --- 9      8 --- 1
    //  +-----------+----+----+----+----+----+----+------------+---------------+---------
    //  |           |    |    |    |    |    |    |            |               |
    //  | Body Size | p6 | p5 | p4 | p3 | p2 | p1 |    p0      | Message Index | Body ...
    //  |           |    |    |    |    |    |    |            |               |
    //  +-----------+----+----+----+----+----+----+------------+---------------+---------
    //       10        1    1    1    1    1    1       8              8
    //
    //  1. Support size = [0, 1023], but buffer will be fixed 1024 for memory align
    //  2. p1 ~ p6 are six bitset parameters
    //  3. P0 are one 8-bit parameter
    //  4. Last 8-bit take as message index, means we support 256 types.

    public:
        enum{
            HEADER_SIZE = sizeof(uint32_t),
            BODY_SIZE   = 1024 - 1,
        };

    public:
        uint8_t P0()
        {
            return (uint8_t)(((((uint32_t *)m_Data)[0]) & 0X0000FF00) >> 8);
        }

        void SetP0(uint8_t nP0)
        {
            (((uint32_t *)m_Data)[0]) &= 0XFFFF00FF;
            (((uint32_t *)m_Data)[0]) |= (((uint32_t)nP0) << 8);
        }

        bool P(int nIndex)
        {
            // nIndex valid in 1 ~ 6
            nIndex = (std::max)(1, (std::min)(nIndex, 6));
            return (bool)((((uint32_t *)m_Data)[0]) & (uint32_t)(1 << (15 + nIndex)));
        }

        void SetP(int nIndex, bool bSet)
        {
            nIndex = (std::max)(1, (std::min)(nIndex, 6));
            if(bSet){
                (((uint32_t *)m_Data)[0]) |= (uint32_t)(1 << (15 + nIndex));
            }else{
                (((uint32_t *)m_Data)[0]) &= (~((uint32_t)(1 << (15 + nIndex))));
            }
        }

        uint8_t Index() const
        {
            return (uint8_t)((((uint32_t *)m_Data)[0]) & 0X000000FF);
        }

        uint8_t Index()
        {
            return (uint8_t)((((uint32_t *)m_Data)[0]) & 0X000000FF);
        }

        void SetIndex(uint8_t nIndex)
        {
            (((uint32_t *)m_Data)[0]) &= 0XFFFFFF00;
            (((uint32_t *)m_Data)[0]) |= ((uint32_t)nIndex);
        }

    public:
        std::size_t BodySize() const
        {
            return (std::size_t)((((uint32_t *)m_Data)[0]) >> 22);
        }

        void SetBodySize(std::size_t nSize)
        {
            (((uint32_t *)m_Data)[0]) &= 0X003FFFFF;
            (((uint32_t *)m_Data)[0]) |= (uint32_t)(((uint32_t)nSize) << 22);
        }

        std::size_t HeaderSize() const
        {
            return Message::HEADER_SIZE;
        }

        void SetHeaderSize() = delete;

    public:
        const uint8_t *Data() const
        {
            return m_Data;
        }

        uint8_t *Data(){
            return m_Data;
        }

        const uint8_t *Body() const
        {
            return m_Data + Message::HEADER_SIZE;
        }

        uint8_t *Body()
        {
            return m_Data + Message::HEADER_SIZE;
        }

    public:
        std::size_t Size() const
        {
            return HeaderSize() + BodySize();
        }

        void SetSize() = delete;

    public:
        std::size_t BodyMaxSize() const
        {
            return Message::BODY_SIZE;
        }

        std::size_t HeaderMaxSize() = delete;

    public:
        void Set(const uint8_t *pData, std::size_t nSize)
        {
            std::memcpy(Body(), pData, (std::min)(nSize, BodyMaxSize()));
            SetBodySize((std::min)(nSize, BodyMaxSize()));
        }

        template<typename T>
        void Set(int nIndex, const T& stArg)
        {
            SetIndex(nIndex);
            Set((const uint8_t *)(&stArg), sizeof(T));
        }

    public:
        Message()
        {
            (((uint32_t *)m_Data)[0]) = 0;
        }

        Message(const Message& stMessage)
        {
			std::memcpy(m_Data, stMessage.m_Data, stMessage.HeaderSize() + stMessage.BodySize());
        }

        Message &operator = (const Message & stMessage)
        {
            if(this != &stMessage){
                std::memcpy(m_Data, stMessage.m_Data, stMessage.HeaderSize() + stMessage.BodySize());
            }
			return *this;
        }

        Message::~Message() = default;

    private:
        uint8_t m_Data[Message::HEADER_SIZE + Message::BODY_SIZE + 1];
};
