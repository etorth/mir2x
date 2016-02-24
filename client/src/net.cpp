/*
 * =====================================================================================
 *
 *       Filename: net.cpp
 *        Created: 02/23/2016 00:09:59
 *  Last Modified: 02/24/2016 01:24:27
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

void Game::NetFunc(void *pData)
{
    Game *pGame = (Game *)pData;
    if(pGame){
        pGame->RunASIO();
    }
}

void Game::RunASIO()
{
    // this function will run in another thread
    // make sure there is no data race
    auto fnStartRead = [this](std::error_code stEC, asio::ip::tcp::resolver::iterator){
        if(stEC){
            SDL_Log("Connect to server IP = %s, Port = %d failed.", m_ServerIP, m_ServerPort);
        }else{
            ReadHC();
        }
    };

    m_NetIO.Run(m_ServerIP, m_ServerPort, fnStartRead);
}

void Game::ReadHC()
{
    std::function<void(uint8_t)> fnProcessHC = [this, fnProcessHC](uint8_t nSMID){
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
