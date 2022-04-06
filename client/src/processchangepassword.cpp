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
    , m_LBID           (DIR_UPLEFT, 0, 0, u8"账号"    , 1, 15, 0, colorf::WHITE + colorf::A_SHF(255))
    , m_LBPwd          (DIR_UPLEFT, 0, 0, u8"密码"    , 1, 15, 0, colorf::WHITE + colorf::A_SHF(255))
    , m_LBNewPwd       (DIR_UPLEFT, 0, 0, u8"新密码"  , 1, 15, 0, colorf::WHITE + colorf::A_SHF(255))
    , m_LBNewPwdConfirm(DIR_UPLEFT, 0, 0, u8"确认密码", 1, 15, 0, colorf::WHITE + colorf::A_SHF(255))
	, m_boxID
      {
          DIR_LEFT,
          m_x + 129 + 6, // offset + start of box in gfx + offset for input char
          m_y +  79,
          186,
          28,

          2,
          15,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          [this]()
          {
              m_boxID           .focus(false);
              m_boxPwd          .focus(true );
              m_boxNewPwd       .focus(true );
              m_boxNewPwdConfirm.focus(false);
          },
          [this]()
          {
              doPostPasswordChange();
          },
      }
	, m_boxPwd
      {
          DIR_LEFT,
          m_x + 129 + 6,
          m_y + 126,
          186,
          28,
          true,

          2,
          15,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          [this]()
          {
              m_boxID           .focus(false);
              m_boxPwd          .focus(false);
              m_boxNewPwd       .focus(false);
              m_boxNewPwdConfirm.focus(true );
          },
          [this]()
          {
              doPostPasswordChange();
          },
      }
	, m_boxNewPwd
      {
          DIR_LEFT,
          m_x + 129 + 6,
          m_y + 173,
          186,
          28,
          true,

          2,
          15,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          [this]()
          {
              m_boxID           .focus(true );
              m_boxPwd          .focus(false);
              m_boxNewPwd       .focus(false);
              m_boxNewPwdConfirm.focus(false);
          },
          [this]()
          {
              doPostPasswordChange();
          },
      }
	, m_boxNewPwdConfirm
      {
          DIR_LEFT,
          m_x + 129 + 6,
          m_y + 220,
          186,
          28,
          true,

          2,
          15,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          [this]()
          {
              m_boxID           .focus(true );
              m_boxPwd          .focus(false);
              m_boxNewPwd       .focus(false);
              m_boxNewPwdConfirm.focus(false);
          },
          [this]()
          {
              doPostPasswordChange();
          },
      }

    , m_LBCheckID           (DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))
    , m_LBCheckPwd          (DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))
    , m_LBCheckNewPwd       (DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))
    , m_LBCheckNewPwdConfirm(DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))

    , m_submit
      {
          DIR_UPLEFT,
          m_x + 189,
          m_y + 254,
          {
              SYS_U32NIL,
              0X0800000B,
              0X0800000C,
          },

          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              doPostPasswordChange();
          },

          0,
          0,
          0,
          0,

          true,
          true,
      }

    , m_quit
      {
          DIR_UPLEFT,
          m_x + 400,
          m_y + 288,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              doExit();
          },

          0,
          0,
          0,
          0,

          true,
          true,
      }

    , m_infoStr
      {
          DIR_NONE,
          400,
          190,

          u8"",
          1,
          15,
          0,
          colorf::YELLOW + colorf::A_SHF(255)
      }
{
    m_boxID.focus(true);
    m_boxPwd.focus(false);
    m_boxNewPwd.focus(false);
    m_boxNewPwdConfirm.focus(false);
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

    m_boxID.draw();
    m_boxPwd.draw();
    m_boxNewPwd.draw();
    m_boxNewPwdConfirm.draw();
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

        title.drawAt(DIR_RIGHT, x - dx      , y);
        check.drawAt(DIR_LEFT , x + dx + 186, y);
    };

    fnDrawInput(m_x + 129, m_y +  79, 10, m_LBID           , m_LBCheckID           );
    fnDrawInput(m_x + 129, m_y + 126, 10, m_LBPwd          , m_LBCheckPwd          );
    fnDrawInput(m_x + 129, m_y + 173, 10, m_LBNewPwd       , m_LBCheckNewPwd       );
    fnDrawInput(m_x + 129, m_y + 220, 10, m_LBNewPwdConfirm, m_LBCheckNewPwdConfirm);

    m_submit.draw();
    m_quit.draw();

    if(hasInfo()){
        g_sdlDevice->fillRectangle(colorf::BLUE + colorf::A_SHF(32), 0, 75, 800, 450);
        m_infoStr.draw();
    }
}

void ProcessChangePassword::processEvent(const SDL_Event &event)
{
    if(m_quit.processEvent(event, true)){
        return;
    }

    if(hasInfo()){
        SDL_FlushEvent(SDL_KEYDOWN);
        return;
    }

    if(m_submit.processEvent(event, true)){
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
                                        boxPtrList[j]->focus(j == ((i + 1) % std::extent_v<decltype(boxPtrList)>));
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

    m_boxID           .processEvent(event, true);
    m_boxPwd          .processEvent(event, true);
    m_boxNewPwd       .processEvent(event, true);
    m_boxNewPwdConfirm.processEvent(event, true);

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
    g_client->send(CM_CHANGEPASSWORD, cmCP);
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
