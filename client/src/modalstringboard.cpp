#include <cstdint>
#include <chrono>
#include "widget.hpp"
#include "imageboard.hpp"
#include "layoutboard.hpp"
#include "gfxcropboard.hpp"
#include "gfxresizeboard.hpp"
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
        ImageBoard     m_image;
        GfxCropBoard   m_imageUp;    // top 180 pixels
        GfxResizeBoard m_imageUpDup; // duplicate top 84 ~ 180 pixels for stretch
        GfxCropBoard   m_imageDown;  // bottom 40 pixels

    public:
        ModalStringBoardImpl()
            : Widget
              ({
                  .dir = DIR_NONE,
                  .x   = [](const Widget *){ return g_sdlDevice->getRendererWidth () / 2; },
                  .y   = [](const Widget *){ return g_sdlDevice->getRendererHeight() / 2; },
              })

            , m_board
              {{
                  .lineWidth = 300,
                  .font
                  {
                      .id = 1,
                      .size = 12,
                  },

                  .lineAlign = LALIGN_JUSTIFY, // LALIGN_CENTER,
              }}

            , m_image
              {{
                  .texLoadFunc = [this](const Widget *){ return g_progUseDB->retrieve(m_texID); },
              }}

            , m_imageUp
              {{
                  .getter = &m_image,
                  .vr
                  {
                      {},
                      [this]{ return m_image.w(); },
                      180,
                  },
              }}

            , m_imageUpDup
              {{
                  .getter = &m_imageUp,
                  .vr
                  {
                      0,
                      84,
                      m_image.w(),
                      180 - 84,
                  },

                  .resize
                  {
                      [this]{ return m_image.w(); },
                      [this]{ return std::max<int>(m_minH, 84 + m_board.h() + 30 * 2); }, // add 30 pixels as top/bottom margin of m_board
                  },

                  .parent{this},
              }}

            , m_imageDown
              {{
                  .x = [this]{ return m_imageUpDup.dx(); },
                  .y = [this]{ return m_imageUpDup.dy() + m_imageUpDup.h(); },

                  .getter = &m_image,
                  .vr
                  {
                      0,
                      [this]{ return m_image.h() - 40; },
                      [this]{ return m_image.w(); },
                      40,
                  },

                  .parent{this},
              }}
        {
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
        void drawDefault(Widget::ROIMap m) const override
        {
            drawChild(&m_imageUpDup, m);
            drawChild(&m_imageDown , m);
            drawChild(&m_board     , m);
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
    m_boardImpl->drawRoot({});
    g_sdlDevice->present();
}
