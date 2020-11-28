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
#include <iostream>
#include <algorithm>

#include "log.hpp"
#include "client.hpp"
#include "message.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processnew.hpp"

extern Client *g_client;
extern SDLDevice *g_SDLDevice;
extern PNGTexDB *g_progUseDB;

ProcessNew::ProcessNew()
	: Process()
    , m_w(400)
    , m_h(300)

    , m_x([this]() -> int
      {
          return (g_SDLDevice->getRendererWidth() - m_w) / 2;
      }())
    , m_y([this]() -> int
      {
          return (g_SDLDevice->getRendererHeight() - m_h) / 2;
      }())

    , m_checkID(true)
    , m_checkPwd(true)
    , m_checkPwdConfirm(true)

    , m_LBID        (0, 0, u8"ID"              , 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0X00))
    , m_LBPwd       (0, 0, u8"Password"        , 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0X00))
    , m_LBPwdConfirm(0, 0, u8"Confirm Passowrd", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0X00))

	, m_boxID
      {
          159,
          540,
          146,
          18,

          2,
          15,
          0,
          colorf::WHITE,

          2,
          colorf::WHITE,

          [this]()
          {
              m_boxID        .focus(false);
              m_boxPwd       .focus(true );
              m_boxPwdConfirm.focus(false);
          },
          [this]()
          {
              DoPostAccount();
          },
      }
	, m_boxPwd
      {
          409,
          540,
          146,
          18,
          true,

          2,
          15,
          0,
          colorf::WHITE,

          2,
          colorf::WHITE,

          [this]()
          {
              m_boxID        .focus(false);
              m_boxPwd       .focus(false);
              m_boxPwdConfirm.focus(true );
          },
          [this]()
          {
              DoPostAccount();
          },
      }
	, m_boxPwdConfirm
      {
          409,
          540,
          146,
          18,
          true,

          2,
          15,
          0,
          colorf::WHITE,

          2,
          colorf::WHITE,

          [this]()
          {
              m_boxID        .focus(true );
              m_boxPwd       .focus(false);
              m_boxPwdConfirm.focus(false);
          },
          [this]()
          {
              DoPostAccount();
          },
      }

    , m_LBCheckID        (0, 0, u8"ID", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0X00))
    , m_LBCheckPwd       (0, 0, u8"ID", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0X00))
    , m_LBCheckPwdConfirm(0, 0, u8"ID", 0, 15, 0, colorf::RGBA(0xFF, 0X00, 0X00, 0X00))

	, m_TBCreate(150, 482, 200, 40, u8"CREATE", 0, 16, 0, []{}, [this](){ DoPostAccount(); })
	, m_TBExit  (352, 482, 200, 40, u8"EXIT",   0, 16, 0, []{}, [this](){ DoExit();        })
{}

void ProcessNew::update(double fUpdateTime)
{
    m_boxID        .update(fUpdateTime);
    m_boxPwd       .update(fUpdateTime);
    m_boxPwdConfirm.update(fUpdateTime);
}

void ProcessNew::draw()
{
    SDLDevice::RenderNewFrame newFrame;

    g_SDLDevice->drawTexture(g_progUseDB->Retrieve(0X00000003), 0, 75);
    g_SDLDevice->drawTexture(g_progUseDB->Retrieve(0X00000004), 0, 75, 0, 0, 800, 450);

    m_TBCreate.draw();
    m_TBExit  .draw();

    auto fnDrawInput = [](int nX, int nY, int nDX, auto &rstLB, auto &rstBox, auto &rstLBCheck)
    {
        //          (nX, nY)
        // +------+ x-------------+ +-----------+
        // |  ID  | | anhong      | | check ... |
        // +------+ +-------------+ +-----------+
        //     -->| |<--       -->| |<--
        //        nDX             nDX

        rstLB.drawEx(nX - rstLB.w() - nDX, nY, 0, 0, rstLB.w(), rstLB.h());

        g_SDLDevice->DrawRectangle(nX, nY, rstBox.w(), rstBox.h());
        rstBox.drawEx(nX, nY, 0, 0, rstBox.w(), rstBox.h());

        rstLBCheck.drawEx(nX + rstBox.w() + nDX, nY, 0, 0, rstLBCheck.w(), rstLBCheck.h());
    };

    SDLDevice::EnableDrawColor drawColor(colorf::RGBA(0X00, 0X80, 0X00, 0X00));
    fnDrawInput(300, 200, 10, m_LBID        , m_boxID        , m_LBCheckID        );
    fnDrawInput(300, 300, 10, m_LBPwd       , m_boxPwd       , m_LBCheckPwd       );
    fnDrawInput(300, 400, 10, m_LBPwdConfirm, m_boxPwdConfirm, m_LBCheckPwdConfirm);
}

void ProcessNew::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(true
                                    && !m_boxID       .focus()
                                    && !m_boxPwd.focus()
                                    && !m_boxPwdConfirm.focus()){

                                m_boxID       .focus(true);
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

    m_TBCreate.processEvent(event, true);
    m_TBExit  .processEvent(event, true);

    // widget idbox and pwdbox are not independent from each other
    // tab in one box will grant focus to another

    m_boxID        .processEvent(event, true);
    m_boxPwd       .processEvent(event, true);
    m_boxPwdConfirm.processEvent(event, true);

    CheckInput();
}

bool ProcessNew::LocalCheckID(const char *szID)
{
    if(szID && std::strlen(szID)){
        std::regex stPattern
        {
            "(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"
        };
        return std::regex_match(szID, stPattern);
    }
    return false;
}

bool ProcessNew::LocalCheckPwd(const char *szPwd)
{
    if(szPwd){
        return std::strlen(szPwd) < 16;
    }
    return false;
}

void ProcessNew::DoExit()
{
    g_client->RequestProcess(PROCESSID_LOGIN);
}

void ProcessNew::DoPostAccount()
{
    if(false
            || m_checkID
            || m_checkPwd
            || m_checkPwdConfirm){
        return;
    }

    PostAccount(m_boxID.getRawString().c_str(), m_boxPwd.getRawString().c_str(), 1);
}

void ProcessNew::PostAccount(const char *szID, const char *szPWD, int nOperation)
{
    if(true
            && szID  && std::strlen(szID)
            && szPWD && std::strlen(szPWD)){

        CMAccount stCMA;
        std::memset(&stCMA, 0, sizeof(stCMA));

        std::strcpy(stCMA.ID, szID);
        std::strcpy(stCMA.Password, szPWD);

        stCMA.Operation = nOperation;
        g_client->send(CM_ACCOUNT, stCMA);
    }
}

void ProcessNew::CheckInput()
{
    auto szID         = m_boxID.getRawString();
    auto szPwd        = m_boxPwd.getRawString();
    auto szPwdConfirm = m_boxPwdConfirm.getRawString();

    if(CacheFind(true, szID)){
        m_checkID = CHECK_OK;
        m_LBCheckID.clear();
    }else if(CacheFind(false, szID)){
        m_checkID = CHECK_ERROR;
        m_LBCheckID.setFontColor(colorf::RED);
        m_LBCheckID.setText(u8"ID has been used by others");
    }else{
        if(szID.empty()){
            m_checkID = CHECK_NONE;
        }else{
            if(LocalCheckID(szID.c_str())){
                m_checkID = CHECK_PENDING;
                m_LBCheckID.setFontColor(colorf::GREEN);
                m_LBCheckID.setText(u8"Pending...");
            }else{
                m_checkID = CHECK_ERROR;
                m_LBCheckID.setFontColor(colorf::RED);
                m_LBCheckID.setText(u8"Invalid ID");
            }
        }
    }

    if(szPwd.empty()){
        m_checkPwd = CHECK_NONE;
    }else{
        if(LocalCheckPwd(szPwd.c_str())){
            m_checkPwd = CHECK_OK;
            m_LBCheckPwd.clear();
        }else{
            m_checkPwd = CHECK_ERROR;
            m_LBCheckPwd.setFontColor(colorf::RED);
            m_LBCheckPwd.setText(u8"Invalid password");
        }
    }

    if(szPwdConfirm.empty()){
        m_checkPwdConfirm = CHECK_NONE;
    }else{
        if(szPwdConfirm == szPwd){
            m_checkPwdConfirm = CHECK_OK;
            m_LBCheckPwdConfirm.clear();
        }else{
            m_checkPwdConfirm = CHECK_ERROR;
            m_LBCheckPwdConfirm.setFontColor(colorf::RED);
            m_LBCheckPwdConfirm.setText(u8"Password doesn't match");
        }
    }
}
