#include <cmath>
#include <utf8.h>
#include "mathf.hpp"
#include "colorf.hpp"
#include "imeboard.hpp"
#include "inputline.hpp"
#include "sdldevice.hpp"
#include "labelboard.hpp"
#include "clientargparser.hpp"

extern IMEBoard *g_imeBoard;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

InputLine::InputLine(InputLine::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::move(args.w),
          .h = std::move(args.h),

          .parent = std::move(args.parent),
      }}

    , m_imeEnabled(std::move(args.enableIME))
    , m_tpset
      {
          0,
          LALIGN_LEFT,
          false,

          args.font.id,
          args.font.size,
          args.font.style,

          std::move(args.font.color),
          std::move(args.font.bgColor),
      }

    , m_cursorArgs(std::move(args.cursor))

    , m_onTab   (std::move(args.onTab))
    , m_onCR    (std::move(args.onCR))
    , m_onChange(std::move(args.onChange))
    , m_validate(std::move(args.validate))
{}

bool InputLine::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                // another widget can consume the event
                // and pass the focus to this widget, don't drop focus for keyboard events

                if(!valid){
                    return false;
                }

                if(!focus()){
                    return false;
                }

                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(m_onTab){
                                m_onTab();
                            }
                            return true;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_onCR){
                                m_onCR();
                            }
                            return true;
                        }
                    case SDLK_LEFT:
                        {
                            m_cursor = std::max<int>(0, m_cursor - 1);
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_RIGHT:
                        {
                            if(m_tpset.empty()){
                                m_cursor = 0;
                            }
                            else{
                                m_cursor = std::min<int>(m_tpset.lineTokenCount(0), m_cursor + 1);
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            if(m_cursor > 0){
                                m_tpset.deleteToken(m_cursor - 1, 0, 1);
                                m_cursor--;

                                if(m_onChange){
                                    m_onChange(m_tpset.getRawString());
                                }
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_ESCAPE:
                        {
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            const char keyChar = SDLDeviceHelper::getKeyChar(event, true);
                            if(!g_clientArgParser->disableIME && Widget::evalBool(m_imeEnabled, this) && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
                                g_imeBoard->gainFocus("", str_printf("%c", keyChar), this, [this](std::string s)
                                {
                                    m_tpset.insertUTF8String(m_cursor, 0, s.c_str());
                                    m_cursor += utf8::distance(s.begin(), s.end());
                                    if(m_onChange){
                                        m_onChange(m_tpset.getRawString());
                                    }
                                });
                            }
                            else if(keyChar != '\0'){
                                m_tpset.insertUTF8String(m_cursor++, 0, str_printf("%c", keyChar).c_str());
                                if(m_onChange){
                                    m_onChange(m_tpset.getRawString());
                                }
                            }

                            m_cursorBlink = 0.0;
                            return true;
                        }
                }
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!valid){
                    return consumeFocus(false);
                }

                if(!m.in(event.button.x, event.button.y)){
                    return consumeFocus(false);
                }

                if(event.type == SDL_MOUSEBUTTONDOWN){
                    const int eventX = event.button.x - m.x;
                    const int eventY = event.button.y - m.y;

                    const auto [cursorX, cursorY] = m_tpset.locCursor(eventX, eventY);
                    if(cursorY != 0){
                        throw fflerror("cursor locates at wrong line");
                    }

                    m_cursor = cursorX;
                    m_cursorBlink = 0.0;
                }

                return consumeFocus(true);
            }
        default:
            {
                return false;
            }
    }
}

void InputLine::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    int dstCropX = m.x;
    int dstCropY = m.y;
    int srcCropX = m.ro->x;
    int srcCropY = m.ro->y;
    int srcCropW = m.ro->w;
    int srcCropH = m.ro->h;

    const int tpsetX = 0;
    const int tpsetY = 0 + (h() - (m_tpset.empty() ? m_tpset.getDefaultFontHeight() : m_tpset.ph())) / 2;

    const auto needDraw = mathf::cropROI(
            &srcCropX, &srcCropY,
            &srcCropW, &srcCropH,
            &dstCropX, &dstCropY,

            w(),
            h(),

            tpsetX, tpsetY, m_tpset.pw(), m_tpset.ph());

    if(needDraw){
        m_tpset.draw({.x=dstCropX, .y=dstCropY, .ro{srcCropX - tpsetX, srcCropY - tpsetY, srcCropW, srcCropH}});
    }

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    if(!focus()){
        return;
    }

    int cursorY = m.y + tpsetY;
    int cursorX = m.x + tpsetX + [this]()
    {
        if(m_tpset.empty() || m_cursor == 0){
            return 0;
        }

        if(m_cursor == m_tpset.lineTokenCount(0)){
            return m_tpset.pw();
        }

        const auto pToken = m_tpset.getToken(m_cursor - 1, 0);
        return pToken->box.state.w1 + pToken->box.state.x + pToken->box.info.w;
    }();

    int cursorW = Widget::evalSizeOpt(m_cursorArgs.w, this, []{ return 2; });
    int cursorH = std::max<int>(m_tpset.ph(), h());

    if(mathf::rectangleOverlapRegion(m.x, m.y, m.ro->w, m.ro->h, cursorX, cursorY, cursorW, cursorH)){
        g_sdlDevice->fillRectangle(Widget::evalU32(m_cursorArgs.color, this), cursorX, cursorY, cursorW, cursorH);
    }

    if(g_clientArgParser->debugDrawInputLine){
        g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), m.x, m.y, w(), h());
    }
}

void InputLine::deleteChar()
{
    m_tpset.deleteToken(m_cursor - 1, 0, 1);
    m_cursor--;
}

void InputLine::insertChar(char ch)
{
    const char rawString[]
    {
        ch, '\0',
    };

    m_tpset.insertUTF8String(m_cursor, 0, rawString);
    m_cursor++;
}

void InputLine::insertUTF8String(const char *utf8Str)
{
    if(str_haschar(utf8Str)){
        m_cursor += m_tpset.insertUTF8String(m_cursor, 0, utf8Str);
    }
}

void InputLine::clear()
{
    m_cursor = 0;
    m_cursorBlink = 0.0;

    if(!m_tpset.empty()){
        m_tpset.clear();

        if(m_onChange){
            m_onChange({});
        }
    }
}

void InputLine::setInput(const char *utf8Str)
{
    m_cursor = 0;
    m_cursorBlink = 0.0;

    m_tpset.clear();
    if(str_haschar(utf8Str)){
        m_cursor = m_tpset.insertUTF8String(m_cursor, 0, utf8Str);
    }

    if(m_onChange){
        m_onChange(m_tpset.getRawString());
    }
}
