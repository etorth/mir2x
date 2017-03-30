/*
 * =====================================================================================
 *
 *       Filename: serverobject.hpp
 *        Created: 04/13/2016 20:04:39
 *  Last Modified: 03/30/2017 00:21:07
 *
 *    Description: basis of all objects in monoserver, with
 *
 *                   --ID()
 *                   --Active()
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

class ServerObject
{
    private:
        const bool m_Active;

    private:
        const uint32_t m_UID;

    public:
        explicit ServerObject(bool bActive, uint32_t nUID)
            : m_Active(bActive)
            , m_UID(nUID)
        {}

        explicit ServerObject(bool);
        virtual ~ServerObject() = default;

    public:
        bool Active() const
        {
            return m_Active;
        }

        uint32_t UID() const
        {
            return m_UID;
        }
};
