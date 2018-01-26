/*
 * =====================================================================================
 *
 *       Filename: commonitem.hpp
 *        Created: 07/30/2017 18:44:32
 *    Description: common item is by two parts
 *                 1. ItemRecord
 *                 2. AdditionalAttributes
 *
 *                 in short, a common item is a real item with itself's info
 *
 *                 common item is used by char objects and can be stored in db
 *                 when a common item is on ground or any status not bound to a
 *                 player, it's unbounded.
 *
 *                 when in a bag, dressed, or on hand, it has a valid DBID
 *                 and in the database there is a corresponding entry for it
 *
 *                 a common item has ID and (may have) DBID, no UID
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
class CommonItem
{
    private:
        uint32_t m_ID   = 0;
        uint32_t m_DBID = 0;

    public:
        CommonItem() = default;

    public:
        CommonItem(uint32_t nID, uint32_t nDBID)
            : m_ID(nID)
            , m_DBID(nDBID)
        {}

    public:
        uint32_t ID() const
        {
            return m_ID;
        }

        uint32_t DBID() const
        {
            return m_DBID;
        }

    public:
        operator bool() const
        {
            return ID() != 0;
        }
};
