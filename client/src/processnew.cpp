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
#include "game.hpp"
#include "message.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "processnew.hpp"
#include "notifyboard.hpp"

ProcessNew::ProcessNew()
	: Process()
    , m_W(400)
    , m_H(300)

    , m_X([this]() -> int
      {
          extern SDLDevice *g_SDLDevice;
          return (g_SDLDevice->WindowW(false) - m_W) / 2;
      }())
    , m_Y([this]() -> int
      {
          extern SDLDevice *g_SDLDevice;
          return (g_SDLDevice->WindowH(false) - m_H) / 2;
      }())

    , m_CheckID(true)
    , m_CheckPwd(true)
    , m_CheckPwdConfirm(true)

    , m_LBID        (0, 0, "ID"              , 0, 15, 0, {0xFF, 0X00, 0X00, 0X00})
    , m_LBPwd       (0, 0, "Password"        , 0, 15, 0, {0xFF, 0X00, 0X00, 0X00})
    , m_LBPwdConfirm(0, 0, "Confirm Passowrd", 0, 15, 0, {0xFF, 0X00, 0X00, 0X00})

	, m_BoxID(
            159,
            540,
            146,
            18,
            2,
            1,
            14,
            {0XFF, 0XFF, 0XFF, 0XFF},
            {0XFF, 0XFF, 0XFF, 0XFF},
            [this]()
            {
                m_BoxID        .Focus(false);
                m_BoxPwd       .Focus(true );
                m_BoxPwdConfirm.Focus(false);
            },
            [this]()
            {
                DoPostAccount();
            })

	, m_BoxPwd(
            409,
            540,
            146,
            18,
            true,
            2,
            1,
            14,
            {0XFF, 0XFF, 0XFF, 0XFF},
            {0XFF, 0XFF, 0XFF, 0XFF},
            [this]()
            {
                m_BoxID        .Focus(false);
                m_BoxPwd       .Focus(false);
                m_BoxPwdConfirm.Focus(true );
            },
            [this]()
            {
                DoPostAccount();
            })

	, m_BoxPwdConfirm(
            409,
            540,
            146,
            18,
            true,
            2,
            1,
            14,
            {0XFF, 0XFF, 0XFF, 0XFF},
            {0XFF, 0XFF, 0XFF, 0XFF},
            [this]()
            {
                m_BoxID        .Focus(true );
                m_BoxPwd       .Focus(false);
                m_BoxPwdConfirm.Focus(false);
            },
            [this]()
            {
                DoPostAccount();
            })

    , m_LBCheckID        (0, 0, "ID", 0, 15, 0, {0xFF, 0X00, 0X00, 0X00})
    , m_LBCheckPwd       (0, 0, "ID", 0, 15, 0, {0xFF, 0X00, 0X00, 0X00})
    , m_LBCheckPwdConfirm(0, 0, "ID", 0, 15, 0, {0xFF, 0X00, 0X00, 0X00})

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
    extern SDLDevice *g_SDLDevice;
    extern PNGTexDBN *g_ProgUseDBN;

    g_SDLDevice->ClearScreen();

    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X00000003), 0, 75);
    g_SDLDevice->DrawTexture(g_ProgUseDBN->Retrieve(0X00000004), 0, 75, 0, 0, 800, 450);

    m_TBCreate.Draw();
    m_TBExit  .Draw();

    auto fnDrawInput = [](int nX, int nY, int nDX, auto &rstLB, auto &rstBox, auto &rstLBCheck)
    {
        //          (nX, nY)
        // +------+ x-------------+ +-----------+
        // |  ID  | | anhong      | | check ... |
        // +------+ +-------------+ +-----------+
        //     -->| |<--       -->| |<--
        //        nDX             nDX

        rstLB.DrawEx(nX - rstLB.W() - nDX, nY, 0, 0, rstLB.W(), rstLB.H());

        extern SDLDevice *g_SDLDevice;
        g_SDLDevice->DrawRectangle(nX, nY, rstBox.W(), rstBox.H());
        rstBox.DrawEx(nX, nY, 0, 0, rstBox.W(), rstBox.H());

        rstLBCheck.DrawEx(nX + rstBox.W() + nDX, nY, 0, 0, rstLBCheck.W(), rstLBCheck.H());
    };

    g_SDLDevice->PushColor(0X00, 0X80, 0X00, 0X00);
    fnDrawInput(300, 200, 10, m_LBID        , m_BoxID        , m_LBCheckID        );
    fnDrawInput(300, 300, 10, m_LBPwd       , m_BoxPwd       , m_LBCheckPwd       );
    fnDrawInput(300, 400, 10, m_LBPwdConfirm, m_BoxPwdConfirm, m_LBCheckPwdConfirm);
    g_SDLDevice->PopColor();

    g_SDLDevice->Present();
}

void ProcessNew::ProcessEvent(const SDL_Event &rstEvent)
{
    switch(rstEvent.type){
        case SDL_KEYDOWN:
            {
                switch(rstEvent.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(true
                                    && !m_BoxID       .Focus()
                                    && !m_BoxPwd.Focus()
                                    && !m_BoxPwdConfirm.Focus()){

                                m_BoxID       .Focus(true);
                                m_BoxPwd.Focus(false);
                                m_BoxPwdConfirm.Focus(false);
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

    m_TBCreate.ProcessEvent(rstEvent, nullptr);
    m_TBExit  .ProcessEvent(rstEvent, nullptr);

    // widget idbox and pwdbox are not independent from each other
    // tab in one box will grant focus to another

    bool bValid = true;
    m_BoxID        .ProcessEvent(rstEvent, &bValid);
    m_BoxPwd       .ProcessEvent(rstEvent, &bValid);
    m_BoxPwdConfirm.ProcessEvent(rstEvent, &bValid);

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
    extern Game *g_Game;
    g_Game->RequestProcess(PROCESSID_LOGIN);
}

void ProcessNew::DoPostAccount()
{
    if(false
            || m_CheckID
            || m_CheckPwd
            || m_CheckPwdConfirm){
        extern NotifyBoard *g_NotifyBoard;
        g_NotifyBoard->AddLog(LOGTYPE_WARNING, "Fix error before send request");
        return;
    }

    PostAccount(m_BoxID.Content().c_str(), m_BoxPwd.Content().c_str(), 1);
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

        extern Game *g_Game;
        g_Game->Send(CM_ACCOUNT, stCMA);
    }
}

void ProcessNew::CheckInput()
{
    auto szID         = m_BoxID.Content();
    auto szPwd        = m_BoxPwd.Content();
    auto szPwdConfirm = m_BoxPwdConfirm.Content();

    if(CacheFind(true, szID)){
        m_CheckID = CHECK_OK;
        m_LBCheckID.Clear();
    }else if(CacheFind(false, szID)){
        m_CheckID = CHECK_ERROR;
        m_LBCheckID.SetColor(ColorFunc::COLOR_RED);
        m_LBCheckID.FormatText ("ID has been used by others");
    }else{
        if(szID.empty()){
            m_CheckID = CHECK_NONE;
        }else{
            if(LocalCheckID(szID.c_str())){
                m_CheckID = CHECK_PENDING;
                m_LBCheckID.SetColor(ColorFunc::COLOR_GREEN);
                m_LBCheckID.FormatText ("Pending...");
            }else{
                m_CheckID = CHECK_ERROR;
                m_LBCheckID.SetColor(ColorFunc::COLOR_RED);
                m_LBCheckID.FormatText ("Invalid ID");
            }
        }
    }

    if(szPwd.empty()){
        m_CheckPwd = CHECK_NONE;
    }else{
        if(LocalCheckPwd(szPwd.c_str())){
            m_CheckPwd = CHECK_OK;
            m_LBCheckPwd.Clear();
        }else{
            m_CheckPwd = CHECK_ERROR;
            m_LBCheckPwd.SetColor(ColorFunc::COLOR_RED);
            m_LBCheckPwd.FormatText ("Invalid password");
        }
    }

    if(szPwdConfirm.empty()){
        m_CheckPwdConfirm = CHECK_NONE;
    }else{
        if(szPwdConfirm == szPwd){
            m_CheckPwdConfirm = CHECK_OK;
            m_LBCheckPwdConfirm.Clear();
        }else{
            m_CheckPwdConfirm = CHECK_ERROR;
            m_LBCheckPwdConfirm.SetColor(ColorFunc::COLOR_RED);
            m_LBCheckPwdConfirm.FormatText ("Password doesn't match");
        }
    }
}
