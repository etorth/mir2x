/*
 * =====================================================================================
 *
 *       Filename: processlogin.cpp
 *        Created: 08/14/2015 02:47:49
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

#include <cstring>
#include <iostream>
#include <algorithm>

#include "log.hpp"
#include "client.hpp"
#include "message.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "buildconfig.hpp"
#include "processlogin.hpp"

extern Log *g_Log;
extern Client *g_client;
extern PNGTexDB *g_ProgUseDB;
extern SDLDevice *g_SDLDevice;

ProcessLogin::ProcessLogin()
	: Process()
	, m_Button1(150, 482, 0X00000005, []{}, [this](){ DoCreateAccount(); })
	, m_Button2(352, 482, 0X00000008, []{}, [    ](){                    })
	, m_Button3(554, 482, 0X0000000B, []{}, [    ](){ std::exit(0);      })
    , m_Button4(600, 536, 0X0000000E, []{}, [this](){ DoLogin();         })
	, m_idBox
      {
          159,
          540,
          146,
          18,

          2,
          18,
          0,
          colorf::WHITE,

          2,
          colorf::WHITE,

          [this]()
          {
              m_idBox      .focus(false);
              m_PasswordBox.focus(true);
          },
          [this]()
          {
              DoLogin();
          }
      }
	, m_PasswordBox
      {
          409,
          540,
          146,
          18,
          true,

          2,
          18,
          0,
          colorf::WHITE,

          2,
          colorf::WHITE,

          [this]()
          {
              m_idBox      .focus(true);
              m_PasswordBox.focus(false);
          },
          [this]()
          {
              DoLogin();
          },
      }

      , m_buildSignature
        {
            0,
            0,
            "build",
            1,
            14,
            0,
            colorf::YELLOW,
        }
{
    m_buildSignature.setText(u8"编译版本号:%s", getBuildSignature());
}

void ProcessLogin::Update(double fMS)
{
    m_idBox.update(fMS);
    m_PasswordBox.update(fMS);
}

void ProcessLogin::Draw()
{
    g_SDLDevice->ClearScreen();

    g_SDLDevice->DrawTexture(g_ProgUseDB->Retrieve(0X00000003),   0,  75);
    g_SDLDevice->DrawTexture(g_ProgUseDB->Retrieve(0X00000004),   0, 465);
    g_SDLDevice->DrawTexture(g_ProgUseDB->Retrieve(0X00000011), 103, 536);

    m_Button1.draw();
    m_Button2.draw();
    m_Button3.draw();
    m_Button4.draw();

    m_idBox      .draw();
    m_PasswordBox.draw();

    m_buildSignature.draw();
    g_SDLDevice->Present();
}

void ProcessLogin::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(true
                                    && !m_idBox      .focus()
                                    && !m_PasswordBox.focus()){

                                m_idBox      .focus(true);
                                m_PasswordBox.focus(false);
                                return;
                            }
                        }
                    default:
                        {
                            break;
                        }
                }
            }
        default:
            {
                break;
            }
    }

    m_Button1.processEvent(event, true);
    m_Button2.processEvent(event, true);
    m_Button3.processEvent(event, true);
    m_Button4.processEvent(event, true);

    // widget idbox and pwdbox are not independent from each other
    // tab in one box will grant focus to another

    if(m_idBox.processEvent(event, true)){
        return;
    }

    if(m_PasswordBox.processEvent(event, true)){
        return;
    }
}

void ProcessLogin::DoLogin()
{
    if(!(m_idBox.getRawString().empty()) && !(m_PasswordBox.getRawString().empty())){
        g_Log->addLog(LOGTYPE_INFO, "login account: (%s:%s)", m_idBox.getRawString().c_str(), m_PasswordBox.getRawString().c_str());

        auto szID  = m_idBox.getRawString();
        auto szPWD = m_PasswordBox.getRawString();

        CMLogin stCML;
        std::memset(&stCML, 0, sizeof(stCML));

        if((szID.size() >= sizeof(stCML.ID)) || (szPWD.size() >= sizeof(stCML.Password))){
            g_Log->addLog(LOGTYPE_WARNING, "Too long ID/PWD provided");
            return;
        }

        std::memcpy(stCML.ID, szID.c_str(), szID.size());
        std::memcpy(stCML.Password, szPWD.c_str(), szPWD.size());

        g_client->send(CM_LOGIN, stCML);
    }
}

void ProcessLogin::DoCreateAccount()
{
    g_client->RequestProcess(PROCESSID_NEW);
}
