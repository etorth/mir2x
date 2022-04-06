#include <regex>
#include <cstring>
#include <algorithm>

#include "log.hpp"
#include "idstrf.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processcreateaccount.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessCreateAccount::ProcessCreateAccount()
	: Process()
    , m_LBID        (DIR_UPLEFT, 0, 0, u8"账号"    , 1, 15, 0, colorf::WHITE + colorf::A_SHF(255))
    , m_LBPwd       (DIR_UPLEFT, 0, 0, u8"密码"    , 1, 15, 0, colorf::WHITE + colorf::A_SHF(255))
    , m_LBPwdConfirm(DIR_UPLEFT, 0, 0, u8"确认密码", 1, 15, 0, colorf::WHITE + colorf::A_SHF(255))
	, m_boxID
      {
          DIR_LEFT,
          m_x + 129 + 6, // offset + start of box in gfx + offset for input char
          m_y +  85,
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
              m_boxID        .focus(false);
              m_boxPwd       .focus(true );
              m_boxPwdConfirm.focus(false);
          },
          [this]()
          {
              doPostAccount();
          },
      }
	, m_boxPwd
      {
          DIR_LEFT,
          m_x + 129 + 6,
          m_y + 143,
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
              m_boxID        .focus(false);
              m_boxPwd       .focus(false);
              m_boxPwdConfirm.focus(true );
          },
          [this]()
          {
              doPostAccount();
          },
      }
	, m_boxPwdConfirm
      {
          DIR_LEFT,
          m_x + 129 + 6,
          m_y + 198,
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
              m_boxID        .focus(true );
              m_boxPwd       .focus(false);
              m_boxPwdConfirm.focus(false);
          },
          [this]()
          {
              doPostAccount();
          },
      }

    , m_LBCheckID        (DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))
    , m_LBCheckPwd       (DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))
    , m_LBCheckPwdConfirm(DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))

    , m_submit
      {
          DIR_UPLEFT,
          m_x + 189,
          m_y + 233,
          {
              SYS_U32NIL,
              0X0800000B,
              0X0800000C,
          },

          nullptr,
          nullptr,
          [this]()
          {
              doPostAccount();
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
          m_y + 267,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},

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
    m_boxPwdConfirm.focus(false);
}

void ProcessCreateAccount::update(double fUpdateTime)
{
    m_boxID        .update(fUpdateTime);
    m_boxPwd       .update(fUpdateTime);
    m_boxPwdConfirm.update(fUpdateTime);
}

void ProcessCreateAccount::draw() const
{
    const SDLDeviceHelper::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000003), 0, 75);
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000004), 0, 75, 0, 0, 800, 450);

    m_boxID.draw();
    m_boxPwd.draw();
    m_boxPwdConfirm.draw();
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X0A000000), m_x, m_y);

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

    fnDrawInput(m_x + 129, m_y +  85, 10, m_LBID        , m_LBCheckID        );
    fnDrawInput(m_x + 129, m_y + 143, 10, m_LBPwd       , m_LBCheckPwd       );
    fnDrawInput(m_x + 129, m_y + 198, 10, m_LBPwdConfirm, m_LBCheckPwdConfirm);

    m_submit.draw();
    m_quit.draw();

    if(hasInfo()){
        g_sdlDevice->fillRectangle(colorf::BLUE + colorf::A_SHF(32), 0, 75, 800, 450);
        m_infoStr.draw();
    }
}

void ProcessCreateAccount::processEvent(const SDL_Event &event)
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
                                &m_boxPwdConfirm,
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

    m_boxID        .processEvent(event, true);
    m_boxPwd       .processEvent(event, true);
    m_boxPwdConfirm.processEvent(event, true);

    localCheck();
}

void ProcessCreateAccount::doExit()
{
    g_client->requestProcess(PROCESSID_LOGIN);
}

void ProcessCreateAccount::doPostAccount()
{
    const auto idStr = m_boxID.getRawString();
    const auto pwdStr = m_boxPwd.getPasswordString();
    const auto pwdConfirmStr = m_boxPwdConfirm.getPasswordString();

    if(!idstrf::isEmail(idStr.c_str())){
        setInfoStr(u8"无效账号", 2);
        clearInput();
        return;
    }

    if(!idstrf::isPassword(pwdStr.c_str())){
        setInfoStr(u8"无效密码", 2);
        m_boxPwd.clear();
        m_boxPwdConfirm.clear();
        return;
    }

    if(pwdStr != pwdConfirmStr){
        setInfoStr(u8"两次密码输入不一致", 2);
        m_boxPwd.clear();
        m_boxPwdConfirm.clear();
        return;
    }

    setInfoStr(u8"提交中");

    CMCreateAccount cmCA;
    std::memset(&cmCA, 0, sizeof(cmCA));

    cmCA.id.assign(idStr);
    cmCA.password.assign(pwdStr);
    g_client->send(CM_CREATEACCOUNT, cmCA);
}

void ProcessCreateAccount::localCheck()
{
    const auto idStr = m_boxID.getRawString();
    const auto pwdStr = m_boxPwd.getPasswordString();
    const auto pwdConfirmStr = m_boxPwdConfirm.getPasswordString();

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
    fnCheckInput(pwdConfirmStr, m_LBCheckPwdConfirm, idstrf::isPassword(pwdConfirmStr.c_str()) && pwdStr == pwdConfirmStr);
}

void ProcessCreateAccount::clearInput()
{
    m_boxID.clear();
    m_boxPwd.clear();
    m_boxPwdConfirm.clear();
}

bool ProcessCreateAccount::hasInfo() const
{
    if(m_infoStr.empty()){
        return false;
    }

    if(m_infoStrSec == 0){
        return true;
    }
    return m_infoStrTimer.diff_sec() < m_infoStrSec;
}

void ProcessCreateAccount::setInfoStr(const char8_t *s)
{
    m_infoStr.setText(str_haschar(s) ? s : u8"");
    m_infoStrSec = 0;
}

void ProcessCreateAccount::setInfoStr(const char8_t *s, uint32_t sec)
{
    m_infoStr.setText(str_haschar(s) ? s : u8"");
    m_infoStrSec = sec;
    m_infoStrTimer.reset();
}
