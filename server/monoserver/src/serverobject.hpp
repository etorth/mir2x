/*
 * =====================================================================================
 *
 *       Filename: serverobject.hpp
 *        Created: 04/13/2016 20:04:39
 *  Last Modified: 03/23/2017 00:02:00
 *
 *    Description: basis of all objects in monoserver, with
 *
 *                   --ID()
 *                   --AddTime()
 *                   --Active()
 *
 *                 previous I made an AsyncObject, and ServerObject derived from
 *                 AsyncObject, since now I employed actors, I make ServerObject
 *                 as the basic of all objects in monoserver, rather than the
 *                 AsyncObject anymore
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

    protected:
        const uint32_t m_UID;
        const uint32_t m_AddTime;

    public:
        explicit ServerObject(bool bActive, uint32_t nUID, uint32_t nAddTime)
            : m_Active(bActive)
            , m_UID(nUID)
            , m_AddTime(nAddTime)
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

        uint32_t AddTime() const
        {
            return m_AddTime;
        }
};
