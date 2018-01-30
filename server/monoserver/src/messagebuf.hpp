/*
 * =====================================================================================
 *
 *       Filename: messagebuf.hpp
 *        Created: 05/03/2016 13:14:40
 *    Description: used to shorten the argument list, so keep it simple
 *                 MessageBuf won't maintain the validation of the pointer
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
#include <cstdint>

class MessageBuf
{
    private:
        int            m_Type;
        const uint8_t *m_Data;
        size_t         m_DataLen;

    public:
        // message descriptor without body
        MessageBuf(int nMessageType)
            : MessageBuf(nMessageType, nullptr, 0)
        {}

        // message descriptor with body
        // if we provide body, we need to supply both pointer and length
        MessageBuf(int nMessageType, const uint8_t *pData, size_t nDataLen)
            : m_Type(nMessageType)
            , m_Data(pData)
            , m_DataLen(nDataLen)
        {}

        // actually you can put a pointer here
        // which makes a MessagePack contain a pointer value
        //
        // since we defined MessageBuf(int, const uint8_t *, size_t)
        // this won't cause ambiguity
        template <typename T> MessageBuf(int nMsgType, const T &rstPOD)
            : MessageBuf(nMsgType, (const uint8_t *)&rstPOD, sizeof(rstPOD))
        {
            static_assert(std::is_pod<T>::value, "POD data type supported only");
        }

    public:
        int Type() const
        {
            return m_Type;
        }

        const uint8_t *Data() const
        {
            return m_Data;
        }

        size_t DataLen() const
        {
            return m_DataLen;
        }

        size_t Size() const
        {
            return DataLen();
        }

    public:
        int Type()
        {
            return m_Type;
        }

        const uint8_t *Data()
        {
            return m_Data;
        }

        size_t DataLen()
        {
            return m_DataLen;
        }

        size_t Size()
        {
            return DataLen();
        }
};
