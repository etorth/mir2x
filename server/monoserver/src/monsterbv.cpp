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
bvnode_ptr Monster::BvTree_GetMasterUID()
{
    return bvtree::lambda_bool([this](bvarg_ref pOutput)
    {
        pOutput.assign<uint64_t>(MasterUID());
        return MasterUID();
    });
}

bvnode_ptr Monster::BvTree_FollowMaster()
{
    return bvtree::lambda_bool([this](){ return true; });
}

bvnode_ptr Monster::BvTree_LocateUID(bvarg_ref pUID)
{
    auto pInited = make_bvarg<bool>(false);
    auto fnReset = [pInited]() mutable
    {
        pInited.assign<bool>(false);
    };

    auto pDone = make_bvarg<bool>(false);
    auto fnUpdate = [this, pUID, pInited, pDone](bvarg_ref p) mutable -> bvres_t
    {
        if(!pInited.as<bool>()){
            auto fnOnOK = [p, pDone](const COLocation &loc) mutable
            {
                pDone.assign<bool>(true);
                p.assign<uint64_t>(loc.MapID);
            };

            auto fnOnError = [p, pDone]() mutable
            {
                pDone.assign<bool>(true);
                p.assign<uint64_t>(0);
            };

            RetrieveLocation(pUID.as<uint64_t>(), fnOnOK, fnOnError);
            pInited.assign<bool>(true);
        }

        if(pDone.as<bool>()){
            return p.as<uint64_t>() ? BV_SUCCESS : BV_FAILURE;
        }
        return BV_PENDING;
    };
    return bvtree::lambda(fnReset, fnUpdate);
}

bvnode_ptr Monster::BvTree_LocateMaster()
{
    bvarg_ref pMasterUID = make_bvarg<uint64_t>(0);
    return bvtree::if_check
    (
        BvTree_GetMasterUID()->bind_outref(pMasterUID),
        BvTree_LocateUID(pMasterUID)
    );
}
