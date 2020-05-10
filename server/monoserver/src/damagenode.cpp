/*
 * =====================================================================================
 *
 *       Filename: damagenode.cpp
 *        Created: 07/21/2017 17:55:30
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

#include <cstdint>
#include <cinttypes>
#include "damagenode.hpp"
#include "monoserver.hpp"

void DamageNode::EffectArrayType::LogError(int nLogType, const char *szLogInfo) const
{
    extern MonoServer *g_monoServer;
    switch(nLogType){
        case 0  : g_monoServer->addLog(LOGTYPE_INFO,    szLogInfo); break;
        case 1  : g_monoServer->addLog(LOGTYPE_WARNING, szLogInfo); break;
        default : g_monoServer->addLog(LOGTYPE_FATAL,   szLogInfo); break;
    }
}

void DamageNode::Print() const
{
    extern MonoServer *g_monoServer;
    g_monoServer->addLog(LOGTYPE_INFO, "DamageNode::0X%0*" PRIXPTR "::UID       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), (int)(UID));
    g_monoServer->addLog(LOGTYPE_INFO, "DamageNode::0X%0*" PRIXPTR "::Damage    = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Damage    );
    g_monoServer->addLog(LOGTYPE_INFO, "DamageNode::0X%0*" PRIXPTR "::Element   = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Element   );

    for(size_t nIndex = 0; nIndex < EffectArray.EffectLen(); ++nIndex){
        if(EffectArray.Effect()[nIndex] != EFF_NONE){
            g_monoServer->addLog(LOGTYPE_INFO, "DamageNode::0X%0*" PRIXPTR "::Effect[%d] = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), (int)(nIndex), EffectArray.Effect()[nIndex]);
        }
    }
}

int DamageNode::EffectParam(int nEffect) const
{
    if(nEffect > EFF_NONE){
        if(nEffect % 256){
            for(size_t nIndex = 0; nIndex < EffectArray.EffectLen(); ++nIndex){
                if(nEffect == EffectArray.Effect()[nIndex]){
                    return nEffect % 256;
                }
            }
        }else{
            for(size_t nIndex = 0; nIndex < EffectArray.EffectLen(); ++nIndex){
                if((EffectArray.Effect()[nIndex] - (EffectArray.Effect()[nIndex] % 256)) == nEffect){
                    return nEffect % 256;
                }
            }
        }
    }
    return -1;
}
