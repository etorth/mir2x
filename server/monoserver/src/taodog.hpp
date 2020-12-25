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
        bool m_standMode = false;

    public:
        TaoDog(ServiceCore *corePtr, ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"神兽"), corePtr, mapPtr, argX, argY, argDir, masterUID)
        {}

    public:
        void setStandMode(bool standMode)
        {
            if(standMode != m_standMode){
                m_standMode = standMode;
                dispatchAction(ActionTransf
                {
                    .x = X(),
                    .y = Y(),
                    .direction = Direction(),
                    .standMode = m_standMode,
                });
            }
        }

        void setTarget(uint64_t uid) override
        {
            Monster::setTarget(uid);
            setStandMode(true);
        }

    public:
        bool standMode() const
        {
            return m_standMode;
        }
};
