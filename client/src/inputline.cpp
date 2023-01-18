#include <cmath>
#include "mathf.hpp"
#include "colorf.hpp"
#include "ime.hpp"
#include "inputline.hpp"
#include "sdldevice.hpp"
#include "labelboard.hpp"
#include "clientargparser.hpp"

extern IME *g_ime;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

bool InputLine::processEvent(const SDL_Event &event, bool valid)
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
                            if(!m_imeEnabled || g_ime->empty()){
                                if(m_onTab){
                                    m_onTab();
                                }
                            }
                            return true;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_imeEnabled){
                                if(const auto result = g_ime->result(); !result.empty()){
                                    m_tpset.insertUTF8String(m_cursor++, 0, result.c_str());
                                    g_ime->clear();
                                }
                                else if(m_onCR){
                                    m_onCR();
                                }
                            }
                            else if(m_onCR){
                                m_onCR();
                            }
                            return true;
                        }
                    case SDLK_LEFT:
                        {
                            if(!m_imeEnabled || g_ime->empty()){
                                m_cursor = std::max<int>(0, m_cursor - 1);
                                m_cursorBlink = 0.0;
                            }
                            return true;
                        }
                    case SDLK_RIGHT:
                        {
                            if(!m_imeEnabled || g_ime->empty()){
                                if(m_tpset.empty()){
                                    m_cursor = 0;
                                }
                                else{
                                    m_cursor = std::min<int>(m_tpset.lineTokenCount(0), m_cursor + 1);
                                }
                                m_cursorBlink = 0.0;
                            }
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            if(m_imeEnabled && !g_ime->empty()){
                                g_ime->backspace();
                            }
                            else{
                                if(m_cursor > 0){
                                    m_tpset.deleteToken(m_cursor - 1, 0, 1);
                                    m_cursor--;
                                }
                                m_cursorBlink = 0.0;
                            }
                            return true;
                        }
                    case SDLK_ESCAPE:
                        {
                            if(m_imeEnabled && !g_ime->empty()){
                                g_ime->clear();
                            }
                            else{
                                setFocus(false);
                            }
                            return true;
                        }
                    default:
                        {
                            const char keyChar = SDLDeviceHelper::getKeyChar(event, true);
                            if(m_imeEnabled){
                                if(keyChar >= 'a' && keyChar <= 'z'){
                                    g_ime->feed(keyChar);
                                }
                            }
                            else{
                                if(keyChar != '\0'){
                                    const char text[] {keyChar, '\0'};
                                    m_tpset.insertUTF8String(m_cursor++, 0, text);
                                }
                            }

                            m_cursorBlink = 0.0;
                            return true;
                        }
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!in(event.button.x, event.button.y)){
                    return consumeFocus(false);
                }

                const int eventX = event.button.x - x();
                const int eventY = event.button.y - y();

                const auto [cursorX, cursorY] = m_tpset.locCursor(eventX, eventY);
                if(cursorY != 0){
                    throw fflerror("cursor locates at wrong line");
                }

                m_cursor = cursorX;
                m_cursorBlink = 0.0;

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

    const auto needDraw = mathf::ROICrop(
            &srcCropX, &srcCropY,
            &srcCropW, &srcCropH,
            &dstCropX, &dstCropY,

            w(),
            h(),

            tpsetX, tpsetY, m_tpset.pw(), m_tpset.ph());

    if(needDraw){
        m_tpset.drawEx(dstCropX, dstCropY, srcCropX - tpsetX, srcCropY - tpsetY, srcCropW, srcCropH);
    }

    if(m_imeEnabled && !g_ime->empty()){
        int offsetY = 50;
        LabelBoard inputString(DIR_UPLEFT, 0, 0, to_u8cstr(g_ime->result()), 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF));

        const int inputBoardW = inputString.w();
        const int inputBoardH = inputString.h();

        inputString.drawEx(0, offsetY, 0, 0, inputBoardW, inputBoardH);
        offsetY += inputBoardH;

        if(const auto candidateList = g_ime->candidateList(); !candidateList.empty()){
            for(int index = 1; const auto &candidate: candidateList){
                LabelBoard candidateString(DIR_UPLEFT, 0, 0, str_printf(u8"%d. %s", index++, candidate.c_str()).c_str(), 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF));

                const int candidateBoardW = candidateString.w();
                const int candidateBoardH = candidateString.h();

                candidateString.drawEx(0, offsetY, 0, 0, candidateBoardW, candidateBoardH);
                offsetY += candidateBoardH;

                if(index >= 10){
                    break;
                }
            }
        }
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
