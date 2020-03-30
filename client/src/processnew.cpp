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
#include "notifyboard.hpp"

extern Client *g_Client;
extern SDLDevice *g_SDLDevice;
extern PNGTexDB *g_ProgUseDB;
extern NotifyBoard *g_NotifyBoard;

ProcessNew::ProcessNew()
	: Process()
    , m_w(400)
    , m_h(300)

    , m_x([this]() -> int
      {
          return (g_SDLDevice->WindowW(false) - m_w) / 2;
      }())
    , m_y([this]() -> int
      {
          return (g_SDLDevice->WindowH(false) - m_h) / 2;
      }())

    , m_CheckID(true)
    , m_CheckPwd(true)
    , m_CheckPwdConfirm(true)

    , m_LBID        (0, 0, "ID"              , 0, 15, 0, ColorFunc::RGBA(0xFF, 0X00, 0X00, 0X00))
    , m_LBPwd       (0, 0, "Password"        , 0, 15, 0, ColorFunc::RGBA(0xFF, 0X00, 0X00, 0X00))
    , m_LBPwdConfirm(0, 0, "Confirm Passowrd", 0, 15, 0, ColorFunc::RGBA(0xFF, 0X00, 0X00, 0X00))

	, m_BoxID
      {
          159,
          540,
          146,
          18,

          2,
          15,
          0,
          ColorFunc::WHITE,

          2,
          ColorFunc::WHITE,

          [this]()
          {
              m_BoxID        .focus(false);
              m_BoxPwd       .focus(true );
              m_BoxPwdConfirm.focus(false);
          },
          [this]()
          {
              DoPostAccount();
          },
      }
	, m_BoxPwd
      {
          409,
          540,
          146,
          18,
          true,

          2,
          15,
          0,
          ColorFunc::WHITE,

          2,
          ColorFunc::WHITE,

          [this]()
          {
              m_BoxID        .focus(false);
              m_BoxPwd       .focus(false);
              m_BoxPwdConfirm.focus(true );
          },
          [this]()
          {
              DoPostAccount();
          },
      }
	, m_BoxPwdConfirm
      {
          409,
          540,
          146,
          18,
          true,

          2,
          15,
          0,
          ColorFunc::WHITE,

          2,
          ColorFunc::WHITE,

          [this]()
          {
              m_BoxID        .focus(true );
              m_BoxPwd       .focus(false);
              m_BoxPwdConfirm.focus(false);
          },
          [this]()
          {
              DoPostAccount();
          },
      }

    , m_LBCheckID        (0, 0, "ID", 0, 15, 0, ColorFunc::RGBA(0xFF, 0X00, 0X00, 0X00))
    , m_LBCheckPwd       (0, 0, "ID", 0, 15, 0, ColorFunc::RGBA(0xFF, 0X00, 0X00, 0X00))
    , m_LBCheckPwdConfirm(0, 0, "ID", 0, 15, 0, ColorFunc::RGBA(0xFF, 0X00, 0X00, 0X00))

	, m_TBCreate(150, 482, 200, 40, "CREATE", 0, 16, 0, []{}, [this](){ DoPostAccount(); })
	, m_TBExit  (352, 482, 200, 40, "EXIT",   0, 16, 0, []{}, [this](){ DoExit();        })
{}

void ProcessNew::Update(double fMS)
{
    m_BoxID        .Update(fMS);
    m_BoxPwd       .Update(fMS);
    m_BoxPwdConfirm.Update(fMS);
}

void ProcessNew::Draw()
{
    g_SDLDevice->ClearScreen();

    g_SDLDevice->DrawTexture(g_ProgUseDB->Retrieve(0X00000003), 0, 75);
    g_SDLDevice->DrawTexture(g_ProgUseDB->Retrieve(0X00000004), 0, 75, 0, 0, 800, 450);

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

        rstLB.drawEx(nX - rstLB.W() - nDX, nY, 0, 0, rstLB.W(), rstLB.H());

        g_SDLDevice->DrawRectangle(nX, nY, rstBox.W(), rstBox.H());
        rstBox.drawEx(nX, nY, 0, 0, rstBox.W(), rstBox.H());

        rstLBCheck.drawEx(nX + rstBox.W() + nDX, nY, 0, 0, rstLBCheck.W(), rstLBCheck.H());
    };

    g_SDLDevice->PushColor(0X00, 0X80, 0X00, 0X00);
    fnDrawInput(300, 200, 10, m_LBID        , m_BoxID        , m_LBCheckID        );
    fnDrawInput(300, 300, 10, m_LBPwd       , m_BoxPwd       , m_LBCheckPwd       );
    fnDrawInput(300, 400, 10, m_LBPwdConfirm, m_BoxPwdConfirm, m_LBCheckPwdConfirm);
    g_SDLDevice->PopColor();

    g_SDLDevice->Present();
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
                                    && !m_BoxID       .focus()
                                    && !m_BoxPwd.focus()
                                    && !m_BoxPwdConfirm.focus()){

                                m_BoxID       .focus(true);
                                m_BoxPwd.focus(false);
                                m_BoxPwdConfirm.focus(false);
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

    m_BoxID        .processEvent(event, true);
    m_BoxPwd       .processEvent(event, true);
    m_BoxPwdConfirm.processEvent(event, true);

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
    g_Client->RequestProcess(PROCESSID_LOGIN);
}

void ProcessNew::DoPostAccount()
{
    if(false
            || m_CheckID
            || m_CheckPwd
            || m_CheckPwdConfirm){
        g_NotifyBoard->AddLog(LOGTYPE_WARNING, "Fix error before send request");
        return;
    }

    PostAccount(m_BoxID.getRawString().c_str(), m_BoxPwd.getRawString().c_str(), 1);
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
        g_Client->Send(CM_ACCOUNT, stCMA);
    }
}

void ProcessNew::CheckInput()
{
    auto szID         = m_BoxID.getRawString();
    auto szPwd        = m_BoxPwd.getRawString();
    auto szPwdConfirm = m_BoxPwdConfirm.getRawString();

    if(CacheFind(true, szID)){
        m_CheckID = CHECK_OK;
        m_LBCheckID.clear();
    }else if(CacheFind(false, szID)){
        m_CheckID = CHECK_ERROR;
        m_LBCheckID.SetFontColor(ColorFunc::RED);
        m_LBCheckID.setText("ID has been used by others");
    }else{
        if(szID.empty()){
            m_CheckID = CHECK_NONE;
        }else{
            if(LocalCheckID(szID.c_str())){
                m_CheckID = CHECK_PENDING;
                m_LBCheckID.SetFontColor(ColorFunc::GREEN);
                m_LBCheckID.setText("Pending...");
            }else{
                m_CheckID = CHECK_ERROR;
                m_LBCheckID.SetFontColor(ColorFunc::RED);
                m_LBCheckID.setText("Invalid ID");
            }
        }
    }

    if(szPwd.empty()){
        m_CheckPwd = CHECK_NONE;
    }else{
        if(LocalCheckPwd(szPwd.c_str())){
            m_CheckPwd = CHECK_OK;
            m_LBCheckPwd.clear();
        }else{
            m_CheckPwd = CHECK_ERROR;
            m_LBCheckPwd.SetFontColor(ColorFunc::RED);
            m_LBCheckPwd.setText("Invalid password");
        }
    }

    if(szPwdConfirm.empty()){
        m_CheckPwdConfirm = CHECK_NONE;
    }else{
        if(szPwdConfirm == szPwd){
            m_CheckPwdConfirm = CHECK_OK;
            m_LBCheckPwdConfirm.clear();
        }else{
            m_CheckPwdConfirm = CHECK_ERROR;
            m_LBCheckPwdConfirm.SetFontColor(ColorFunc::RED);
            m_LBCheckPwdConfirm.setText("Password doesn't match");
        }
    }
}
