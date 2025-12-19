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
        , m_canvas
          {{
              .w = 800,
              .h = 600,
          }}

	, m_button1{{.x = 150, .y = 482, .texIDList{.off = 0X00000005, .on = 0X00000006, .down = 0X00000007}, .onTrigger = [this](Widget *, int){ doCreateAccount (); }, .parent{&m_canvas}}}
	, m_button2{{.x = 352, .y = 482, .texIDList{.off = 0X00000008, .on = 0X00000009, .down = 0X0000000A}, .onTrigger = [this](Widget *, int){ doChangePassword(); }, .parent{&m_canvas}}}
	, m_button3{{.x = 554, .y = 482, .texIDList{.off = 0X0000000B, .on = 0X0000000C, .down = 0X0000000D}, .onTrigger = [this](Widget *, int){ doExit ();          }, .parent{&m_canvas}}}
        , m_button4{{.x = 600, .y = 536, .texIDList{.off = 0X0000000E, .on = 0X0000000F, .down = 0X00000010}, .onTrigger = [this](Widget *, int){ doLogin();          }, .parent{&m_canvas}}}

	, m_idBox
          {{
              .x = 159,
              .y = 540,

              .w = 146,
              .h =  18,

              .font
              {
                  .id = 2,
                  .size = 18,
              },

              .onTab = [this]
              {
                  m_idBox      .setFocus(false);
                  m_passwordBox.setFocus(true);
              },

              .onCR = [this]
              {
                  doLogin();
              },

              .parent{&m_canvas},
          }}

	, m_passwordBox
          {{
              .x = 409,
              .y = 540,

              .w = 146,
              .h =  18,

              .font
              {
                  .id = 2,
                  .size = 18,
              },

              .onTab = [this]
              {
                  m_idBox      .setFocus(true);
                  m_passwordBox.setFocus(false);
              },

              .onCR = [this]
              {
                  doLogin();
              },

              .parent{&m_canvas},
          }}

    , m_buildSignature
      {{
          .textFunc = [](const Widget *)
          {
              return str_printf("编译版本号:%s", getBuildSignature());
          },

          .font
          {
              .id = 1,
              .size = 14,
              .color = colorf::YELLOW_A255,
          },

          .parent{&m_canvas},
      }}

    , m_notifyBoardBg
      {{
          .drawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::RGBA(0, 0,   0, 128), drawDstX, drawDstY, self->w(), self->h(), 8);
              g_sdlDevice->drawRectangle(colorf::RGBA(0, 0, 255, 128), drawDstX, drawDstY, self->w(), self->h(), 8);
          },

          .parent{&m_canvas},
      }}

    , m_notifyBoard
      {
          DIR_NONE,
          [this]{ return m_canvas.w() / 2; },
          [this]{ return m_canvas.h() / 2; },
          0, // single line

          1,
          15,
          0,

          colorf::YELLOW_A255,

          5000,
          10,

          &m_canvas,
          false,
      }
{
    m_notifyBoard  .setShow([this]{ return !m_notifyBoard.empty(); });
    m_notifyBoardBg.setShow([this]{ return !m_notifyBoard.empty(); });

    m_notifyBoardBg.moveAt(DIR_UPLEFT, [this]{ return m_notifyBoard.dx() - 10; }, [this]{ return m_notifyBoard.dy() - 10; });
    m_notifyBoardBg.setSize(           [this]{ return m_notifyBoard. w() + 20; }, [this]{ return m_notifyBoard. h() + 20; });

    g_sdlDevice->playBGM(g_bgmDB->retrieve(0X00040007));
    if(g_clientArgParser->autoLogin.has_value()){
        sendLogin(g_clientArgParser->autoLogin.value().first, g_clientArgParser->autoLogin.value().second);
    }
}

void ProcessLogin::update(double fUpdateTime)
{
    m_canvas.update(fUpdateTime);
}

void ProcessLogin::draw() const
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000003),   0,  75);
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000004),   0, 465);
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000011), 103, 536);

    m_canvas.drawRoot({});
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

    m_canvas.processEventRoot(event, true, {});
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
