/*
 * =====================================================================================
 *
 *       Filename: netonhc.cpp
 *        Created: 02/22/2016 16:57:49
 *  Last Modified: 02/22/2016 17:54:29
 *
 *    Description: when received head code
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
#include <SDL2/SDL.h>

#include "game.hpp"

Game::OnPing()
{
    static Uint32 nTick;

    auto fnOnPing = [this, &nTick](std::error_code stEC, std::size_t){
        if(stEC){
            m_NetIO.Stop();
        }else{
            // MakeEvent(SM_PING, &nTick, sizeof(nTick));
            MakeEvent(SM_PING, nTick);
        }
    };

    m_NetIO.Read(stSMPing, asio::buffer(&nTick, sizeof(nTick)), fnOnPing)
}
