#include <cinttypes>
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "cbface.hpp"
#include "processrun.hpp"
#include "clientmonster.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBFace::CBFace(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          std::move(argW),
          std::move(argH),

          {},

          argParent,
          argAutoDelete,
      }

    , m_processRun(argProc)
    , m_face
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },
          [this](const Widget *)
          {
              if(auto texPtr = g_progUseDB->retrieve(getFaceTexID())){
                  return texPtr;
              }
              return g_progUseDB->retrieve(0X010007CF);
          },

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(255),
          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , m_hpBar
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *)
          {
              return to_dround(getHPRatio() * w());
          },

          CBFace::BAR_HEIGHT,

          [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000015);
          },

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(255),
          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , m_drawBuffIDList
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *self, int drawDstX, int drawDstY)
          {
              drawBuffIDList(drawDstX, drawDstY, self->w(), self->h());
          }
      }
{}

double CBFace::getHPRatio() const
{
    if(const auto coPtr = m_processRun->findUID(m_processRun->getFocusUID(FOCUS_MOUSE))){
        switch(coPtr->type()){
            case UID_PLY:
            case UID_MON:
                {
                    return coPtr->getHealthRatio().at(0);
                }
            default:
                {
                    break;
                }
        }
    }
    return m_processRun->getMyHero()->getHealthRatio().at(0);
}

uint32_t CBFace::getFaceTexID() const
{
    if(const auto coPtr = m_processRun->findUID(m_processRun->getFocusUID(FOCUS_MOUSE))){
        switch(coPtr->type()){
            case UID_PLY:
                {
                    return dynamic_cast<Hero *>(coPtr)->faceGfxID();
                }
            case UID_MON:
                {
                    if(const auto lookID = dynamic_cast<ClientMonster *>(coPtr)->lookID(); lookID >= 0){
                        return UINT32_C(0X01000000) + (lookID - LID_BEGIN);
                    }
                    return SYS_U32NIL;
                }
            default:
                {
                    break;
                }
        }
    }
    return m_processRun->getMyHero()->faceGfxID();
}

const std::optional<SDBuffIDList> &CBFace::getSDBuffIDListOpt() const
{
    if(const auto coPtr = m_processRun->findUID(m_processRun->getFocusUID(FOCUS_MOUSE))){
        switch(coPtr->type()){
            case UID_PLY:
            case UID_MON:
                {
                    return coPtr->getSDBuffIDListOpt();
                }
            default:
                {
                    break;
                }
        }
    }
    return m_processRun->getMyHero()->getSDBuffIDListOpt();
}

void CBFace::drawBuffIDList(int drawDstX, int drawDstY, int, int) const
{
    const auto sdBuffIDListOpt = getSDBuffIDListOpt();
    if(!sdBuffIDListOpt.has_value()){
        return;
    }

    const auto &sdBuffIDList = sdBuffIDListOpt.value();

    constexpr int buffIconDrawW = 16;
    constexpr int buffIconDrawH = 16;

    // +--16--+
    // |      |
    // |      16
    // |      |
    // *------+
    // ^
    // |
    // +--- (buffIconOffStartX, buffIconOffStartY)

    const int buffIconOffStartX = drawDstX + 20;
    const int buffIconOffStartY = drawDstY + 79;

    for(int drawIconCount = 0; const auto id: sdBuffIDList.idList){
        const auto &br = DBCOM_BUFFRECORD(id);
        fflassert(br);

        if(br.icon.gfxID != SYS_U32NIL){
            if(auto iconTexPtr = g_progUseDB->retrieve(br.icon.gfxID)){
                const int buffIconOffX = buffIconOffStartX + (drawIconCount % 5) * buffIconDrawW;
                const int buffIconOffY = buffIconOffStartY - (drawIconCount / 5) * buffIconDrawH;

                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(iconTexPtr);
                g_sdlDevice->drawTexture(iconTexPtr, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, 0, 0, texW, texH);

                const auto baseColor = [&br]() -> uint32_t
                {
                    if(br.favor > 0){
                        return colorf::GREEN;
                    }
                    else if(br.favor == 0){
                        return colorf::YELLOW;
                    }
                    else{
                        return colorf::RED;
                    }
                }();

                const auto startColor = baseColor | colorf::A_SHF(255);
                const auto   endColor = baseColor | colorf::A_SHF( 64);

                const auto edgeGridCount = (buffIconDrawW + buffIconDrawH) * 2 - 4;
                const auto startLoc = std::lround(edgeGridCount * std::fmod(m_accuTime, 1500.0) / 1500.0);

                g_sdlDevice->drawBoxFading(startColor, endColor, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, startLoc, buffIconDrawW + buffIconDrawH);
                drawIconCount++;
            }
        }
    }
}
