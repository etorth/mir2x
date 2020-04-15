/*
 * =====================================================================================
 *
 *       Filename: clientpathfinder.cpp
 *        Created: 03/28/2017 21:15:25
 *    Description: class for path finding in ProcessRun
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

#include "log.hpp"
#include "client.hpp"
#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientpathfinder.hpp"

extern Log *g_Log;
extern Client *g_client;

ClientPathFinder::ClientPathFinder(bool bCheckGround, int nCheckCreature, int nMaxStep)
    : AStarPathFinder([this](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double
      {
          if(0){
              if(true
                      && MaxStep() != 1
                      && MaxStep() != 2
                      && MaxStep() != 3){
                  throw fflerror("invalid MaxStep provided: %d, should be (1, 2, 3)", MaxStep());
              }

              int nDistance2 = mathf::LDistance2(nSrcX, nSrcY, nDstX, nDstY);
              if(true
                      && nDistance2 != 1
                      && nDistance2 != 2
                      && nDistance2 != MaxStep() * MaxStep()
                      && nDistance2 != MaxStep() * MaxStep() * 2){
                  throw fflerror("invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
              }
          }

          auto pRun = (ProcessRun *)(g_client->ProcessValid(PROCESSID_RUN));

          if(!pRun){
              throw fflerror("ProcessRun is invalid");
          }
          return pRun->OneStepCost(this, m_CheckGround, m_CheckCreature, nSrcX, nSrcY, nDstX, nDstY);
      }, nMaxStep)
    , m_CheckGround(bCheckGround)
    , m_CheckCreature(nCheckCreature)
{
    switch(m_CheckCreature){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid CheckCreature provided: %d, should be (0, 1, 2)", m_CheckCreature);
            }
    }

    switch(MaxStep()){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid MaxStep provided: %d, should be (1, 2, 3)", MaxStep());
            }
    }
}

int ClientPathFinder::GetGrid(int nX, int nY) const
{
    auto pRun = (ProcessRun *)(g_client->ProcessValid(PROCESSID_RUN));

    if(!pRun){
        throw fflerror("ProcessRun is invalid");
    }

    if(!pRun->ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    int32_t nX32 = nX;
    int32_t nY32 = nY;

    uint64_t nKey = ((uint64_t)(nX32) << 32) | nY32;
    if(auto p = m_Cache.find(nKey); p != m_Cache.end()){
        return p->second;
    }

    auto nGrid = pRun->CheckPathGrid(nX, nY);
    m_Cache[nKey] = nGrid;
    return nGrid;
}
