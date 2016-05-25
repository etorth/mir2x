/*
 * =====================================================================================
 *
 *       Filename: serverobject.hpp
 *        Created: 04/13/2016 20:04:39
 *  Last Modified: 05/24/2016 21:46:02
 *
 *    Description: basis of all objects in monoserver, with
 *
 *                   --ID()
 *                   --AddTime()
 *                   --Category()
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

enum ObjectCategory: uint8_t {
    CATEGORY_NONE,
    CATEGORY_ITEM,
    CATEGORY_EVENT,
    CATEGORY_ACTIVEOBJECT,
};

class ServerObject
{
    protected:
        uint8_t  m_Category;

    protected:
        uint32_t m_UID;
        uint32_t m_AddTime;


    public:
        explicit ServerObject(uint8_t);
        virtual ~ServerObject() = default;

    public:
        uint32_t UID()
        {
            return m_UID;
        }

        uint32_t AddTime()
        {
            return m_AddTime;
        }

        uint8_t Category()
        {
            return m_Category;
        }
};
