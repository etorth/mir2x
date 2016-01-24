/*
 * =====================================================================================
 *
 *       Filename: processnetevent.cpp
 *        Created: 01/23/2016 05:22:15
 *  Last Modified: 01/23/2016 05:23:20
 *
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

void Game::ProcessEvent(uint32_t nCode, uint32_t nData1, uint8_t *pData2)
{
    uint8_t B1InCode = (uint8_t)((nCode & 0X000000FF) >>  0);
    uint8_t B2InCode = (uint8_t)((nCode & 0X0000FF00) >>  8);
    uint8_t B3InCode = (uint8_t)((nCode & 0X00FF0000) >> 16);
    
    uint8_t chHC     = B1InCode;
    uint8_t chSubHC  = B2InCode;
    size_t  nDataLen = (size_t)B3InCode;
    
    switch(((uint16_t)(chHC) << 8) + chSubHC){
        case MSG_PING:      NEOnPing((Uint32)nData1); break;
        case MSG_PLAYERNUM: NEOnPlayerNum(nData1);    break;
        default: break;
    }
}
