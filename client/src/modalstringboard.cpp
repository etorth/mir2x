#include <chrono>
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "layoutboard.hpp"
#include "modalstringboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

// cut off middle
// use rest as string background
//
//                                 +------- 84
//                                 | +----- 180
//                                 v v
//        +--+      +--+           - -
//        |  |      |  |
// +------+  +------+  +------+
// |+------------------------+|    -
// ||                        ||    ^
// ||                        ||    |
// ||                        ||
// |+------------------------+|      - +--- 40
// |+------------------------+|      ^ |
// ||       INPUT AREA       ||      | v
// |+------------------------+|        -
// +--------------------------+        -
//                                     ^
//                                     |
ModalStringBoard::ModalStringBoard()
    : Widget
      {
          DIR_NONE,
          [](const Widget *){ return g_sdlDevice->getRendererWidth () / 2; },
          [](const Widget *){ return g_sdlDevice->getRendererHeight() / 2; },
      }

    , m_board
      {
          DIR_UPLEFT, // ignore
          0,
          0,
          300,

          "<layout><par></par></layout>",
          0,

          {},

          false,
          false,
          false,
          false,

          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_JUSTIFY, // LALIGN_CENTER,
      }

    , m_image
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [this](const ImageBoard *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(m_texID);
          }
      }

    , m_imageUp
      {
          DIR_UPLEFT,
          0,
          0,

          &m_image,

          0,
          0,
          [this](const Widget *){ return m_image.w(); },
          180,
      }

    , m_imageUpDup
      {
          DIR_UPLEFT,
          0,
          0,
          [this](const Widget *){ return m_image.w(); },
          [this](const Widget *){ return std::max<int>(m_minH, 84 + m_board.h() + 30 * 2); }, // add 30 pixels as top/bottom margin of m_board

          &m_imageUp,

          0,
          84,
          [this](const Widget *){ return m_image.w(); },
          180 - 84,

          this,
          false,
      }

    , m_imageDown
      {
          DIR_UPLEFT,
          [this](const Widget *){ return m_imageUpDup.dx(); },
          [this](const Widget *){ return m_imageUpDup.dy() + m_imageUpDup.h(); },

          &m_image,

          0,
          [this](const Widget *){ return m_image.h() - 40; },
          [this](const Widget *){ return m_image.w(); },
          40,

          {},

          this,
          false,
      }
{
    if(auto texPtr = g_progUseDB->retrieve(m_texID); !texPtr){
        throw fflerror("invalid texID: %llu", to_llu(m_texID));
    }

    addChild(&m_board, DIR_NONE, [this](const Widget *)
    {
        return m_imageUpDup.w() / 2;
    },

    [this](const Widget *)
    {
        return (m_imageUpDup.h() - 84) / 2;
    },

    false);
}

void ModalStringBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    drawChildEx(&m_imageUpDup, dstX, dstY, srcX, srcY, srcW, srcH);
    drawChildEx(&m_imageDown , dstX, dstY, srcX, srcY, srcW, srcH);
    {
        const std::lock_guard<std::mutex> lockGuard(m_lock);
        drawChildEx(&m_board, dstX, dstY, srcX, srcY, srcW, srcH);
    }
}

void ModalStringBoard::loadXML(std::u8string s)
{
    {
        const std::lock_guard<std::mutex> lockGuard(m_lock);
        if(s == m_xmlString){
            return;
        }

        m_xmlString = std::move(s);
        m_board.loadXML(to_cstr(m_xmlString));
    }
    m_cond.notify_one();
}

void ModalStringBoard::setDone()
{
    {
        const std::lock_guard<std::mutex> lockGuard(m_lock);
        m_done = true;
    }
    m_cond.notify_one();
}

void ModalStringBoard::waitDone()
{
    while(true){
        // ignore events but need to flush
        // seems it's required to commit event like window size change
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            continue;
        }

        g_sdlDevice->clearScreen();
        draw();
        g_sdlDevice->present();

        // put wait at end
        // this makes sure every waitDone() can draw at least 1 frame
        {
            std::unique_lock<std::mutex> lockGuard(m_lock);
            if(m_cond.wait_for(lockGuard, std::chrono::milliseconds(10), [this](){ return m_done; })){
                return;
            }
        }
    }
}
