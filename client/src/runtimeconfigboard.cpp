#include "luaf.hpp"
#include "client.hpp"
#include "imeboard.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"

extern Client *g_client;
extern IMEBoard *g_imeBoard;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

bool RuntimeConfigBoard::SwitchIntegerButton::setValue(int value, bool triggerSwitchCallback)
{
    fflassert(value >= 0, value);
    fflassert(value < getValueCount(), value, getValueCount());

    if(value == getValue()){
        return false;
    }

    const auto oldValue = getValue();
    m_valueState.first = value;

    if(triggerSwitchCallback){
        if(m_onSwitch){
            m_onSwitch(oldValue, getValue());
        }

        for(auto parentPtr = parent(); parentPtr; parentPtr = parentPtr->parent()){
            if(auto p = dynamic_cast<RuntimeConfigBoard *>(parentPtr)){
                p->reportRuntimeConfig();
                break;
            }
        }
    }

    return true;
}

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

    , m_musicSwitch
      {
          DIR_UPLEFT,
          431,
          85,

          to_d(SDRuntimeConfig().bgm),
          2,

          [this](int, int state)
          {
              m_sdRuntimeConfig.bgm = state;
              g_sdlDevice->setBGMVolume(getMusicVolume().value_or(0.0f));
          },
          this
      }

    , m_soundEffectSwitch
      {
          DIR_UPLEFT,
          431,
          145,

          to_d(SDRuntimeConfig().soundEff),
          2,

          [this](int, int state)
          {
              m_sdRuntimeConfig.soundEff = state;
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

    , m_entryProtoList
      {
          {{u8"游戏设置选项"}, 0, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},
          {{u8"游戏设置选项"}, 1, [](int, int){}},

          {{u8"和平攻击", u8"组队攻击", u8"行会攻击", u8"全体攻击"}, to_d(SDRuntimeConfig().attackMode - ATKMODE_BEGIN), [this](int, int mode)
          {
              m_sdRuntimeConfig.attackMode = ATKMODE_BEGIN + mode;
          }},

          {{u8"拼音输入法"}, to_d(SDRuntimeConfig().ime), [this](int, int state)
          {
              g_imeBoard->setActive(state);
              m_sdRuntimeConfig.ime = state;
          }},
      }

    , m_processRun([proc]()
      {
          fflassert(proc);
          return proc;
      }())
{
    // 1.0f -> SDL_MIX_MAXVOLUME
    // SDL_mixer initial sound/music volume is SDL_MIX_MAXVOLUME

    m_musicSlider.setValue(to_f(SDRuntimeConfig().bgmValue) / 100.0, false);
    m_soundEffectSlider.setValue(to_f(SDRuntimeConfig().soundEffValue) / 100.0, false);

    setShow(false);
    auto texPtr = g_progUseDB->retrieve(0X0000001B);

    fflassert(texPtr);
    std::tie(m_w, m_h) = SDLDeviceHelper::getTextureSize(texPtr);

    m_switchList.reserve(m_entryProtoList.size());
    for(const auto &[titleList, initValue, onSwitch]: m_entryProtoList){
        // can not skip invalid entry
        // m_entryProtoList and m_switch shares indices
        fflassert(!titleList.empty());
        const auto [infoX, infoY, buttonX, buttonY] = getEntryPLoc(m_switchList.size());

        if(titleList.size() == 1){
            m_switchList.push_back(new OnOffButton
            {
                DIR_UPLEFT,
                buttonX,
                buttonY,
                initValue,
                2,
                onSwitch,
                this,
                true,
            });
        }
        else{
            m_switchList.push_back(new SwitchNextButton
            {
                DIR_UPLEFT,
                buttonX,
                buttonY,
                initValue,
                to_d(titleList.size()),
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

    for(size_t i = 0; i < m_switchList.size(); ++i){
        const auto buttonPtr = m_switchList.at(i);
        const auto &titleList = std::get<0>(m_entryProtoList.at(i));
        const auto &[infoX, infoY, buttonX, buttonY] = getEntryPLoc(i);

        if(titleList.size() == 1){
            drawEntryTitle(titleList.front().c_str(), infoX, infoY);
        }
        else{
            drawEntryTitle(titleList.at(buttonPtr->getValue()).c_str(), infoX, infoY);
        }
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

    for(const auto buttonPtr: m_switchList){
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

void RuntimeConfigBoard::setConfig(SDRuntimeConfig config)
{
    m_sdRuntimeConfig = std::move(config);

    m_musicSwitch.setValue(m_sdRuntimeConfig.bgm, false);
    m_musicSlider.setValue(m_sdRuntimeConfig.bgmValue / 100.0, false);

    m_soundEffectSwitch.setValue(m_sdRuntimeConfig.soundEff, false);
    m_soundEffectSlider.setValue(m_sdRuntimeConfig.soundEffValue / 100.0, false);
}

void RuntimeConfigBoard::reportRuntimeConfig()
{
    CMSetRuntimeConfig cmSRC;
    std::memset(&cmSRC, 0, sizeof(cmSRC));

    cmSRC.bgm = getConfig().bgm;
    cmSRC.bgmValue = getConfig().bgmValue;

    cmSRC.soundEff = getConfig().soundEff;
    cmSRC.soundEffValue = getConfig().soundEffValue;

    cmSRC.ime = getConfig().ime;
    cmSRC.attackMode = getConfig().attackMode;

    g_client->send(CM_SETRUNTIMECONFIG, cmSRC);
}
