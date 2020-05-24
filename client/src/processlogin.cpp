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

extern Log *g_log;
extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

ProcessLogin::ProcessLogin()
	: Process()
	, m_button1(150, 482, 0X00000005, []{}, [this](){ doCreateAccount(); })
	, m_button2(352, 482, 0X00000008, []{}, [    ](){                    })
	, m_button3(554, 482, 0X0000000B, []{}, [    ](){ std::exit(0);      })
    , m_button4(600, 536, 0X0000000E, []{}, [this](){ doLogin();         })
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
              m_passwordBox.focus(true);
          },
          [this]()
          {
              doLogin();
          }
      }
	, m_passwordBox
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
              m_passwordBox.focus(false);
          },
          [this]()
          {
              doLogin();
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

void ProcessLogin::update(double fUpdateTime)
{
    m_idBox.update(fUpdateTime);
    m_passwordBox.update(fUpdateTime);
}

void ProcessLogin::draw()
{
    SDLDevice::RenderNewFrame newFrame;

    g_SDLDevice->DrawTexture(g_progUseDB->Retrieve(0X00000003),   0,  75);
    g_SDLDevice->DrawTexture(g_progUseDB->Retrieve(0X00000004),   0, 465);
    g_SDLDevice->DrawTexture(g_progUseDB->Retrieve(0X00000011), 103, 536);

    m_button1.draw();
    m_button2.draw();
    m_button3.draw();
    m_button4.draw();

    m_idBox      .draw();
    m_passwordBox.draw();

    m_buildSignature.draw();
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
                                    && !m_passwordBox.focus()){

                                m_idBox      .focus(true);
                                m_passwordBox.focus(false);
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

    bool takeEvent = false;

    takeEvent |= m_button1    .processEvent(event, !takeEvent);
    takeEvent |= m_button2    .processEvent(event, !takeEvent);
    takeEvent |= m_button3    .processEvent(event, !takeEvent);
    takeEvent |= m_button4    .processEvent(event, !takeEvent);
    takeEvent |= m_idBox      .processEvent(event, !takeEvent);
    takeEvent |= m_passwordBox.processEvent(event, !takeEvent);
}

void ProcessLogin::doLogin()
{
    if(!(m_idBox.getRawString().empty()) && !(m_passwordBox.getRawString().empty())){
        g_log->addLog(LOGTYPE_INFO, "login account: (%s:%s)", m_idBox.getRawString().c_str(), m_passwordBox.getRawString().c_str());

        const auto szID  = m_idBox.getRawString();
        const auto szPWD = m_passwordBox.getPasswordString();

        CMLogin stCML;
        std::memset(&stCML, 0, sizeof(stCML));

        if((szID.size() >= sizeof(stCML.ID)) || (szPWD.size() >= sizeof(stCML.Password))){
            g_log->addLog(LOGTYPE_WARNING, "Too long ID/PWD provided");
            return;
        }

        std::memcpy(stCML.ID, szID.c_str(), szID.size());
        std::memcpy(stCML.Password, szPWD.c_str(), szPWD.size());

        g_client->send(CM_LOGIN, stCML);
    }
}

void ProcessLogin::doCreateAccount()
{
    g_client->RequestProcess(PROCESSID_NEW);
}
