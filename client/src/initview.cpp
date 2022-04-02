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

#include "log.hpp"
#include "strf.hpp"
#include "mathf.hpp"
#include "rawbuf.hpp"
#include "bevent.hpp"
#include "initview.hpp"
#include "fontexdb.hpp"
#include "pngtexdb.hpp"
#include "mapbindb.hpp"
#include "threadpool.hpp"
#include "emojidb.hpp"
#include "bgmusicdb.hpp"
#include "soundeffectdb.hpp"
#include "pngtexoffdb.hpp"

extern Log       *g_log;
extern XMLConf   *g_xmlConf;
extern SDLDevice *g_sdlDevice;

extern EmojiDB       *g_emojiDB;
extern BGMusicDB     *g_bgmDB;
extern SoundEffectDB *g_seffDB;

extern PNGTexDB *g_mapDB;
extern MapBinDB *g_mapBinDB;
extern FontexDB *g_fontexDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexDB *g_itemDB;

extern PNGTexOffDB *g_heroDB;
extern PNGTexOffDB *g_hairDB;
extern PNGTexOffDB *g_magicDB;
extern PNGTexOffDB *g_equipDB;
extern PNGTexOffDB *g_weaponDB;
extern PNGTexOffDB *g_helmetDB;
extern PNGTexOffDB *g_monsterDB;
extern PNGTexOffDB *g_standNPCDB;
extern PNGTexOffDB *g_selectCharDB;

InitView::InitView(uint8_t fontSize)
    : m_fontSize(fontSize)
    , m_taskList
      {
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_progUseDB,    "root/texture/progUseDB"   ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_itemDB,       "root/texture/itemDB"      ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_mapDB,        "root/texture/mapDB"       ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_fontexDB,     "root/font/fontexDB"       ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_heroDB,       "root/texture/heroDB"      ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_hairDB,       "root/texture/hairDB"      ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_monsterDB,    "root/texture/monsterDB"   ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_weaponDB,     "root/texture/weaponDB"    ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_helmetDB,     "root/texture/helmetDB"    ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_magicDB,      "root/texture/magicDB"     ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_equipDB,      "root/texture/equipDB"     ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_standNPCDB,   "root/texture/standNPCDB"  ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_selectCharDB, "root/texture/selectCharDB"); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_mapBinDB,     "root/map/mapBinDB"        ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_emojiDB,      "root/emoji/emojiDB"       ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_bgmDB,        "root/sound/bgmDB"         ); }},
          {1, [this](size_t weight){ loadDB(weight, g_xmlConf, g_seffDB,       "root/sound/seffDB"        ); }},
      }
{
    const Rawbuf boardData
    {
        #include "ivboard.inc"
    };

    const Rawbuf buttonData
    {
        #include "ivbutton.inc"
    };

    g_sdlDevice->createInitViewWindow();

    m_boardTexture  = g_sdlDevice->loadPNGTexture( boardData.data(),  boardData.size());
    m_buttonTexture = g_sdlDevice->loadPNGTexture(buttonData.data(), buttonData.size());

    fflassert(m_boardTexture);
    fflassert(m_buttonTexture);

    threadPool::abortedTag hasError;
    threadPool taskPool(threadPool::limitedThreadCount(m_taskList.size()));

    for(size_t i = 0; i < m_taskList.size(); ++i){
        taskPool.addTask(hasError, [i, this](int)
        {
            const auto &[weight, taskFunc] = m_taskList.at(i);
            taskFunc(weight);
        });
    }

    while(donePercent() < 100){
        processEvent(); // can abort internally
        draw();
    }

    taskPool.finish();
    hasError.checkError();
}

InitView::~InitView()
{
    for(auto &entry: m_logSink){
        if(entry.texture){
            SDL_DestroyTexture(entry.texture);
        }
    }

    if(m_boardTexture){
        SDL_DestroyTexture(m_boardTexture);
    }

    if(m_buttonTexture){
        SDL_DestroyTexture(m_buttonTexture);
    }
}

void InitView::processEvent()
{
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_MOUSEBUTTONUP:
                {
                    if(mathf::pointInRectangle(event.button.x, event.button.y, m_buttonX, m_buttonY, m_buttonW, m_buttonH)){
                        if(m_buttonState == BEVENT_DOWN){
                            std::exit(0);
                        }
                        else{
                            m_buttonState = BEVENT_ON;
                        }
                    }
                    else{
                        m_buttonState = BEVENT_OFF;
                    }
                    break;
                }
            case SDL_MOUSEBUTTONDOWN:
                {
                    switch(event.button.button){
                        case SDL_BUTTON_LEFT:
                            {
                                if(mathf::pointInRectangle(event.button.x, event.button.y, m_buttonX, m_buttonY, m_buttonW, m_buttonH)){
                                    m_buttonState = BEVENT_DOWN;
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
                    if(mathf::pointInRectangle(event.button.x, event.button.y, m_buttonX, m_buttonY, m_buttonW, m_buttonH)){
                        m_buttonState = BEVENT_ON;
                    }
                    else{
                        m_buttonState = BEVENT_OFF;
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

void InitView::addIVLog(int logType, const char *format, ...)
{
    std::string logStr;
    str_format(format, logStr);

    switch(logType){
        case LOGIV_INFO   : g_log->addLog(LOGTYPE_INFO,    "%s", logStr.c_str()); break;
        case LOGIV_WARNING: g_log->addLog(LOGTYPE_WARNING, "%s", logStr.c_str()); break;
        case LOGIV_FATAL  : g_log->addLog(LOGTYPE_FATAL,   "%s", logStr.c_str()); break;
        default: throw fflreach();
    }

    {
        std::lock_guard<std::mutex> lockGuard(m_lock);
        m_logSink.push_back(LogEntry
        {
            .type = logType,
            .log  = std::move(logStr),
        });
    }
}

void InitView::draw()
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(m_boardTexture, 0, 0);

    switch(m_buttonState){
        case BEVENT_ON:
            {
                g_sdlDevice->drawTexture(m_buttonTexture, m_buttonX, m_buttonY,  0, 0, 32, 30);
                break;
            }
        case BEVENT_DOWN:
            {
                g_sdlDevice->drawTexture(m_buttonTexture, m_buttonX, m_buttonY, 32, 0, 32, 30);
                break;
            }
        default:
            {
                break;
            }
    }

    const auto fnBuildLogTexture = [this](int logType, const std::string &log) -> SDL_Texture *
    {
        const auto color = [logType]() -> SDL_Color
        {
            switch(logType){
                case LOGIV_INFO   : return {0XFF, 0XFF, 0XFF, 0XFF};
                case LOGIV_WARNING: return {0XFF, 0XFF, 0X00, 0XFF};
                default           : return {0XFF, 0X00, 0X00, 0XFF};
            }
        }();

        SDL_Texture *texPtr = nullptr;
        if(auto surfPtr = TTF_RenderUTF8_Blended(g_sdlDevice->defaultTTF(m_fontSize), log.c_str(), color)){
            texPtr = g_sdlDevice->createTextureFromSurface(surfPtr);
            SDL_FreeSurface(surfPtr);
        }
        return texPtr;
    };

    std::array<SDL_Texture *, 6> texList;
    texList.fill(nullptr);
    {
        std::lock_guard<std::mutex> lockGuard(m_lock);
        {
            size_t startIndex = 0;
            if(m_logSink.size() > texList.size()){
                startIndex = m_logSink.size() - texList.size();
            }

            for(size_t i = startIndex, texIndex = 0; i < m_logSink.size(); ++i, ++texIndex){
                if(!m_logSink[i].texture){
                    m_logSink[i].texture = fnBuildLogTexture(m_logSink[i].type, m_logSink[i].log);
                }

                fflassert(m_logSink[i].texture);
                texList[texIndex] = m_logSink[i].texture;
            }
        }
    }

    int startX = 25;
    int startY = 25;

    for(auto texPtr: texList){
        if(texPtr){
            g_sdlDevice->drawTexture(texPtr, startX, startY);
            startY += (SDLDeviceHelper::getTextureHeight(texPtr) + 5);
        }
    }
}
