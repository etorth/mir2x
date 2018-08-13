/*
 * =====================================================================================
 *
 *       Filename: uidrecord.hpp
 *        Created: 05/01/2017 11:35:58
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
#include <cstdint>
#include <typeinfo>
#include <type_traits>
#include <Theron/Address.h>

class UIDRecord
{
    private:
        uint32_t m_UID;

    private:
        Theron::Address m_Address;

    public:
        UIDRecord(uint32_t nUID, const Theron::Address &rstAddress)
            : m_UID(nUID)
            , m_Address(rstAddress)
        {}

        UIDRecord()
            : UIDRecord(0, Theron::Address::Null())
        {}

    public:
        uint32_t UID() const
        {
            return m_UID;
        }

        const Theron::Address &GetAddress() const
        {
            return m_Address;
        }

    public:
        operator bool () const
        {
            return UID() != 0;
        }
};
