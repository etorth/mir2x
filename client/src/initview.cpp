#include <mutex>
#include <memory>
#include <chrono>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

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
#include "clientargparser.hpp"

extern Log *g_mir2xLog;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

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
          {1, [this](size_t weight){ loadDB(weight, g_progUseDB,    g_clientArgParser->resPath, "texture/proguse.zsdb"   ); }},
          {1, [this](size_t weight){ loadDB(weight, g_itemDB,       g_clientArgParser->resPath, "texture/item.zsdb"      ); }},
          {1, [this](size_t weight){ loadDB(weight, g_mapDB,        g_clientArgParser->resPath, "texture/map.zsdb"       ); }},
          {1, [this](size_t weight){ loadDB(weight, g_heroDB,       g_clientArgParser->resPath, "texture/hero.zsdb"      ); }},
          {1, [this](size_t weight){ loadDB(weight, g_hairDB,       g_clientArgParser->resPath, "texture/hair.zsdb"      ); }},
          {1, [this](size_t weight){ loadDB(weight, g_monsterDB,    g_clientArgParser->resPath, "texture/monster.zsdb"   ); }},
          {1, [this](size_t weight){ loadDB(weight, g_weaponDB,     g_clientArgParser->resPath, "texture/weapon.zsdb"    ); }},
          {1, [this](size_t weight){ loadDB(weight, g_helmetDB,     g_clientArgParser->resPath, "texture/helmet.zsdb"    ); }},
          {1, [this](size_t weight){ loadDB(weight, g_magicDB,      g_clientArgParser->resPath, "texture/magic.zsdb"     ); }},
          {1, [this](size_t weight){ loadDB(weight, g_equipDB,      g_clientArgParser->resPath, "texture/equip.zsdb"     ); }},
          {1, [this](size_t weight){ loadDB(weight, g_standNPCDB,   g_clientArgParser->resPath, "texture/npc.zsdb"       ); }},
          {1, [this](size_t weight){ loadDB(weight, g_selectCharDB, g_clientArgParser->resPath, "texture/selectchar.zsdb"); }},
          {1, [this](size_t weight){ loadDB(weight, g_fontexDB,     g_clientArgParser->resPath, "font/fontex.zsdb"       ); }},
          {1, [this](size_t weight){ loadDB(weight, g_mapBinDB,     g_clientArgParser->resPath, "map/mapbin.zsdb"        ); }},
          {1, [this](size_t weight){ loadDB(weight, g_emojiDB,      g_clientArgParser->resPath, "emoji/emoji.zsdb"       ); }},
          {1, [this](size_t weight){ loadDB(weight, g_bgmDB,        g_clientArgParser->resPath, "sound/bgm.zsdb"         ); }},
          {1, [this](size_t weight){ loadDB(weight, g_seffDB,       g_clientArgParser->resPath, "sound/seff.zsdb"        ); }},
      }
{
    constexpr uint8_t boardData []
    {
        #embed "ivboard.png"
    };

    constexpr uint8_t buttonData []
    {
        #embed "ivbutton.png"
    };

    g_sdlDevice->createInitViewWindow();

    m_boardTexture  = g_sdlDevice->loadPNGTexture(std::data( boardData), std::size( boardData));
    m_buttonTexture = g_sdlDevice->loadPNGTexture(std::data(buttonData), std::size(buttonData));

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

    // Wait for tasks to finish. If any task throws, loadDB logs the specific
    // failure via addIVLog (visible on the loading window) then rethrows;
    // threadCBWrapper captures the exception into hasError, and the
    // per-iteration checkError() rethrows here so the loop bails without
    // deadlocking on donePercent() < 100 (m_doneWeight is not incremented for
    // the failed task).
    while(donePercent() < 100){
        processEvent(); // can abort internally
        draw();
        hasError.checkError();
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
            case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    if(mathf::pointInRectangle(to_d(event.button.x), to_d(event.button.y), m_buttonX, m_buttonY, m_buttonW, m_buttonH)){
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
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    switch(event.button.button){
                        case SDL_BUTTON_LEFT:
                            {
                                if(mathf::pointInRectangle(to_d(event.button.x), to_d(event.button.y), m_buttonX, m_buttonY, m_buttonW, m_buttonH)){
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
            case SDL_EVENT_MOUSE_MOTION:
                {
                    if(mathf::pointInRectangle(to_d(event.button.x), to_d(event.button.y), m_buttonX, m_buttonY, m_buttonW, m_buttonH)){
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
        case LOGIV_INFO   : g_mir2xLog->addLog(LOGTYPE_INFO,    "%s", logStr.c_str()); break;
        case LOGIV_WARNING: g_mir2xLog->addLog(LOGTYPE_WARNING, "%s", logStr.c_str()); break;
        default           : g_mir2xLog->addLog(LOGTYPE_FATAL,   "%s", logStr.c_str()); break;
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
        if(auto surfPtr = TTF_RenderText_Blended(g_sdlDevice->defaultTTF(m_fontSize), log.c_str(), 0, color)){
            texPtr = g_sdlDevice->createTextureFromSurface(surfPtr);
            SDL_DestroySurface(surfPtr);
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
