/*
 * =====================================================================================
 *
 *       Filename: netevent.cpp
 *        Created: 01/16/2016 09:48:40
 *  Last Modified: 02/22/2016 16:36:34
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

// SDL_UserEvent::code used as four part:

// code: ch4 ch3 ch2 ch1
//      ch4     : denote as net event or timer event, we can add more event here
//      ch3     : if it's net event, this is CM_ID / SM_ID
//      ch2/ch1 : used as ch[2];

void Game::PushNetEvent(uint8_t chMsgType, size_t nDataLen, uint8_t *pData)
{
    // for each SDL_Event we have 8 + 8 + 2 bytes
    // if more than that we need to allocate memory
    if(MessageTypeSize(chMsgType) <= 18){
    }
}

void Game::OnNetLoginOK()
{
    static SMLoginOK stMsg;
    auto fnProcessLoginOK = [this, &stMsg](){

    };
    m_NetIO.Read(stMsg, std::);
}

void Game::ProcessNet(ServerMessageID nMsgID)
{
    switch(nMsgID){
        case ServerMessageID::Ping: 
    }
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
