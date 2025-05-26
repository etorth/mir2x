#include <cstdint>
#include <chrono>
#include "widget.hpp"
#include "imageboard.hpp"
#include "layoutboard.hpp"
#include "gfxcropboard.hpp"
#include "gfxcropdupboard.hpp"
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
// ||       text area        ||    |   +--- 40: take some bg of input area and splice with text area
// ||                        ||        |
// |+------------------------+|      - v
// |+------------------------+|      ^ -
// ||       INPUT AREA       ||      |
// |+------------------------+|
// +--------------------------+        -
//                                     ^
//                                     |

class ModalStringBoardImpl: public Widget
{
    private:
        friend class ModalStringBoard;

    private:
        const int m_minH = 220;
        const uint32_t m_texID = 0X07000000;

    private:
        LayoutBoard m_board;

    private:
        ImageBoard      m_image;
        GfxCropBoard    m_imageUp;    // top 180 pixels
        GfxCropDupBoard m_imageUpDup; // duplicate top 84 ~ 180 pixels for stretch
        GfxCropBoard    m_imageDown;  // bottom 40 pixels

    public:
        ModalStringBoardImpl()
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

            addChildAt(&m_board, DIR_NONE, [this](const Widget *)
            {
                return m_imageUpDup.w() / 2;
            },

            [this](const Widget *)
            {
                return 84 + (m_imageUpDup.h() - 84) / 2;
            },

            false);
        }

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            drawChildEx(&m_imageUpDup, dstX, dstY, srcX, srcY, srcW, srcH);
            drawChildEx(&m_imageDown , dstX, dstY, srcX, srcY, srcW, srcH);
            drawChildEx(&m_board     , dstX, dstY, srcX, srcY, srcW, srcH);
        }
};

ModalStringBoard::ModalStringBoard()
    : m_boardImpl(std::make_unique<ModalStringBoardImpl>())
{}

void ModalStringBoard::loadXML(std::u8string s)
{
    if(s == m_xmlString){
        return;
    }

    m_xmlString = std::move(s);
    dynamic_cast<ModalStringBoardImpl *>(m_boardImpl.get())->m_board.loadXML(to_cstr(m_xmlString));
}

void ModalStringBoard::drawScreen(bool drainEvents) const
{
    if(drainEvents){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            continue;
        }
    }

    g_sdlDevice->clearScreen();
    m_boardImpl->draw();
    g_sdlDevice->present();
}
