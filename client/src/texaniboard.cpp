#include <cmath>
#include "colorf.hpp"
#include "mathf.hpp"
#include "progusedb.hpp"
#include "sdldevice.hpp"
#include "texaniboard.hpp"

extern ProgUseDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

TexAniBoard::TexAniBoard(TexAniBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = [this](const Widget *)
          {
              if(const auto [frame, _] = getDrawFrame(); frame >= 0){
                  if(auto texPtr = g_progUseDB->retrieve(m_texAniID, frame)){
                      return SDLDeviceHelper::getTextureWidth(texPtr);
                  }
              }
              return 0;
          },

          .h = [this](const Widget *)
          {
              if(const auto [frame, _] = getDrawFrame(); frame >= 0){
                  if(auto texPtr = g_progUseDB->retrieve(m_texAniID, frame)){
                      return SDLDeviceHelper::getTextureHeight(texPtr);
                  }
              }
              return 0;
          },

          .parent = std::move(args.parent),
      }}

    , m_texAniID(args.texAniID)
    , m_frameCount(args.frameCount)
    , m_fps(args.fps)

    , m_crossFade(args.crossFade)
    , m_loop(args.loop)

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

              if(auto currTexPtr = g_progUseDB->retrieve(m_texAniID, frame)){
                  const SDLDeviceHelper::EnableTextureModColor enableModColor(currTexPtr, colorf::WHITE + colorf::A_SHF(255 - alpha));
                  g_sdlDevice->drawTexture(currTexPtr, drawDstX, drawDstY);
              }

              if(alpha > 0){
                  if(auto nextTexPtr = g_progUseDB->retrieve(m_texAniID, (frame + 1) % frameCount())){
                      const SDLDeviceHelper::EnableTextureModColor enableModColor(nextTexPtr, colorf::WHITE + colorf::A_SHF(alpha));
                      g_sdlDevice->drawTexture(nextTexPtr, drawDstX, drawDstY);
                  }
              }
          },

          .parent{this},
      }}
{}

int TexAniBoard::frameCount() const
{
    return m_frameCount.has_value() ? to_d(m_frameCount.value()) : g_progUseDB->frameCount(m_texAniID);
}

double TexAniBoard::frameDuration(int frame) const
{
    if(m_fps.has_value()){
        return 1000.0 / m_fps.value();
    }
    return std::max<int>(1, g_progUseDB->frameDelay(m_texAniID, frame));
}

std::tuple<int, uint8_t> TexAniBoard::getDrawFrame() const
{
    const int count = frameCount();
    if(count <= 0){
        return {-1, 0};
    }

    if(m_fps.has_value() && m_fps.value() == 0){
        return {0, 0};
    }

    double period = 0.0;
    for(int i = 0; i < count; ++i){
        period += frameDuration(i);
    }

    if(!m_loop && m_accuTime >= period){
        return {count - 1, 0};
    }

    double time = std::fmod(m_accuTime, period);
    for(int i = 0; i < count; ++i){
        const double duration = frameDuration(i);
        if(time < duration){
            // no blending into frame 0 at the end of a non-looping animation
            if(!m_crossFade || (!m_loop && i == count - 1)){
                return {i, 0};
            }
            return {i, to_dround(255 * mathf::bound<double>(time / duration, 0.0, 1.0))};
        }
        time -= duration;
    }
    return {count - 1, 0};
}
