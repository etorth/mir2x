/*
 * =====================================================================================
 *
 *       Filename: modalstringboard.hpp
 *        Created: 07/18/2021 23:06:52
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

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
//        +--+   +--+
//        |  |   |  |
// +------+  +---+  +------+ ---
// |                       |  ^
// |                       |  | 180
// |                       |  v
// +-----------------------+ ---
// |                       |
// +-----------------------+ ---
// |                       |  | 40
// +-----------------------+ ---

ModalStringBoard::ModalStringBoard()
    : Widget
      {
          DIR_UPLEFT,
          0,
          0,
          0,
          0,
      }

    , m_image
      {
          DIR_UPLEFT,
          0,
          0,
          0,
          0,

          [](const ImageBoard *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X07000000);
          }
      }
{
    if(auto texPtr = g_progUseDB->retrieve(0X07000000)){
        m_w = SDLDeviceHelper::getTextureWidth(texPtr);
    }
    else{
        m_w = 358;
    }
    m_h = 180 + 40;
}

void ModalStringBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int, int) const
{
    m_image.drawEx(dstX, dstY,       0, 0,                w(), 180);
    m_image.drawEx(dstX, dstY + 180, 0, m_image.h() - 40, w(),  40);

    std::u8string xmlCopy;
    {
        std::lock_guard<std::mutex> lockGuard(m_lock);
        xmlCopy = m_xmlString;
    }

    if(!xmlCopy.empty()){
        LayoutBoard board
        {
            DIR_NONE,
            0,
            0,
            300,

            false,
            {0, 0, 0, 0},

            false,

            1,
            12,
            0,
            colorf::WHITE + colorf::A_SHF(255),
            0,

            LALIGN_JUSTIFY, // LALIGN_CENTER,
        };

        board.loadXML(to_cstr(xmlCopy));
        board.drawAt(DIR_NONE, dstX - srcX + w() / 2, dstY - srcY + h() / 2 + 20);
    }
}

void ModalStringBoard::waitDone()
{
    using namespace std::chrono_literals;
    while(true){
        // ignore events but need to flush
        // seems it's required to commit event like window size change
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            continue;
        }

        g_sdlDevice->clearScreen();
        {
            const auto [winW, winH] = g_sdlDevice->getRendererSize();
            drawAt(DIR_NONE, winW / 2, winH / 2);
        }
        g_sdlDevice->present();

        // put wait at end
        // this makes sure every waitDone() can draw at least 1 frame
        {
            std::unique_lock<std::mutex> lockGuard(m_lock);
            if(m_cond.wait_for(lockGuard, 10ms, [this](){ return m_done; })){
                return;
            }
        }
    }
}
