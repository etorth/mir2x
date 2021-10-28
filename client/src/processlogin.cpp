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
#include "notifyboard.hpp"
#include "processlogin.hpp"

extern Log *g_log;
extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessLogin::ProcessLogin()
	: Process()
	, m_button1(DIR_UPLEFT, 150, 482, {0X00000005, 0X00000006, 0X00000007}, nullptr, nullptr, [this](){ doCreateAccount();  })
	, m_button2(DIR_UPLEFT, 352, 482, {0X00000008, 0X00000009, 0X0000000A}, nullptr, nullptr, [this](){ doChangePassword(); })
	, m_button3(DIR_UPLEFT, 554, 482, {0X0000000B, 0X0000000C, 0X0000000D}, nullptr, nullptr, [this](){ doExit();           })
    , m_button4(DIR_UPLEFT, 600, 536, {0X0000000E, 0X0000000F, 0X00000010}, nullptr, nullptr, [this](){ doLogin();          })
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
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

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
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

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
          colorf::YELLOW + colorf::A_SHF(255),
      }

    , m_notifyBoard
      {
          DIR_UPLEFT,
          0,
          0,
          0,
          1,
          15,
          0,
          colorf::YELLOW + colorf::A_SHF(255),
          5000,
          10,
      }
{
    m_buildSignature.setText(u8"编译版本号:%s", getBuildSignature());
}

void ProcessLogin::update(double fUpdateTime)
{
    m_idBox.update(fUpdateTime);
    m_passwordBox.update(fUpdateTime);
    m_notifyBoard.update(fUpdateTime);
}

void ProcessLogin::draw() const
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000003),   0,  75);
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000004),   0, 465);
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000011), 103, 536);

    m_button1.draw();
    m_button2.draw();
    m_button3.draw();
    m_button4.draw();

    m_idBox      .draw();
    m_passwordBox.draw();

    m_buildSignature.draw();

    const int notifX = (800 - m_notifyBoard.pw()) / 2;
    const int notifY = (600 - m_notifyBoard. h()) / 2;
    const int margin = 15;

    if(!m_notifyBoard.empty()){
        g_sdlDevice->fillRectangle(colorf::RGBA(0, 0,   0, 128), notifX - margin, notifY - margin, m_notifyBoard.pw() + margin * 2, m_notifyBoard.h() + margin * 2, 8);
        g_sdlDevice->drawRectangle(colorf::RGBA(0, 0, 255, 128), notifX - margin, notifY - margin, m_notifyBoard.pw() + margin * 2, m_notifyBoard.h() + margin * 2, 8);
    }
    m_notifyBoard.drawAt(DIR_UPLEFT, notifX, notifY);
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
    const auto idStr  = m_idBox.getRawString();
    const auto pwdStr = m_passwordBox.getPasswordString();

    if(idStr.empty() || pwdStr.empty()){
        m_notifyBoard.addLog(u8"无效的账号或密码");
    }
    else{
        g_log->addLog(LOGTYPE_INFO, "Login account: (%s:%s)", idStr.c_str(), pwdStr.c_str());

        CMLogin cmL;
        std::memset(&cmL, 0, sizeof(cmL));

        cmL.id.assign(idStr);
        cmL.password.assign(pwdStr);
        g_client->send(CM_LOGIN, cmL);
    }
}

void ProcessLogin::doCreateAccount()
{
    g_client->requestProcess(PROCESSID_CREATEACCOUNT);
}

void ProcessLogin::doChangePassword()
{
    g_client->requestProcess(PROCESSID_CHANGEPASSWORD);
}

void ProcessLogin::doExit()
{
    std::exit(0);
}
