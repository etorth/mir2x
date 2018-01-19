/*
 * =====================================================================================
 *
 *       Filename: uidrecord.hpp
 *        Created: 05/01/2017 11:35:58
 *  Last Modified: 01/19/2018 00:20:39
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
#include <cstdint>
#include <typeinfo>
#include <type_traits>
#include <Theron/Address.h>
#include "invardata.hpp"

class UIDRecord
{
    private:
        uint32_t m_UID;

    private:
        size_t m_ClassCode;

    private:
        InvarData m_InvarData;

    private:
        Theron::Address m_Address;

    public:
        UIDRecord(uint32_t nUID, size_t nClassCode, const InvarData &rstData, const Theron::Address &rstAddress)
            : m_UID(nUID)
            , m_ClassCode(nClassCode)
            , m_InvarData(rstData)
            , m_Address(rstAddress)
        {}

        UIDRecord()
            : UIDRecord(0, 0, {}, Theron::Address::Null())
        {}

    public:
        uint32_t UID() const
        {
            return m_UID;
        }

        size_t ClassCode() const
        {
            return m_ClassCode;
        }

        const InvarData &GetInvarData() const
        {
            return m_InvarData;
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

    public:
        template<typename T> bool ClassFrom() const
        {
            return true
                && std::is_final<T>::value
                && ClassCode() == typeid(T).hash_code();
        }
};
