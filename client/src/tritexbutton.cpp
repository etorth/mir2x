#include "colorf.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "tritexbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

TritexButton::TritexButton(TritexButton::InitArgs args)
    : ButtonBase
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = 0,
          .h = 0,

          .onOverIn  = std::move(args.onOverIn),
          .onOverOut = std::move(args.onOverOut),

          .onClick   = std::move(args.onClick),
          .onTrigger = std::move(args.onTrigger),

          .seff = std::move(args.seff),

          .offXOnOver = args.offXOnOver,
          .offYOnOver = args.offYOnOver,

          .offXOnClick = args.offXOnClick,
          .offYOnClick = args.offYOnClick,

          .onClickDone = args.onClickDone,
          .radioMode   = args.radioMode,

          .attrs  = std::move(args.attrs),
          .parent = std::move(args.parent),
      }}

    , m_texIDFunc(std::move(args.texIDFunc))
    , m_texIDList(std::move(args.texIDList))

    , m_img
      {{
          .texLoadFunc = [this]
          {
              return evalGfxTexture();
          },

          .modColor = [modColor = std::move(args.modColor), this]
          {
              if(!active()){
                  return colorf::RGBA(128, 128, 128, 255);
              }

              if(modColor.has_value()){
                  return Widget::evalU32(modColor.value(), this);
              }

              switch(getState()){
                  case BEVENT_OFF : return colorf::RGBA(255, 255, 255, 255);
                  case BEVENT_ON  : return colorf::RGBA(255, 200, 255, 255);
                  case BEVENT_DOWN: return colorf::RGBA(200, 150, 150, 255);
                  default: std::unreachable();
              }
          },

          .blendMode = [this]
          {
              if(m_blinkTime.has_value()){
                  const auto offTime = std::get<0>(m_blinkTime.value());
                  const auto  onTime = std::get<1>(m_blinkTime.value());

                  if(offTime == 0){
                      return SDL_BLENDMODE_ADD;
                  }
                  else if(onTime == 0){
                      return SDL_BLENDMODE_BLEND;
                  }
                  else{
                      if(std::fmod(m_accuBlinkTime, offTime + onTime) < offTime){
                          return SDL_BLENDMODE_BLEND;
                      }
                      else{
                          return SDL_BLENDMODE_ADD;
                      }
                  }
              }
              else{
                  return SDL_BLENDMODE_BLEND;
              }
          },
      }}
{}

void TritexButton::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }
    drawAsChild(&m_img, DIR_UPLEFT, m_offset[getState()][0], m_offset[getState()][1], m);
}

SDL_Texture *TritexButton::evalGfxTexture(std::optional<int> stateOpt) const
{
    const auto state = stateOpt.value_or(getState());
    return std::visit(VarDispatcher
    {
        [state, this](const std::function<std::optional<uint32_t>(                   )> &f){ return (f ? f(           ) : m_texIDList[state]).transform([](auto id){ return g_progUseDB->retrieve(id); }).value_or(nullptr); },
        [state, this](const std::function<std::optional<uint32_t>(                int)> &f){ return (f ? f(      state) : m_texIDList[state]).transform([](auto id){ return g_progUseDB->retrieve(id); }).value_or(nullptr); },
        [state, this](const std::function<std::optional<uint32_t>(const Widget *, int)> &f){ return (f ? f(this, state) : m_texIDList[state]).transform([](auto id){ return g_progUseDB->retrieve(id); }).value_or(nullptr); },
        [state, this](const                                                        auto & ){ return (                     m_texIDList[state]).transform([](auto id){ return g_progUseDB->retrieve(id); }).value_or(nullptr); },
    },
    m_texIDFunc);
}

SDL_Texture *TritexButton::evalGfxTextureValid() const
{
    for(const auto s = getState() - BEVENT_BEGIN; int i: {0, 1, 2}){
        if(const auto gfxPtr = evalGfxTexture(BEVENT_BEGIN + ((s + i) % 3))){
            return gfxPtr;
        }
    }
    return nullptr;
}
