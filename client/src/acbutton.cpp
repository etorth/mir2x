#include "widget.hpp"
#include "acbutton.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ACButton::ACButton(ACButton::InitArgs args)
    : ButtonBase
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),

          .onTrigger = [this](Widget *, int)
          {
              m_currButtonName = (m_currButtonName + 1) % m_buttonNameList.size();
              setLabel();
          },

          .onClickDone = false,
          .parent = args.parent,
      }}

    , m_proc(fflcheck(args.proc))
    , m_texMap
      {
          {"AC", 0X00000046},
          {"DC", 0X00000047},
          {"MA", 0X00000048},
          {"MC", 0X00000049},
      }
    , m_currButtonName(0)
    , m_buttonNameList(std::move(fflcheck(args.names, !args.names.empty())))
    , m_labelBoard
      {{
          .label = u8"0-0",
          .font
          {
              .id = 1,
              .size = 12,
              .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
          },
          .parent{this},
      }}
{
    bool inited = false;
    for(auto &p: m_texMap){
        if(auto texPtr = g_progUseDB->retrieve(p.second)){
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
            setW(std::max<int>(w(), texW));
            setH(std::max<int>(h(), texH));
            inited = true;
        }
    }

    if(!inited){
        throw fflerror("missing texture for ACButton");
    }

    setLabel();
}

void ACButton::drawDefault(Widget::ROIMap m) const
{
    const auto buttonName = m_buttonNameList.at(m_currButtonName);
    auto texPtr = g_progUseDB->retrieve(m_texMap.at(buttonName));

    if(!texPtr){
        throw fflerror("no texture for %s", buttonName.c_str());
    }

    SDL_SetTextureAlphaMod(texPtr, 255);
    SDL_SetTextureColorMod(texPtr, 255, 255, 255);

    g_sdlDevice->drawTexture(texPtr, m.x, m.y);
    m_labelBoard.draw({.x = m.x + w() + 5, .y = m.y, .ro = m.ro});

    switch(getState()){
        case BEVENT_ON:
        case BEVENT_DOWN:
            {
                SDL_SetTextureColorMod(texPtr, 255, 0, 0);
                SDL_SetTextureAlphaMod(texPtr, 128);
                g_sdlDevice->drawTexture(texPtr, m.x, m.y);
                break;
            }
        default:
            {
                break;
            }
    }
}

void ACButton::setLabel()
{
    const auto buttonName = m_buttonNameList.at(m_currButtonName);
    const auto [low, high] = m_proc->getACNum(buttonName);

    if(low > high){
        throw fflerror("invalid %s: %d-%d", to_cstr(buttonName), low, high);
    }
    m_labelBoard.setText(u8"%d-%d", low, high);
}
