#include "luaf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

RuntimeConfigBoard::RuntimeConfigBoard(int argX, int argY, ProcessRun *proc, Widget *widgetPtr, bool autoDelete)
    : Widget(DIR_UPLEFT, argX, argY, 0, 0, widgetPtr, autoDelete)
    , m_closeButton
      {
          DIR_UPLEFT,
          449,
          415,
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
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }

    , m_attackModeSwitch
      {
          DIR_UPLEFT,
          431,
          55,
          {
              0X0000130,
              0X0000131,
              0X0000130,
          },

          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          false,
          false,
          this,
      }

    , m_musicSwitch
      {
          431,
          85,
          true,
          [this](bool)
          {
              g_sdlDevice->setBGMVolume(getMusicVolume().value_or(0.0f));
          },
          this
      }

    , m_soundEffectSwitch
      {
          431,
          145,
          true,
          [this](bool)
          {
              g_sdlDevice->setSoundEffectVolume(getSoundEffectVolume().value_or(0.0f));
          },
          this
      }

    , m_musicSlider
      {
          DIR_UPLEFT,
          280,
          124,
          194,
          6,

          true,
          1,
          [this](float)
          {
              g_sdlDevice->setBGMVolume(getMusicVolume().value_or(0.0f));
          },
          this,
      }

    , m_soundEffectSlider
      {
          DIR_UPLEFT,
          280,
          184,
          194,
          6,

          true,
          1,
          [this](float)
          {
              g_sdlDevice->setSoundEffectVolume(getSoundEffectVolume().value_or(0.0f));
          },
          this,
      }

    , m_processRun([proc]()
      {
          fflassert(proc);
          return proc;
      }())
{
    // 1.0f -> SDL_MIX_MAXVOLUME
    // SDL_mixer initial sound/music volume is SDL_MIX_MAXVOLUME

    m_musicSlider.setValue(1.0f, false);
    m_soundEffectSlider.setValue(1.0f, false);

    setShow(false);
    auto texPtr = g_progUseDB->retrieve(0X0000001B);

    fflassert(texPtr);
    std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);

    const std::array<std::tuple<const char8_t *, bool, std::function<void(bool)>>, 19> entryList // there is only 19 free slots
    {{
        {u8"游戏设置选项", false, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
        {u8"游戏设置选项",  true, [](bool){},},
    }};

    for(const auto &[info, initValue, onSwitch]: entryList){
        if(info){
            const auto [infoX, infoY, buttonX, buttonY] = getEntryPLoc(m_switchList.size());
            m_switchList.emplace_back(info, new OnOffTexButton
            {
                buttonX,
                buttonY,
                initValue,
                onSwitch,
                this,
                true,
            });
        }
    }
}

void RuntimeConfigBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000100), dstX, dstY);
    drawEntryTitle(u8"【游戏设置】", 255, 35);
    m_closeButton.draw();

    drawEntryTitle(u8"攻击模式", 345, 67);
    m_attackModeSwitch.draw();

    drawEntryTitle(u8"背景音乐", 345,  97);
    drawEntryTitle(u8"音效",     345, 157);

    m_musicSwitch.draw();
    m_soundEffectSwitch.draw();

    if(m_musicSwitch.getValue()){
        m_musicSlider.draw();
    }

    if(m_soundEffectSwitch.getValue()){
        m_soundEffectSlider.draw();
    }

    for(int i = 0; const auto &[infoText, buttonPtr]: m_switchList){
        const auto [infoX, infoY, buttonX, buttonY] = getEntryPLoc(i++);
        drawEntryTitle(infoText, infoX, infoY);
        buttonPtr->draw();
    }
}

bool RuntimeConfigBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    for(auto widgetPtr:
    {
        static_cast<Widget *>(&m_closeButton),
        static_cast<Widget *>(&m_attackModeSwitch),
        static_cast<Widget *>(&m_musicSwitch),
        static_cast<Widget *>(&m_soundEffectSwitch),
    }){
        if(widgetPtr->processEvent(event, valid)){
            return consumeFocus(true);
        }
    }

    if(m_musicSwitch.getValue() && m_musicSlider.processEvent(event, valid)){
        return consumeFocus(true);
    }

    if(m_soundEffectSwitch.getValue() && m_soundEffectSlider.processEvent(event, valid)){
        return consumeFocus(true);
    }

    for(const auto &[infoText, buttonPtr]: m_switchList){
        if(buttonPtr->processEvent(event, valid)){
            return consumeFocus(true);
        }
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    setShow(false);
                    return consumeFocus(false);
                }
                return consumeFocus(true);
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));
                    moveBy(newX - x(), newY - y());
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(in(event.button.x, event.button.y));
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void RuntimeConfigBoard::drawEntryTitle(const char8_t *info, int dstCenterX, int dstCenterY) const
{
    const LabelBoard titleBoard
    {
        DIR_NONE,
        0, // reset by new width
        0,
        to_u8cstr(info),

        1,
        12,
        0,

        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
    };
    titleBoard.drawAt(DIR_NONE, x() + dstCenterX, y() + dstCenterY);
}

std::tuple<int, int, int, int> RuntimeConfigBoard::getEntryPLoc(size_t entry)
{
    if(entry < 12){
        return {100, 67 + entry * 30, 186, 55 + entry * 30};
    }
    else if(entry >= 12 && entry < 19){
        return {345, 67 + (entry - 7) * 30, 431, 55 + (entry - 7) * 30};
    }
    else{
        throw fflvalue(entry);
    }
}
