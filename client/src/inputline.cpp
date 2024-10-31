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

bool InputLine::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
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
                            if(m_imeEnabled && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
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
                if(!in(event.button.x, event.button.y)){
                    return consumeFocus(false);
                }

                if(event.type == SDL_MOUSEBUTTONDOWN){
                    const int eventX = event.button.x - x();
                    const int eventY = event.button.y - y();

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

void InputLine::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    int srcCropX = srcX;
    int srcCropY = srcY;
    int srcCropW = srcW;
    int srcCropH = srcH;
    int dstCropX = dstX;
    int dstCropY = dstY;

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
        m_tpset.drawEx(dstCropX, dstCropY, srcCropX - tpsetX, srcCropY - tpsetY, srcCropW, srcCropH);
    }

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    if(!focus()){
        return;
    }

    int cursorY = y() + tpsetY;
    int cursorX = x() + tpsetX + [this]()
    {
        if(m_tpset.empty() || m_cursor == 0){
            return 0;
        }

        if(m_cursor == m_tpset.lineTokenCount(0)){
            return m_tpset.pw();
        }

        const auto pToken = m_tpset.getToken(m_cursor - 1, 0);
        return pToken->Box.State.W1 + pToken->Box.State.X + pToken->Box.Info.W;
    }();

    int cursorW = m_cursorWidth;
    int cursorH = std::max<int>(m_tpset.ph(), h());

    if(mathf::rectangleOverlapRegion(dstX, dstY, srcW, srcH, cursorX, cursorY, cursorW, cursorH)){
        g_sdlDevice->fillRectangle(m_cursorColor, cursorX, cursorY, cursorW, cursorH);
    }

    if(g_clientArgParser->debugDrawInputLine){
        g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), x(), y(), w(), h());
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
