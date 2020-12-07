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

#include "strf.hpp"
#include "mathf.hpp"
#include "initview.hpp"
#include "fontexdb.hpp"
#include "pngtexdb.hpp"
#include "mapbindb.hpp"
#include "emoticondb.hpp"
#include "pngtexoffdb.hpp"

extern Log *g_log;
extern XMLConf *g_XMLConf;
extern SDLDevice *g_sdlDevice;
extern emoticonDB *g_emoticonDB;

extern PNGTexDB *g_mapDB;
extern MapBinDB *g_mapBinDB;
extern FontexDB *g_fontexDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexDB *g_commonItemDB;
extern PNGTexDB *g_groundItemDB;

extern PNGTexOffDB *g_heroDB;
extern PNGTexOffDB *g_magicDB;
extern PNGTexOffDB *g_weaponDB;
extern PNGTexOffDB *g_monsterDB;
extern PNGTexOffDB *g_standNPCDB;

InitView::InitView(uint8_t nFontSize)
    : m_procState(IVPROC_LOOP)
    , m_buttonState(0)
    , m_fontSize(nFontSize)
    , m_loadProcV()
    , m_lock()
    , m_messageList()
    , m_textureV {nullptr, nullptr}
{
    // 1. emplace all loading procedures
    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_progUseDB, "Root/Texture/ProgUseDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_groundItemDB, "Root/Texture/GroundItemDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_commonItemDB, "Root/Texture/CommonItemDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_mapDB, "Root/Texture/MapDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_fontexDB, "Root/Font/FontexDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_heroDB, "Root/Texture/HeroDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_monsterDB, "Root/Texture/MonsterDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_weaponDB, "Root/Texture/WeaponDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_magicDB, "Root/Texture/MagicDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_standNPCDB, "Root/Texture/standNPCDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_mapBinDB, "Root/Map/MapBinDB");
    });

    m_loadProcV.emplace_back(1, [this](size_t nIndex) -> bool
    {
        return LoadDB(nIndex, g_XMLConf, g_emoticonDB, "Root/Emoticon/Path");
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
                throw fflerror("invalid data in initview.inc");
            }
        }
        return stBufU32V;
    }();

    g_sdlDevice->CreateInitViewWindow();

    // create window before loading textures
    // in SDL2 textures binds to SDL_Renderer

    m_textureV[0] = g_sdlDevice->CreateTexture((uint8_t *)(&(stBufU32V[1][1])), (size_t)(stBufU32V[1][0]));
    m_textureV[1] = g_sdlDevice->CreateTexture((uint8_t *)(&(stBufU32V[2][1])), (size_t)(stBufU32V[2][0]));

    if(false
            || !m_textureV[0]
            || !m_textureV[1]){
        throw fflerror("build graphics resources failed for InitView");
    }

    static std::once_flag stOnceFlag;
    std::call_once(stOnceFlag, [this](){ Proc(); });
}

InitView::~InitView()
{
    for(auto &rstMessage: m_messageList){
        if(rstMessage.Texture){
            SDL_DestroyTexture(rstMessage.Texture);
        }
    }

    if(m_textureV[0]){
        SDL_DestroyTexture(m_textureV[0]);
        m_textureV[0] = nullptr;
    }

    if(m_textureV[1]){
        SDL_DestroyTexture(m_textureV[1]);
        m_textureV[1] = nullptr;
    }
}

void InitView::processEvent()
{
    int nX = m_buttonX;
    int nY = m_buttonY;
    int nW = m_buttonW;
    int nH = m_buttonH;

    SDL_Event stEvent;
    while(SDL_PollEvent(&stEvent)){
        switch(stEvent.type){
            case SDL_MOUSEBUTTONUP:
                {
                    if(mathf::pointInRectangle(stEvent.button.x, stEvent.button.y, nX, nY, nW, nH)){
                        if(m_buttonState == 2){
                            std::exit(0);
                        }else{
                            m_buttonState = 1;
                        }
                    }else{
                        m_buttonState = 0;
                    }
                    break;
                }
            case SDL_MOUSEBUTTONDOWN:
                {
                    switch(stEvent.button.button){
                        case SDL_BUTTON_LEFT:
                            {
                                if(mathf::pointInRectangle(stEvent.button.x, stEvent.button.y, nX, nY, nW, nH)){
                                    m_buttonState = 2;
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
                    if(mathf::pointInRectangle(stEvent.button.x, stEvent.button.y, nX, nY, nW, nH)){
                        m_buttonState = 1;
                    }else{
                        m_buttonState = 0;
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
    m_procState.store(IVPROC_LOOP);
    std::thread stThread([this](){ Load(); });

    while(true){
        if([this]() -> bool
        {
            switch(m_procState.load()){
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

        processEvent();
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
                g_log->addLog(LOGTYPE_INFO, "%s", szLog.c_str());
                break;
            }
        case LOGIV_WARNING:
            {
                g_log->addLog(LOGTYPE_WARNING, "%s", szLog.c_str());
                break;
            }
        case LOGIV_FATAL:
            {
                g_log->addLog(LOGTYPE_FATAL, "%s", szLog.c_str());
                break;
            }
        default:
            {
                g_log->addLog(LOGTYPE_WARNING, "Unknow LogType %d for message: %s", nLogType, szLog.c_str());
                return;
            }
    }

    {
        std::lock_guard<std::mutex> stLockGuard(m_lock);
        m_messageList.emplace_back(nLogType, szLog.c_str(), nullptr);
    }
}

void InitView::Load()
{
    for(size_t nIndex = 0; nIndex < m_loadProcV.size(); ++nIndex){
        if(m_loadProcV[nIndex].Event){
            if(!((m_loadProcV[nIndex].Event)(nIndex))){
                m_procState.store(IVPROC_ERROR);
                return;
            }
        }
    }

    // let user see initialization done status
    AddIVLog(LOGIV_INFO, "[100%%]Loading done for InitView");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    m_procState.store(IVPROC_DONE);
}

void InitView::Draw()
{
    SDLDevice::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(m_textureV[0], 0, 0);

    int nX = m_buttonX;
    int nY = m_buttonY;
    switch(m_buttonState){
        case 1:
            {
                g_sdlDevice->drawTexture(m_textureV[1], nX, nY,  0, 0, 32, 30);
                break;
            }
        case 2:
            {
                g_sdlDevice->drawTexture(m_textureV[1], nX, nY, 32, 0, 32, 30);
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

        if(auto pSurface = TTF_RenderUTF8_Blended(g_sdlDevice->DefaultTTF(m_fontSize), szMessage.c_str(), stColor)){
            pTexture = g_sdlDevice->CreateTextureFromSurface(pSurface);
            SDL_FreeSurface(pSurface);
        }
        return pTexture;
    };

    std::array<SDL_Texture *, 6> stTextureV;
    stTextureV.fill(nullptr);
    {
        std::lock_guard<std::mutex> stLockGuard(m_lock);
        {
            size_t nStartIndex = 0;
            if(m_messageList.size() > stTextureV.size()){
                nStartIndex = m_messageList.size() - stTextureV.size();
            }

            for(size_t nIndex = nStartIndex, nTextureIndex = 0; nIndex < m_messageList.size(); ++nIndex, ++nTextureIndex){
                if(nIndex < m_messageList.size()){
                    if(!m_messageList[nIndex].Texture){
                        m_messageList[nIndex].Texture = fnCreateTexture(m_messageList[nIndex].Type, m_messageList[nIndex].Message.c_str());
                    }
                    stTextureV[nTextureIndex] = m_messageList[nIndex].Texture;
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
                g_sdlDevice->drawTexture(stTextureV[nTextureIndex], nStartX, nStartY);
                nStartY += (nH + (nH / 3) + 2);
            }
        }
    }
}
