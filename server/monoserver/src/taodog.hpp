/*
 * =====================================================================================
 *
 *       Filename: taodog.hpp
 *        Created: 04/10/2016 02:32:45
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
#include "dbcomid.hpp"
#include "monster.hpp"

class TaoDog final: public Monster
{
    private:
        bool m_stand = false;

    public:
        TaoDog(ServiceCore *corePtr, ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"神兽"), corePtr, mapPtr, argX, argY, argDir, masterUID)
        {}

    public:
        void setTransf(bool stand)
        {
            m_stand = stand;
            dispatchAction(ActionTransf(X(), Y(), Direction(), m_stand));
        }

        void SetTarget(uint64_t nUID) override
        {
            Monster::SetTarget(nUID);
            setTransf(true);
        }
};
