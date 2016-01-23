/*
 * =====================================================================================
 *
 *       Filename: processlogin.cpp
 *        Created: 8/14/2015 2:47:49 PM
 *  Last Modified: 01/14/2016 06:26:50
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

#include "game.hpp"
#include <cstring>
#include <algorithm>
#include "message.hpp"
#include "processlogin.hpp"
#include "texturemanager.hpp"
#include "devicemanager.hpp"
#include "messagemanager.hpp"
#include "clientmessagedef.hpp"
#include "configurationmanager.hpp"

ProcessLogin::ProcessLogin(Game *pGame)
	: Process(Process::PROCESSID_LOGIN, pGame)
    , m_TextureBackground1()
    , m_TextureBackground2()
    , m_FrameBox()
	, m_Button1(63, 0,  5, [](){})
	, m_Button2(63, 0,  8, [](){})
	, m_Button3(63, 0, 11, [](){ exit(0); })
    , m_Button4(63, 0, 14, [this](){DoLogin(); })
	, m_IDInputBox(146, 14, {0, 14, 0}, {200, 200, 200, 128})
	, m_PasswordBox(146, 14, {0, 14, 0}, {200, 200, 200, 128})
{
    m_TextureBackground1  = GetTextureManager()->RetrieveTexture(63, 0,  3);
    m_TextureBackground2  = GetTextureManager()->RetrieveTexture(63, 0,  4);
    m_FrameBox            = GetTextureManager()->RetrieveTexture(63, 0, 17);
    m_Button1.SetX(150);
    m_Button2.SetX(352);
    m_Button3.SetX(554);
    m_Button4.SetX(600);
    m_Button1.SetY(482);
    m_Button2.SetY(482);
    m_Button3.SetY(482);
    m_Button4.SetY(536);

    m_IDInputBox.SetX(159);
    m_IDInputBox.SetY(540);

    m_PasswordBox.SetX(409);
    m_PasswordBox.SetY(540);
}

ProcessLogin::~ProcessLogin()
{}

void ProcessLogin::Enter()
{
    Process::Enter();
}

void ProcessLogin::Exit()
{
    Process::Exit();
}

void ProcessLogin::HandleMessage(const Message & stMessage)
{
    switch(stMessage.Index()){
        case CLIENTMT_CONNECTSUCCEED:
            {
                break;
            }
        case CLIENTMT_LOGINSUCCEED:
            {
                ClientMessageLoginSucceed stTmpCM;
                std::memcpy(&stTmpCM, stMessage.Body(), sizeof(stTmpCM));
                m_Game->OnProcessRun()->OnLoginSucceed(stTmpCM);
                m_NextProcessID = Process::PROCESSID_RUN;
                break;
            }
        default:
            break;
    }
}

void ProcessLogin::Update()
{
    Process::Update();
    m_IDInputBox.Update(50);
    m_PasswordBox.Update(50);
    auto fnMessageHandler = [this](const Message &stMessage){
        HandleMessage(stMessage);
    };

    GetMessageManager()->BatchHandleMessage(fnMessageHandler);
}

void ProcessLogin::Draw()
{
    SDL_Rect stRectDst1, stRectDst2;
    stRectDst1.x = 0;
    stRectDst1.y = 75;
    stRectDst1.w = 800;
    stRectDst1.h = 450;

    stRectDst2.x = 0;
    stRectDst2.y = 465; //since I know the size
    stRectDst2.w = 800;
    stRectDst2.h = 60;

    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            m_TextureBackground1, nullptr, &stRectDst1);
    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            m_TextureBackground2, nullptr, &stRectDst2);

    SDL_Rect stRectDst3 = { 103, 536, 456, 26 };
    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            m_FrameBox, nullptr, &stRectDst3);

    m_Button1.Draw();
    m_Button2.Draw();
    m_Button3.Draw();
    m_Button4.Draw();

	m_IDInputBox.Draw();
    m_PasswordBox.Draw();
}

void ProcessLogin::HandleEvent(SDL_Event *pEvent)
{
    if(pEvent){
		m_IDInputBox.HandleEvent(*pEvent);
        m_PasswordBox.HandleEvent(*pEvent);

        if(false
                || m_Button1.HandleEvent(*pEvent)
                || m_Button2.HandleEvent(*pEvent)
                || m_Button3.HandleEvent(*pEvent)
                || m_Button4.HandleEvent(*pEvent)
          ){
            return;
        }

        switch(pEvent->type){
            case SDL_KEYDOWN:
                {
                    break;
                }
            default:
                break;
        }
    }
}

void ProcessLogin::DoLogin()
{
    Message stMessage;
    ClientMessageLogin stTmpCM;
    std::memcpy(stTmpCM.szID,  m_IDInputBox.Content(), std::strlen(m_IDInputBox.Content()) + 1);
    std::memcpy(stTmpCM.szPWD, m_PasswordBox.Content(), std::strlen(m_PasswordBox.Content()) + 1);

	stMessage.Set(CLIENTMT_LOGIN, stTmpCM);

    GetMessageManager()->SendMessage(stMessage);
}
