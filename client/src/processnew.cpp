/*
 * =====================================================================================
 *
 *       Filename: processnew.cpp
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

#include <regex>
#include <cstring>
#include <algorithm>

#include "log.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processnew.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessNew::ProcessNew()
	: Process()
    , m_LBID        (DIR_UPLEFT, 0, 0, u8"账号"    , 0, 15, 0, colorf::WHITE + 255)
    , m_LBPwd       (DIR_UPLEFT, 0, 0, u8"密码"    , 0, 15, 0, colorf::WHITE + 255)
    , m_LBPwdConfirm(DIR_UPLEFT, 0, 0, u8"确认密码", 0, 15, 0, colorf::WHITE + 255)
	, m_boxID
      {
          DIR_LEFT,
          m_x + 129 + 3, // offset + start of box in gfx + offset for input char
          m_y +  85,
          186,
          28,

          2,
          15,
          0,
          colorf::WHITE + 255,

          2,
          colorf::WHITE + 255,

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
          m_x + 129 + 3,
          m_y + 143,
          186,
          28,
          true,

          2,
          15,
          0,
          colorf::WHITE + 255,

          2,
          colorf::WHITE + 255,

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
          m_x + 129 + 3,
          m_y + 198,
          186,
          28,
          true,

          2,
          15,
          0,
          colorf::WHITE + 255,

          2,
          colorf::WHITE + 255,

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

    , m_LBCheckID        (DIR_UPLEFT, 0, 0, u8"√", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))
    , m_LBCheckPwd       (DIR_UPLEFT, 0, 0, u8"√", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))
    , m_LBCheckPwdConfirm(DIR_UPLEFT, 0, 0, u8"√", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0XFF))

    , m_submit
      {
          DIR_UPLEFT,
          m_x + 189,
          m_y + 233,
          {
              SYS_TEXNIL,
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
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

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
{}

void ProcessNew::update(double fUpdateTime)
{
    m_boxID        .update(fUpdateTime);
    m_boxPwd       .update(fUpdateTime);
    m_boxPwdConfirm.update(fUpdateTime);
}

void ProcessNew::draw()
{
    const SDLDeviceHelper::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(g_progUseDB->Retrieve(0X00000003), 0, 75);
    g_sdlDevice->drawTexture(g_progUseDB->Retrieve(0X00000004), 0, 75, 0, 0, 800, 450);

    m_boxID.draw();
    m_boxPwd.draw();
    m_boxPwdConfirm.draw();
    g_sdlDevice->drawTexture(g_progUseDB->Retrieve(0X0A000000), m_x, m_y);

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
}

void ProcessNew::processEvent(const SDL_Event &event)
{
    if(m_submit.processEvent(event, true)){
        return;
    }

    if(m_quit.processEvent(event, true)){
        return;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(true
                                    && !m_boxID.focus()
                                    && !m_boxPwd.focus()
                                    && !m_boxPwdConfirm.focus()){

                                m_boxID.focus(true);
                                m_boxPwd.focus(false);
                                m_boxPwdConfirm.focus(false);
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

    // widget idbox and pwdbox are not independent from each other
    // tab in one box will grant focus to another

    m_boxID        .processEvent(event, true);
    m_boxPwd       .processEvent(event, true);
    m_boxPwdConfirm.processEvent(event, true);
}

bool ProcessNew::localCheckID(const char *id) const
{
    if(str_haschar(id)){
        std::regex ptn
        {
            "(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"
        };
        return std::regex_match(id, ptn);
    }
    return false;
}

bool ProcessNew::localCheckPwd(const char *pwd) const
{
    return str_haschar(pwd);
}

void ProcessNew::doExit()
{
    g_client->RequestProcess(PROCESSID_LOGIN);
}

void ProcessNew::doPostAccount()
{
    postAccount(m_boxID.getRawString().c_str(), m_boxPwd.getRawString().c_str(), 1);
}

void ProcessNew::postAccount(const char *id, const char *pwd, int op)
{
    if(str_haschar(id) && str_haschar(pwd)){
        CMAccount cmA;
        std::memset(&cmA, 0, sizeof(cmA));

        std::strcpy(cmA.ID, id);
        std::strcpy(cmA.Password, pwd);

        cmA.Operation = op;
        g_client->send(CM_ACCOUNT, cmA);
    }
}
