#include <regex>
#include <cstring>
#include <algorithm>

#include "log.hpp"
#include "idstrf.hpp"
#include "client.hpp"
#include "widget.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processchangepassword.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessChangePassword::ProcessChangePassword()
	: Process()
    , m_LBID           {{.label = u8"账号"    , .font{.size = 15}}}
    , m_LBPwd          {{.label = u8"密码"    , .font{.size = 15}}}
    , m_LBNewPwd       {{.label = u8"新密码"  , .font{.size = 15}}}
    , m_LBNewPwdConfirm{{.label = u8"确认密码", .font{.size = 15}}}
    , m_boxID
      {{
          .dir = DIR_LEFT,

          .x = m_x + 129 + 6, // offset + start of box in gfx + offset for input char
          .y = m_y +  79,

          .w = 186,
          .h =  28,

          .font
          {
              .id = 2,
              .size = 15,
          },

          .onTab = [this]
          {
              m_boxID           .setFocus(false);
              m_boxPwd          .setFocus(true );
              m_boxNewPwd       .setFocus(true );
              m_boxNewPwdConfirm.setFocus(false);
          },

          .onCR = [this]
          {
              doPostPasswordChange();
          },
      }}

    , m_boxPwd
      {{
          .dir = DIR_LEFT,

          .x = m_x + 129 + 6,
          .y = m_y + 126,

          .w = 186,
          .h =  28,

          .font
          {
              .id = 2,
              .size = 15,
          },

          .onTab = [this]
          {
              m_boxID           .setFocus(false);
              m_boxPwd          .setFocus(false);
              m_boxNewPwd       .setFocus(false);
              m_boxNewPwdConfirm.setFocus(true );
          },

          .onCR = [this]
          {
              doPostPasswordChange();
          },
      }}

    , m_boxNewPwd
      {{
          .dir = DIR_LEFT,

          .x = m_x + 129 + 6,
          .y = m_y + 173,

          .w = 186,
          .h =  28,

          .font
          {
              .id = 2,
              .size = 15,
          },

          .onTab = [this]
          {
              m_boxID           .setFocus(true );
              m_boxPwd          .setFocus(false);
              m_boxNewPwd       .setFocus(false);
              m_boxNewPwdConfirm.setFocus(false);
          },

          .onCR = [this]
          {
              doPostPasswordChange();
          },
      }}

    , m_boxNewPwdConfirm
      {{
          .dir = DIR_LEFT,

          .x = m_x + 129 + 6,
          .y = m_y + 220,

          .w = 186,
          .h =  28,

          .font
          {
              .id = 2,
              .size = 15,
          },

          .onTab = [this]
          {
              m_boxID           .setFocus(true );
              m_boxPwd          .setFocus(false);
              m_boxNewPwd       .setFocus(false);
              m_boxNewPwdConfirm.setFocus(false);
          },

          .onCR = [this]
          {
              doPostPasswordChange();
          },
      }}

    , m_LBCheckID           {{.font{.id = 0, .size = 15, .color = colorf::RGBA(0xFF, 0X00, 0X00, 0XFF)}}}
    , m_LBCheckPwd          {{.font{.id = 0, .size = 15, .color = colorf::RGBA(0xFF, 0X00, 0X00, 0XFF)}}}
    , m_LBCheckNewPwd       {{.font{.id = 0, .size = 15, .color = colorf::RGBA(0xFF, 0X00, 0X00, 0XFF)}}}
    , m_LBCheckNewPwdConfirm{{.font{.id = 0, .size = 15, .color = colorf::RGBA(0xFF, 0X00, 0X00, 0XFF)}}}

    , m_submit
      {{
          .x = m_x + 189,
          .y = m_y + 254,

          .texIDList
          {
              .on   = 0X0800000B,
              .down = 0X0800000C,
          },

          .onTrigger = [this](Widget *, int)
          {
              doPostPasswordChange();
          },

          .radioMode = true,
      }}

    , m_quit
      {{
          .x = m_x + 400,
          .y = m_y + 288,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              doExit();
          },

          .radioMode = true,
      }}

    , m_infoStr
      {{
          .dir = DIR_NONE,
          .x = 400,
          .y = 190,

          .font
          {
              .id = 1,
              .size = 15,
              .color = colorf::YELLOW_A255,
          },
      }}
{
    m_boxID.setFocus(true);
    m_boxPwd.setFocus(false);
    m_boxNewPwd.setFocus(false);
    m_boxNewPwdConfirm.setFocus(false);
}

void ProcessChangePassword::update(double fUpdateTime)
{
    m_boxID           .update(fUpdateTime);
    m_boxPwd          .update(fUpdateTime);
    m_boxNewPwd       .update(fUpdateTime);
    m_boxNewPwdConfirm.update(fUpdateTime);
}

void ProcessChangePassword::draw() const
{
    const SDLDeviceHelper::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000003), 0, 75);
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000004), 0, 75, 0, 0, 800, 450);

    m_boxID.drawRoot({});
    m_boxPwd.drawRoot({});
    m_boxNewPwd.drawRoot({});
    m_boxNewPwdConfirm.drawRoot({});
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X0A000001), m_x, m_y);

    const auto fnDrawInput = [](int x, int y, int dx, auto &title, auto &check)
    {
        //           (x, y)
        // +-------+  +-------------+  +-------+
        // |       |  |             |  |       |
        // | title |  x anhong      |  | check |
        // |       |  |             |  |       |
        // +-------+  +-------------+  +-------+
        //      -->|  |<--  186  -->|  |<--
        //          dx               dx

        title.draw({.dir=DIR_RIGHT, .x{x - dx      }, .y=y});
        check.draw({.dir=DIR_LEFT , .x{x + dx + 186}, .y=y});
    };

    fnDrawInput(m_x + 129, m_y +  79, 10, m_LBID           , m_LBCheckID           );
    fnDrawInput(m_x + 129, m_y + 126, 10, m_LBPwd          , m_LBCheckPwd          );
    fnDrawInput(m_x + 129, m_y + 173, 10, m_LBNewPwd       , m_LBCheckNewPwd       );
    fnDrawInput(m_x + 129, m_y + 220, 10, m_LBNewPwdConfirm, m_LBCheckNewPwdConfirm);

    m_submit.drawRoot({});
    m_quit.drawRoot({});

    if(hasInfo()){
        g_sdlDevice->fillRectangle(colorf::BLUE + colorf::A_SHF(32), 0, 75, 800, 450);
        m_infoStr.drawRoot({});
    }
}

void ProcessChangePassword::processEvent(const SDL_Event &event)
{
    if(m_quit.processEventRoot(event, true, {})){
        return;
    }

    if(hasInfo()){
        SDL_FlushEvent(SDL_KEYDOWN);
        return;
    }

    if(m_submit.processEventRoot(event, true, {})){
        return;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            Widget * boxPtrList[]
                            {
                                &m_boxID,
                                &m_boxPwd,
                                &m_boxNewPwd,
                                &m_boxNewPwdConfirm,
                            };

                            for(size_t i = 0; i < std::extent_v<decltype(boxPtrList)>; ++i){
                                if(boxPtrList[i]->focus()){
                                    for(size_t j = 0; j < std::extent_v<decltype(boxPtrList)>; ++j){
                                        boxPtrList[j]->setFocus(j == ((i + 1) % std::extent_v<decltype(boxPtrList)>));
                                    }
                                    break;
                                }
                            }
                            return;
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

    // widget idbox and pwdbox are not independent from each other
    // tab in one box will grant focus to another

    m_boxID           .processEventRoot(event, true, {});
    m_boxPwd          .processEventRoot(event, true, {});
    m_boxNewPwd       .processEventRoot(event, true, {});
    m_boxNewPwdConfirm.processEventRoot(event, true, {});

    localCheck();
}

void ProcessChangePassword::doExit()
{
    g_client->requestProcess(PROCESSID_LOGIN);
}

void ProcessChangePassword::doPostPasswordChange()
{
    const auto idStr = m_boxID.getRawString();
    const auto pwdStr = m_boxPwd.getPasswordString();
    const auto pwdNewStr = m_boxNewPwd.getPasswordString();
    const auto pwdNewConfirmStr = m_boxNewPwdConfirm.getPasswordString();

    if(!idstrf::isEmail(idStr.c_str())){
        setInfoStr(u8"无效账号", 2);
        clearInput();
        return;
    }

    if(!idstrf::isPassword(pwdStr.c_str())){
        setInfoStr(u8"无效密码", 2);
        m_boxPwd.clear();
        m_boxNewPwd.clear();
        m_boxNewPwdConfirm.clear();
        return;
    }

    if(!idstrf::isPassword(pwdNewStr.c_str())){
        setInfoStr(u8"无效新密码", 2);
        m_boxNewPwd.clear();
        m_boxNewPwdConfirm.clear();
        return;
    }

    if(pwdNewStr != pwdNewConfirmStr){
        setInfoStr(u8"新密码两次输入不一致", 2);
        m_boxNewPwd.clear();
        m_boxNewPwdConfirm.clear();
        return;
    }

    if(pwdStr == pwdNewStr){
        setInfoStr(u8"新旧密码相同", 2);
        m_boxNewPwd.clear();
        m_boxNewPwdConfirm.clear();
        return;
    }

    setInfoStr(u8"提交中");

    CMChangePassword cmCP;
    std::memset(&cmCP, 0, sizeof(cmCP));

    cmCP.id.assign(idStr);
    cmCP.password.assign(pwdStr);
    cmCP.passwordNew.assign(pwdNewStr);
    g_client->send({CM_CHANGEPASSWORD, cmCP});
}

void ProcessChangePassword::localCheck()
{
    const auto idStr = m_boxID.getRawString();
    const auto pwdStr = m_boxPwd.getPasswordString();
    const auto pwdNewStr = m_boxNewPwd.getPasswordString();
    const auto pwdNewConfirmStr = m_boxNewPwdConfirm.getPasswordString();

    const auto fnCheckInput = [](const std::string &s, auto &check, bool good)
    {
        if(s.empty()){
            check.setText(u8"");
        }
        else if(good){
            check.loadXML(to_cstr(str_printf(u8"<par><t color=\"green\">√</t></par>").c_str()));
        }
        else{
            check.loadXML(to_cstr(str_printf(u8"<par><t color=\"red\">×</t></par>").c_str()));
        }
    };

    fnCheckInput(idStr, m_LBCheckID, idstrf::isEmail(idStr.c_str()));
    fnCheckInput(pwdStr, m_LBCheckPwd, idstrf::isPassword(pwdStr.c_str()));
    fnCheckInput(pwdNewStr, m_LBCheckNewPwd, idstrf::isPassword(pwdNewStr.c_str()));
    fnCheckInput(pwdNewConfirmStr, m_LBCheckNewPwdConfirm, idstrf::isPassword(pwdNewConfirmStr.c_str()) && pwdNewStr == pwdNewConfirmStr);
}

void ProcessChangePassword::clearInput()
{
    m_boxID.clear();
    m_boxPwd.clear();
    m_boxNewPwd.clear();
    m_boxNewPwdConfirm.clear();
}

bool ProcessChangePassword::hasInfo() const
{
    if(m_infoStr.empty()){
        return false;
    }

    if(m_infoStrSec == 0){
        return true;
    }
    return m_infoStrTimer.diff_sec() < m_infoStrSec;
}

void ProcessChangePassword::setInfoStr(const char8_t *s)
{
    m_infoStr.setText(str_haschar(s) ? s : u8"");
    m_infoStrSec = 0;
}

void ProcessChangePassword::setInfoStr(const char8_t *s, uint32_t sec)
{
    m_infoStr.setText(str_haschar(s) ? s : u8"");
    m_infoStrSec = sec;
    m_infoStrTimer.reset();
}
