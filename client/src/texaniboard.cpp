#include <cfloat>
#include <numeric>
#include "colorf.hpp"
#include "fflerror.hpp"
#include "progusedb.hpp"
#include "sdldevice.hpp"
#include "texaniboard.hpp"

extern ProgUseDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

TexAniBoard::TexAniBoard(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        uint32_t argAniID,
        size_t   argFrameCount,

        size_t argFps,

        bool argFadeInout,
        bool argLoop,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .w = [this](const Widget *)
          {
              if(const auto [frame, _] = getDrawFrame(); frame >= 0){
                  if(auto texPtr = g_progUseDB->retrieve(m_aniID, frame)){
                      return SDLDeviceHelper::getTextureWidth(texPtr);
                  }
              }
              return 0;
          },

          .h = [this](const Widget *)
          {
              if(const auto [frame, _] = getDrawFrame(); frame >= 0){
                  if(auto texPtr = g_progUseDB->retrieve(m_aniID, frame)){
                      return SDLDeviceHelper::getTextureHeight(texPtr);
                  }
              }
              return 0;
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_aniID(argAniID)
    , m_frameCount(argFrameCount)
    , m_fps(argFps)

    , m_fadeInout(argFadeInout)
    , m_loop(argLoop)

    , m_cropArea
      {{
          .w = [this](const Widget *){ return w(); },
          .h = [this](const Widget *){ return h(); },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              const auto [frame, alpha] = getDrawFrame();
              if(frame < 0){
                  return;
              }

              const int currFrame = (frame + 0) % m_frameCount;
              const int nextFrame = (frame + 1) % m_frameCount;

              if(auto currTexPtr = g_progUseDB->retrieve(m_aniID, currFrame)){
                  const SDLDeviceHelper::EnableTextureModColor enableModColor(currTexPtr, colorf::WHITE + colorf::A_SHF(255 - alpha));
                  g_sdlDevice->drawTexture(currTexPtr, drawDstX, drawDstY);
              }

              if(auto nextTexPtr = g_progUseDB->retrieve(m_aniID, nextFrame)){
                  const SDLDeviceHelper::EnableTextureModColor enableModColor(nextTexPtr, colorf::WHITE + colorf::A_SHF(alpha));
                  g_sdlDevice->drawTexture(nextTexPtr, drawDstX, drawDstY);
              }
          },

          .parent{this},
      }}
{}

std::tuple<int, uint8_t> TexAniBoard::getDrawFrame() const
{
    if(m_frameCount == 0){ return {-1, 0}; }
    if(m_fps        == 0){ return { 0, 0}; }

    const double decimalFrame = m_accuTime * m_fps / 1000.0;
    const int    integerFrame = to_dround(std::floor(decimalFrame));

    return
    {
        [integerFrame, this]() -> int // current frame
        {
            if(m_loop){
                return integerFrame % m_frameCount;
            }
            else{
                return std::min<int>(integerFrame, m_frameCount - 1);
            }
        }(),


        [integerFrame, decimalFrame, this]() -> uint8_t // current alpha
        {
            if(m_fadeInout){
                return to_dround(255 * mathf::bound<double>(decimalFrame - integerFrame, 0.0, 1.0));
            }
            return 0;
        }(),
    };
}
