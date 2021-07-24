/*
 * =====================================================================================
 *
 *       Filename: clientwedgemoth.hpp
 *        Created: 07/31/2021 08:26:19
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
#include "clientmonster.hpp"

class ClientWedgeMoth: public ClientMonster
{
    public:
        ClientWedgeMoth(uint64_t, ProcessRun *, const ActionNode &);

    protected:
        bool onActionAttack(const ActionNode &) override;
};
