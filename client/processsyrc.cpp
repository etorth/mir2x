/*
 * =====================================================================================
 *
 *       Filename: processsyrc.cpp
 *        Created: 8/14/2015 2:47:49 PM
 *  Last Modified: 09/03/2015 3:13:54 AM
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

#include <algorithm>
#include "processsyrc.hpp"
#include "texturemanager.hpp"
#include "devicemanager.hpp"
#include "messagemanager.hpp"
#include "clientmessagedef.hpp"

ProcessSyrc::ProcessSyrc(Game *pGame)
	: Process(Process::PROCESSID_SYRC, pGame)
    , m_TextureBackground(nullptr)
    , m_TextureProgressBar(nullptr)
    , m_Ratio(0)
    , m_Info({0, 14, 0}, {255, 255, 255, 0}, "Connecting...")
    // , m_TokenBoardConnect(800, true)
    // , m_TokenBoardConnectSucceed(800, true)
    // , m_CurrentTokenBoard(nullptr)
{
	m_Info.SetX((800 - m_Info.W()) / 2);
	m_Info.SetY(505);

    m_TextureBackground  = GetTextureManager()->RetrieveTexture(63, 0, 1);
    m_TextureProgressBar = GetTextureManager()->RetrieveTexture(63, 0, 2);
}

ProcessSyrc::~ProcessSyrc()
{}

void ProcessSyrc::Enter()
{
    m_FrameCount = 0;

    // if(m_CurrentTokenBoard == nullptr){
    //     tinyxml2::XMLDocument stDoc1;
    //     stDoc1.LoadFile("./Res/Label/connect.xml");
    //     m_TokenBoardConnect.Load(stDoc1);

    //     tinyxml2::XMLDocument stDoc2;
    //     stDoc2.LoadFile("./Res/Label/succeed.xml");
    //     m_TokenBoardConnectSucceed.Load(stDoc2);
    // }
    // m_CurrentTokenBoard = &m_TokenBoardConnect;
    GetMessageManager()->Start();

    SDL_ShowCursor(0);
}

void ProcessSyrc::Exit()
{
    Process::Exit();
    SDL_ShowCursor(1);
}

void ProcessSyrc::HandleMessage(const Message & stMessage)
{
    switch(stMessage.Index()){
        case CLIENTMT_CONNECTSUCCEED:
            {
                m_NextProcessID = Process::PROCESSID_LOGIN;
                m_Info.SetContent("Connection established!");
				m_Info.SetX((GetDeviceManager()->WindowSizeW() - m_Info.W()) / 2);
				m_Info.SetY(505);
                break;
            }
        default:
            break;
    }
}

void ProcessSyrc::Update()
{
    Process::Update();

    m_FrameCount++;
    // testing...
    m_Ratio = m_FrameCount % 100;

    if(m_Ratio == 100){
        m_NextProcessID = Process::PROCESSID_LOGO;
    }

    auto fnMessageHandler = [this](const Message &stMessage){
        HandleMessage(stMessage);
    };

    GetMessageManager()->BatchHandleMessage(fnMessageHandler);
}

void ProcessSyrc::Draw()
{
    SDL_Rect stRectSrc, stRectDst;
	int nW, nH;
	SDL_QueryTexture(m_TextureProgressBar, nullptr, nullptr, &nW, &nH);

    stRectSrc.x = 0;
    stRectSrc.y = 0;
    stRectSrc.w = std::lround(nW * (m_Ratio / 100.0));
	stRectSrc.h = nH;
    stRectDst.x = 112;
    stRectDst.y = 528;
    stRectDst.w = stRectSrc.w;
    stRectDst.h = stRectSrc.h;

    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            m_TextureProgressBar, &stRectSrc, &stRectDst);
    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            m_TextureBackground, nullptr, nullptr);

	m_Info.Draw();

}

void ProcessSyrc::HandleEvent(SDL_Event *pEvent)
{
    if(pEvent){
        switch(pEvent->type){
            case SDL_KEYDOWN:
                {
                    if(pEvent->key.keysym.sym == SDLK_ESCAPE){
                        m_NextProcessID = Process::PROCESSID_LOGO;
                    }
                    break;
                }
            default:
                break;
        }
    }
}
