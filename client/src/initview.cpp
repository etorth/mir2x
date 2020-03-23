/*
 * =====================================================================================
 *
 *       Filename: initview.cpp
 *        Created: 07/18/2017 16:04:25
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

#include <mutex>
#include <memory>
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "strfunc.hpp"
#include "mathfunc.hpp"
#include "initview.hpp"
#include "fontexdb.hpp"
#include "pngtexdb.hpp"
#include "mapbindb.hpp"
#include "emoticondb.hpp"
#include "pngtexoffdb.hpp"

extern Log *g_Log;
extern XMLConf *g_XMLConf;
extern SDLDevice *g_SDLDevice;
extern EmoticonDB *g_EmoticonDB;

extern PNGTexDB *g_MapDB;
extern MapBinDB *g_MapBinDB;
extern FontexDB *g_FontexDB;
extern PNGTexDB *g_ProgUseDB;
extern PNGTexDB *g_CommonItemDB;
extern PNGTexDB *g_GroundItemDB;

extern PNGTexOffDB *g_HeroDB;
extern PNGTexOffDB *g_MagicDB;
extern PNGTexOffDB *g_WeaponDB;
extern PNGTexOffDB *g_MonsterDB;

InitView::InitView(uint8_t nFontSize)
    : m_ProcState(IVPROC_LOOP)
    , m_ButtonState(0)
    , m_FontSize(nFontSize)
    , m_LoadProcV()
    , m_Lock()
    , m_MessageList()
    , m_TextureV {nullptr, nullptr}
{
    // 1. emplace all loading procedures
    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_ProgUseDB, "Root/Texture/ProgUseDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_GroundItemDB, "Root/Texture/GroundItemDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_CommonItemDB, "Root/Texture/CommonItemDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_MapDB, "Root/Texture/MapDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_FontexDB, "Root/Font/FontexDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_HeroDB, "Root/Texture/HeroDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_MonsterDB, "Root/Texture/MonsterDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_WeaponDB, "Root/Texture/WeaponDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_MagicDB, "Root/Texture/MagicDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_MapBinDB, "Root/Map/MapBinDB");
    });

    m_LoadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_EmoticonDB, "Root/Emoticon/Path");
    });

    // 2. loading font and textures
    static auto stBufU32V = []() -> std::array<std::vector<uint32_t>, 3>
    {
        // [0] : monaco ttf
        // [1] : error board texture top
        // [2] : error board texture middle
        std::array<std::vector<uint32_t>, 3> stBufU32V
        {{
            // binary format for .inc file
            // there could be serveral zeros at end
            // [0] : dataLen + N in bytes
            // [x] : data
            // [N] : zeros
            #include "initview.inc"
        }};

        for(auto &stBufU32: stBufU32V){
            if(stBufU32.empty() || ((size_t)((stBufU32[0] + 3) / 4) + 1) > stBufU32.size()){
                g_Log->AddLog(LOGTYPE_FATAL, "Invalid data in initview.inc");
            }
        }
        return stBufU32V;
    }();

    g_SDLDevice->CreateInitViewWindow();

    // create window before loading textures
    // in SDL2 textures binds to SDL_Renderer

    m_TextureV[0] = g_SDLDevice->CreateTexture((uint8_t *)(&(stBufU32V[1][1])), (size_t)(stBufU32V[1][0]));
    m_TextureV[1] = g_SDLDevice->CreateTexture((uint8_t *)(&(stBufU32V[2][1])), (size_t)(stBufU32V[2][0]));

    if(false
            || !m_TextureV[0]
            || !m_TextureV[1]){
        g_Log->AddLog(LOGTYPE_FATAL, "Build graphics resources failed for InitView");
    }

    static std::once_flag stOnceFlag;
    std::call_once(stOnceFlag, [this](){ Proc(); });
}

InitView::~InitView()
{
    for(auto &rstMessage: m_MessageList){
        if(rstMessage.Texture){
            SDL_DestroyTexture(rstMessage.Texture);
        }
    }

    if(m_TextureV[0]){
        SDL_DestroyTexture(m_TextureV[0]);
        m_TextureV[0] = nullptr;
    }

    if(m_TextureV[1]){
        SDL_DestroyTexture(m_TextureV[1]);
        m_TextureV[1] = nullptr;
    }
}

void InitView::ProcessEvent()
{
    int nX = m_ButtonX;
    int nY = m_ButtonY;
    int nW = m_ButtonW;
    int nH = m_ButtonH;

    SDL_Event stEvent;
    while(SDL_PollEvent(&stEvent)){
        switch(stEvent.type){
            case SDL_MOUSEBUTTONUP:
                {
                    if(MathFunc::PointInRectangle(stEvent.button.x, stEvent.button.y, nX, nY, nW, nH)){
                        if(m_ButtonState == 2){
                            std::exit(0);
                        }else{
                            m_ButtonState = 1;
                        }
                    }else{
                        m_ButtonState = 0;
                    }
                    break;
                }
            case SDL_MOUSEBUTTONDOWN:
                {
                    switch(stEvent.button.button){
                        case SDL_BUTTON_LEFT:
                            {
                                if(MathFunc::PointInRectangle(stEvent.button.x, stEvent.button.y, nX, nY, nW, nH)){
                                    m_ButtonState = 2;
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    break;
                }
            case SDL_MOUSEMOTION:
                {
                    if(MathFunc::PointInRectangle(stEvent.button.x, stEvent.button.y, nX, nY, nW, nH)){
                        m_ButtonState = 1;
                    }else{
                        m_ButtonState = 0;
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }
    }
}

void InitView::Proc()
{
    m_ProcState.store(IVPROC_LOOP);
    std::thread stThread([this](){ Load(); });

    while(true){
        if([this]() -> bool
        {
            switch(m_ProcState.load()){
                case IVPROC_DONE:
                    {
                        return true;
                    }
                default:
                    {
                        return false;
                    }
            }
        }()){ break; }

        ProcessEvent();
        Draw();
    }
    stThread.join();
}

void InitView::AddIVLog(int nLogType, const char *szLogFormat, ...)
{
    std::string szLog;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szLogFormat);

        try{
            szLog = str_vprintf(szLogFormat, ap);
        }catch(const std::exception &e){
            bError = true;
            szLog = str_printf("Exception caught in InitView::AddIVLog(\"%s\", ...): %s", szLogFormat, e.what());
        }

        va_end(ap);
    }

    if(bError){
        nLogType = LOGIV_WARNING;
    }

    switch(nLogType){
        case LOGIV_INFO:
            {
                g_Log->AddLog(LOGTYPE_INFO, "%s", szLog.c_str());
                break;
            }
        case LOGIV_WARNING:
            {
                g_Log->AddLog(LOGTYPE_WARNING, "%s", szLog.c_str());
                break;
            }
        case LOGIV_FATAL:
            {
                g_Log->AddLog(LOGTYPE_FATAL, "%s", szLog.c_str());
                break;
            }
        default:
            {
                g_Log->AddLog(LOGTYPE_WARNING, "Unknow LogType %d for message: %s", nLogType, szLog.c_str());
                return;
            }
    }

    {
        std::lock_guard<std::mutex> stLockGuard(m_Lock);
        m_MessageList.emplace_back(nLogType, szLog.c_str(), nullptr);
    }
}

void InitView::Load()
{
    for(size_t nIndex = 0; nIndex < m_LoadProcV.size(); ++nIndex){
        if(m_LoadProcV[nIndex].Event){
            if(!((m_LoadProcV[nIndex].Event)(nIndex))){
                m_ProcState.store(IVPROC_ERROR);
                return;
            }
        }
    }

    // let user see initialization done status
    AddIVLog(LOGIV_INFO, "[100%%]Loading done for InitView");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    m_ProcState.store(IVPROC_DONE);
}

void InitView::Draw()
{
    g_SDLDevice->ClearScreen();
    g_SDLDevice->DrawTexture(m_TextureV[0], 0, 0);

    int nX = m_ButtonX;
    int nY = m_ButtonY;
    switch(m_ButtonState){
        case 1:
            {
                g_SDLDevice->DrawTexture(m_TextureV[1], nX, nY,  0, 0, 32, 30);
                break;
            }
        case 2:
            {
                g_SDLDevice->DrawTexture(m_TextureV[1], nX, nY, 32, 0, 32, 30);
                break;
            }
        default:
            {
                break;
            }
    }

    auto fnCreateTexture = [this](int nLogType, const std::string &szMessage) -> SDL_Texture *
    {
        SDL_Texture *pTexture = nullptr;
        auto stColor = [nLogType]() -> SDL_Color
        {
            switch(nLogType){
                case 0  : return {0XFF, 0XFF, 0XFF, 0XFF};
                case 1  : return {0XFF, 0XFF, 0X00, 0XFF};
                default : return {0XFF, 0X00, 0X00, 0XFF};
            }
        }();

        if(auto pSurface = TTF_RenderUTF8_Blended(g_SDLDevice->DefaultTTF(m_FontSize), szMessage.c_str(), stColor)){
            pTexture = g_SDLDevice->CreateTextureFromSurface(pSurface);
            SDL_FreeSurface(pSurface);
        }
        return pTexture;
    };

    std::array<SDL_Texture *, 6> stTextureV;
    stTextureV.fill(nullptr);
    {
        std::lock_guard<std::mutex> stLockGuard(m_Lock);
        {
            size_t nStartIndex = 0;
            if(m_MessageList.size() > stTextureV.size()){
                nStartIndex = m_MessageList.size() - stTextureV.size();
            }

            for(size_t nIndex = nStartIndex, nTextureIndex = 0; nIndex < m_MessageList.size(); ++nIndex, ++nTextureIndex){
                if(nIndex < m_MessageList.size()){
                    if(!m_MessageList[nIndex].Texture){
                        m_MessageList[nIndex].Texture = fnCreateTexture(m_MessageList[nIndex].Type, m_MessageList[nIndex].Message.c_str());
                    }
                    stTextureV[nTextureIndex] = m_MessageList[nIndex].Texture;
                }
            }
        }
    }

    int nStartX = 25;
    int nStartY = 25;

    for(size_t nTextureIndex = 0; nTextureIndex < stTextureV.size(); ++nTextureIndex){
        if(stTextureV[nTextureIndex]){
            int nW = -1;
            int nH = -1;
            if(!SDL_QueryTexture(stTextureV[nTextureIndex], nullptr, nullptr, &nW, &nH)){
                g_SDLDevice->DrawTexture(stTextureV[nTextureIndex], nStartX, nStartY);
                nStartY += (nH + (nH / 3) + 2);
            }
        }
    }
    g_SDLDevice->Present();
}
