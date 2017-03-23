/*
 * =====================================================================================
 *
 *       Filename: serverobject.hpp
 *        Created: 04/13/2016 20:04:39
 *  Last Modified: 03/21/2017 22:30:53
 *
 *    Description: basis of all objects in monoserver, with
 *
 *                   --Category()
 *                   --ID()
 *                   --AddTime()
 *
 *                 previous I made an AsyncObject, and ServerObject derived from
 *                 AsyncObject, since now I employed actors, I make ServerObject
 *                 as the basic of all objects in monoserver, rather than the
 *                 AsyncObject anymore
 *
 *
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

enum ObjectCategory: uint8_t
{
    CATEGORY_NONE           = 0,
    CATEGORY_ITEM           = 1,
    CATEGORY_EVENT          = 2,
    CATEGORY_ACTIVEOBJECT   = 3,
};

class ServerObject
{
    protected:
        const uint8_t  m_Category;
        const uint32_t m_UID;
        const uint32_t m_AddTime;

    public:
        explicit ServerObject(uint8_t nCategory, uint32_t nUID, uint32_t nAddTime)
            : m_Category(nCategory)
            , m_UID(nUID)
            , m_AddTime(nAddTime)
        {}

        explicit ServerObject(uint8_t);
        virtual ~ServerObject() = default;

    public:
        uint8_t Category() const
        {
            return m_Category;
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
