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
extern SDLDevice *g_sdlDevice;

ProcessLogin::ProcessLogin()
	: Process()
	, m_button1(DIR_UPLEFT, 150, 482, {0X00000005, 0X00000006, 0X00000007}, nullptr, nullptr, [this](){ doCreateAccount(); })
	, m_button2(DIR_UPLEFT, 352, 482, {0X00000008, 0X00000009, 0X0000000A}, nullptr, nullptr, [    ](){                    })
	, m_button3(DIR_UPLEFT, 554, 482, {0X0000000B, 0X0000000C, 0X0000000D}, nullptr, nullptr, [    ](){ std::exit(0);      })
    , m_button4(DIR_UPLEFT, 600, 536, {0X0000000E, 0X0000000F, 0X00000010}, nullptr, nullptr, [this](){ doLogin();         })
	, m_idBox
      {
          DIR_UPLEFT,
          159,
          540,
          146,
          18,

          2,
          18,
          0,
          colorf::WHITE + 255,

          2,
          colorf::WHITE + 255,

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
          DIR_UPLEFT,
          409,
          540,
          146,
          18,
          true,

          2,
          18,
          0,
          colorf::WHITE + 255,

          2,
          colorf::WHITE + 255,

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
            DIR_UPLEFT,
            0,
            0,
            u8"build",
            1,
            14,
            0,
            colorf::YELLOW + 255,
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
    SDLDeviceHelper::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(g_progUseDB->Retrieve(0X00000003),   0,  75);
    g_sdlDevice->drawTexture(g_progUseDB->Retrieve(0X00000004),   0, 465);
    g_sdlDevice->drawTexture(g_progUseDB->Retrieve(0X00000011), 103, 536);

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

        const auto idStr  = m_idBox.getRawString();
        const auto pwdStr = m_passwordBox.getPasswordString();

        CMLogin cmL;
        std::memset(&cmL, 0, sizeof(cmL));

        if((idStr.size() >= sizeof(cmL.id)) || (pwdStr.size() >= sizeof(cmL.password))){
            g_log->addLog(LOGTYPE_WARNING, "Too long id/password provided");
            return;
        }

        std::strcpy(cmL.id, idStr.c_str());
        std::strcpy(cmL.password, pwdStr.c_str());
        g_client->send(CM_LOGIN, cmL);
    }
}

void ProcessLogin::doCreateAccount()
{
    g_client->RequestProcess(PROCESSID_NEW);
}
