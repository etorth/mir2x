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
