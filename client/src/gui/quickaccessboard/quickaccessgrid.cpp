#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "quickaccessgrid.hpp"

extern PNGTexDB *g_itemDB;
extern SDLDevice *g_sdlDevice;

QuickAccessGrid::QuickAccessGrid(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        int argSlot,
        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::move(argW),
          .h = std::move(argH),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , slot(argSlot)
    , proc(argProc)

    , bg
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](int drawDstX, int drawDstY)
          {
              if(Widget::ROIMap{.x{drawDstX}, .y{drawDstY}, .ro{roi()}}.in(SDLDeviceHelper::getMousePLoc())){
                   g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
               }
          },

          .parent{this},
      }}

    , item
      {{
          .dir = DIR_NONE,

          .x = [this]{ return w() / 2; },
          .y = [this]{ return h() / 2; },

          .texLoadFunc = [this] -> SDL_Texture *
          {
              if(const auto &item = proc->getMyHero()->getBelt(slot)){
                  return g_itemDB->retrieve(DBCOM_ITEMRECORD(item.itemID).pkgGfxID | 0X01000000);
              }
              return nullptr;
          },

          .parent{this},
      }}

    , count
      {{
          .dir = DIR_UPRIGHT,
          .x = [this]{ return w() - 1; },
          .y = 0,

          .textFunc = [this] -> std::string
          {
              if(const auto &item = proc->getMyHero()->getBelt(slot); item && (item.count > 1)){
                  return std::to_string(item.count);
              }
              return {};
          },

          .font
          {
              .id = 1,
              .size = 10,
          },

          .parent{this},
      }}
{}
