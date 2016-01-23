/*
 * =====================================================================================
 *
 *       Filename: netevent.cpp
 *        Created: 01/16/2016 09:48:40
 *  Last Modified: 01/23/2016 01:56:06
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


void Game::PushNetEvent(uint8_t *pBuf)
{

}

void Game::OnNetLoginOK()
{
    static SMLoginOK stMsg;
    auto fnProcessLoginOK = [this, &stMsg](){

    };
    m_NetIO.Read(stMsg, std::);
}

void Game::ProcessNet(uint8_t nMsgHead)
{
    switch(nMsgHead){
        case SM_NONE:      OnNetNone();      break;
        case SM_BROADCAST: OnNetBroadcast(); break;
        case SM_LOGINOK:   OnNetLoginOK();   break;
        default: break;
    }
    m_NetIO.Read();
}

void Game::NetByte()
{
    uint8_t chMsgHead = 0;
    auto fnRead = [this](){

    };
}

void Game::NetEvent()
{


    auto 
    m_NetIO.Read()
    auto fnRead = [this](std::error_code stEC, std::size_t){
        if(!stEC){
            DoReadBody();
        }else{
            m_Socket.close();
        }
    };
    asio::async_read(m_Socket, asio::buffer(m_Message.Data(), m_Message.HeaderSize()), fnRead);
}
