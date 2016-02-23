/*
 * =====================================================================================
 *
 *       Filename: netonhc.cpp
 *        Created: 01/16/2016 09:48:40
 *  Last Modified: 02/23/2016 04:09:39
 *
 *    Description: define operations on receiving HC
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

void Game::OnMyHeroDesc()
{
    m_LastStringHC = SM_MYHERODESC;
    static uint16_t nNewStringLen;
    m_NetIO.Read(nNewStringLen,
            [this, &nNewStringLen](){ ExtendNetString(nNewStringLen); });
}

void Game::OnString()
{
    switch(m_StringHC){
        case SM_MYHERODESC:
        {
            m_NetIO.Read(m_NetString, m_NetStrinLen,
                    [this](){ ParseMyHeroDescString() });
            break;
        }
        case SM_BROADCAST:
        {
            m_NetIO.Read(m_NetString, m_NetStrinLen,
                    [this](){ SetServerBroadcast() });
            break;
        }
        default:
        {
            SDL_Log("Undefined string package");
        }
    }
}

void Game::OnLoginOK()
{
    // LoginOK(No Data) ->
    // PlayerDescString(String Length) ->
    // String(String Data) ->
    //
    m_LoginOK = true;
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
