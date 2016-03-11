/*
 * =====================================================================================
 *
 *       Filename: net.cpp
 *        Created: 02/23/2016 00:09:59
 *  Last Modified: 03/10/2016 19:12:14
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
    // this function will run in another thread
    // make sure there is no data race

    m_NetIO.RunIO([this](uint8_t nHC){ ReadHC(nHC); });
}

void Game::ReadHC()
{
    static std::function<void(uint8_t)> fnProcessHC = [this, &fnProcessHC](uint8_t nSMID){
        switch(nSMHC){
            case SM_PING:           OnPing(); break;
            case SM_LOGINOK:        OnLoginOK(); break;
            case SM_PING:           OnPing(); break;
            case SM_PING:           OnPing(); break;
            case SM_PING:           OnPing(); break;
            default: break;
        }
        m_NetIO.ReadHC(fnProcessHC);
    };
    m_NetIO.ReadHC(fnProcessHC);
}
