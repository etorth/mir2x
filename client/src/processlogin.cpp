#include <cstring>
#include <iostream>
#include <algorithm>

#include "log.hpp"
#include "client.hpp"
#include "message.hpp"
#include "pngtexdb.hpp"
#include "bgmusicdb.hpp"
#include "sdldevice.hpp"
#include "buildconfig.hpp"
#include "notifyboard.hpp"
#include "processlogin.hpp"
#include "clientargparser.hpp"

extern Log *g_log;
extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern BGMusicDB *g_bgmDB;
extern ClientArgParser *g_clientArgParser;

ProcessLogin::ProcessLogin()
	: Process()
	, m_button1(DIR_UPLEFT, 150, 482, {0X00000005, 0X00000006, 0X00000007}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ doCreateAccount();  })
	, m_button2(DIR_UPLEFT, 352, 482, {0X00000008, 0X00000009, 0X0000000A}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ doChangePassword(); })
	, m_button3(DIR_UPLEFT, 554, 482, {0X0000000B, 0X0000000C, 0X0000000D}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ doExit();           })
        , m_button4(DIR_UPLEFT, 600, 536, {0X0000000E, 0X0000000F, 0X00000010}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ doLogin();          })
	, m_idBox
      {
          DIR_UPLEFT,
          159,
          540,
          146,
          18,

          false,

          2,
          18,
          0,
          colorf::WHITE_A255,

          2,
          colorf::WHITE_A255,

          [this]()
          {
              m_idBox      .setFocus(false);
              m_passwordBox.setFocus(true);
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
          colorf::WHITE_A255,

          2,
          colorf::WHITE_A255,

          [this]()
          {
              m_idBox      .setFocus(true);
              m_passwordBox.setFocus(false);
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
    g_sdlDevice->playBGM(g_bgmDB->retrieve(0X00040007));

    if(g_clientArgParser->autoLogin.has_value()){
        sendLogin(g_clientArgParser->autoLogin.value().first, g_clientArgParser->autoLogin.value().second);
    }
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

    m_button1.drawRoot(0, 0);
    m_button2.drawRoot(0, 0);
    m_button3.drawRoot(0, 0);
    m_button4.drawRoot(0, 0);

    m_idBox      .drawRoot(0, 0);
    m_passwordBox.drawRoot(0, 0);

    m_buildSignature.drawRoot(0, 0);

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

                                m_idBox      .setFocus(true);
                                m_passwordBox.setFocus(false);
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

    takeEvent |= m_button1    .applyRootEvent(event, !takeEvent, 0, 0);
    takeEvent |= m_button2    .applyRootEvent(event, !takeEvent, 0, 0);
    takeEvent |= m_button3    .applyRootEvent(event, !takeEvent, 0, 0);
    takeEvent |= m_button4    .applyRootEvent(event, !takeEvent, 0, 0);
    takeEvent |= m_idBox      .applyRootEvent(event, !takeEvent, 0, 0);
    takeEvent |= m_passwordBox.applyRootEvent(event, !takeEvent, 0, 0);
}

void ProcessLogin::doLogin()
{
    const auto idStr  = m_idBox.getRawString();
    const auto pwdStr = m_passwordBox.getPasswordString();

    if(idStr.empty() || pwdStr.empty()){
        m_notifyBoard.addLog(u8"无效的账号或密码");
    }
    else{
        // don't check id/password by idstf functions
        // this allows some test account like: (test, 123456)
        // but when creating account, changing password we need to be extremely careful

        sendLogin(idStr, pwdStr);
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

void ProcessLogin::sendLogin(const std::string &id, const std::string &password)
{
    CMLogin cmL;
    std::memset(&cmL, 0, sizeof(cmL));

    cmL.id.assign(id);
    cmL.password.assign(password);
    g_client->send({CM_LOGIN, cmL});
}
