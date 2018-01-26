/*
 * =====================================================================================
 *
 *       Filename: serverobject.hpp
 *        Created: 04/13/2016 20:04:39
 *    Description: basis of all objects in monoserver, with
 *
 *                   --UID()
 *
 *                   --ClassName()
 *                   --ClassCode()
 *
 *                   --GetInvarData()
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
#include <atomic>
#include <string>
#include <cstdint>
#include <cstddef>
#include <typeinfo>
#include "invardata.hpp"
#include "uidrecord.hpp"

class ServerObject
{
    private:
        const uint32_t m_UID;

    public:
        ServerObject();

    public:
        virtual ~ServerObject() = default;

    public:
        uint32_t UID() const
        {
            return m_UID;
        }

    public:
        const char *ClassName() const
        {
            return typeid(*this).name();
        }

        size_t ClassCode() const
        {
            return typeid(*this).hash_code();
        }

    public:
        virtual InvarData GetInvarData() const
        {
            return {};
        }
};
