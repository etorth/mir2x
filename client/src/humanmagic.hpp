/*
 * =====================================================================================
 *
 *       Filename: humanmagic.hpp
 *        Created: 08/07/2017 21:19:44
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
#include "magicbase.hpp"
#include "fixedlocmagic.hpp"
#include "followuidmagic.hpp"

template<int magicID, int magicStage> class HumanMagic;
template<> class HumanMagic<DBCOM_MAGICID(u8"灵魂火符"), magicStageID(u8"运行")>: public FollowUIDMagic
{
    public:
        HumanMagic(int x, int y, int gfxDirIndex, uint64_t aimUID, ProcessRun *runPtr)
            : FollowUIDMagic(u8"灵魂火符", u8"运行", x, y, gfxDirIndex, 10, aimUID, runPtr)
        {
            addOnDone([this]()
            {
                auto magicPtr = new AttachMagic(u8"灵魂火符", u8"结束");
                magicPtr->addOnDone([this]()
                {
                    m_process->requestMagicDamage(DBCOM_MAGICID(u8"灵魂火符"), m_uid);
                });
                m_process->addAttachMagic(m_uid, std::unique_ptr<AttachMagic>(magicPtr));
            });
        }
};
