/*
 * =====================================================================================
 *
 *       Filename: runasio.cpp
 *        Created: 02/22/2016 16:28:32
 *  Last Modified: 02/22/2016 16:57:38
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

void Game::RunASIO()
{
    std::function<void(uint8_t)> fnProcessHC = [this, fnProcessHC](uint8_t nSMID){
        switch(nSMID){
            case SM_PING:   OnHCPing(); break;
            case SM_LOGINOK:   OnHCLoginOK(); break;
            case SM_PING:   OnPing(); break;
            case SM_PING:   OnPing(); break;
            case SM_PING:   OnPing(); break;
            default: break;
        }
        m_NetIO.ReadHC(fnProcessHC);
    };
    m_NetIO.ReadHC(fnProcessHC);
}
