/*
 * =====================================================================================
 *
 *       Filename: monsterbv.cpp
 *        Created: 03/19/2019 06:43:21
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

#include "monster.hpp"
bvnode_ptr Monster::BvTree_GetMasterUID(bvarg_ref nMasterUID)
{
    return bvtree::lambda_bool([this, nMasterUID]() mutable
    {
        nMasterUID.assign<uint64_t>(MasterUID());
        return MasterUID();
    });
}

bvnode_ptr Monster::BvTree_FollowMaster()
{
    return bvtree::lambda_bool([this](){ return true; });
}

bvnode_ptr Monster::BvTree_LocateUID(bvarg_ref nUID, bvarg_ref stLocation)
{
    auto bInited = make_bvarg<bool>(false);
    auto fnReset = [bInited]() mutable
    {
        bInited.assign<bool>(false);
    };

    auto bDone = make_bvarg<bool>(false);
    auto fnUpdate = [this, nUID, stLocation, bInited, bDone]() mutable -> bvres_t
    {
        if(!bInited.as<bool>()){
            auto fnOnOK = [stLocation, bDone](const COLocation &loc) mutable
            {
                bDone.assign<bool>(true);
                stLocation.assign<uint64_t>(loc.MapID);
            };

            auto fnOnError = [stLocation, bDone]() mutable
            {
                bDone.assign<bool>(true);
                stLocation.assign<uint64_t>(0);
            };

            RetrieveLocation(nUID.as<uint64_t>(), fnOnOK, fnOnError);
            bInited.assign<bool>(true);
        }

        if(bDone.as<bool>()){
            return stLocation.as<uint64_t>() ? BV_SUCCESS : BV_FAILURE;
        }
        return BV_PENDING;
    };
    return bvtree::lambda(fnReset, fnUpdate);
}

bvnode_ptr Monster::BvTree_LocateMaster(bvarg_ref stLocation)
{
    bvarg_ref nMasterUID;

    return bvtree::if_check
    (
        BvTree_GetMasterUID(nMasterUID),
        BvTree_LocateUID(nMasterUID, stLocation)
    );
}

bvnode_ptr Monster::BvTree_RandomMove()
{
    return bvtree::random
    (
        BvTree_RandomTurn(),
        BvTree_MoveOneStep(),
        BvTree_MoveOneStep(),
        BvTree_MoveOneStep(),
        BvTree_MoveOneStep(),
        BvTree_MoveOneStep(),
        BvTree_MoveOneStep(),
        BvTree_MoveOneStep()
    );
}

bvnode_ptr Monster::BvTree_RandomTurn()
{
    auto fnUpdate = [this]() -> bvres_t
    {
        static const int nDirV[]
        {
            DIR_UP,
            DIR_UPRIGHT,
            DIR_RIGHT,
            DIR_DOWNRIGHT,
            DIR_DOWN,
            DIR_DOWNLEFT,
            DIR_LEFT,
            DIR_UPLEFT,
        };

        auto nDirCount = (int)(std::extent<decltype(nDirV)>::value);
        auto nDirStart = (int)(std::rand() % nDirCount);

        for(int nIndex = 0; nIndex < nDirCount; ++nIndex){
            auto nDirection = nDirV[(nDirStart + nIndex) % nDirCount];
            if(Direction() != nDirection){
                int nX = -1;
                int nY = -1;
                if(OneStepReach(nDirection, 1, &nX, &nY) == 1){
                    // current direction is possible for next move
                    // report the turn and do motion (by chance) in next update
                    m_Direction = nDirection;
                    DispatchAction(ActionStand(X(), Y(), Direction()));

                    // we won't do ReportStand() for monster
                    // monster's moving is only driven by server currently
                    return BV_SUCCESS;
                }
            }
        }
        return BV_FAILURE;
    };

    return bvtree::if_branch
    (
        bvtree::lambda(fnUpdate),
        bvtree::op_delay(200),
        bvtree::op_abort()
    );
}

bvnode_ptr Monster::BvTree_MoveOneStep()
{
    enum
    {
        NONE,
        DONE,
        ERROR,
        PENDING,
    };

    bvarg_ref nStage = make_bvarg<int>(NONE);

    auto fnReset = [nStage]() mutable
    {
        nStage.assign<int>(NONE);
    };

    auto fnUpdate = [this, nStage]() mutable -> bvres_t
    {
        switch(auto stage = nStage.as<int>()){
            case NONE:
                {
                    int nX = -1;
                    int nY = -1;

                    if(OneStepReach(Direction(), 1, &nX, &nY) == 1){
                        RequestMove(nX, nY, MoveSpeed(), false, [nStage]() mutable
                        {
                            nStage.assign<int>(DONE);
                        },

                        [nStage]() mutable
                        {
                            nStage.assign<int>(ERROR);
                        });
                        return BV_PENDING;
                    }else{
                        return BV_FAILURE;
                    }
                }
            case DONE:
                {
                    return BV_SUCCESS;
                }
            case ERROR:
                {
                    return BV_FAILURE;
                }
            case PENDING:
                {
                    return BV_PENDING;
                }
            default:
                {
                    throw std::runtime_error(str_fflprintf(": Invalid stage: %d", stage));
                }
        }
    };

    return bvtree::if_branch
    (
        bvtree::lambda(fnReset, fnUpdate),
        bvtree::op_delay(1000),
        bvtree::op_delay( 200)
    );
}
