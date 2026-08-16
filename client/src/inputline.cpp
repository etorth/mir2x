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

    , m_tpsetAlign(std::move(args.align))
    , m_imeEnabled(std::move(args.enableIME))
    , m_tpset
      {
          0,
          LALIGN_LEFT,
          false,
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
        case SDL_EVENT_TEXT_INPUT:
            {
                // when system IME is activated
                // no matter SDL_StopTextInput() is called or not
                // IME still show the input candidates, SDL just doesn't dispatch SDL_EVENT_TEXT_INPUT anymore

                // so SDL_StopTextInput does nothing
                // just filter the SDL_EVENT_TEXT_INPUT event

                if(const auto ime = Widget::evalInt(m_imeEnabled, this); (ime != IME_SYSTEM) && valid){
                    throw fflpanic("received valid SDL_EVENT_TEXT_INPUT while system input is not enabled: IME {}", ime);
                }

                if(!valid || !focus()){
                    return false;
                }

                if(str_haschar(event.text.text)){
                    if(insertInput(event.text.text)){
                        notifyInputChange();
                    }
                }

                m_cursorBlink = 0.0;
                return true;
            }
        case SDL_EVENT_KEY_DOWN:
            {
                // another widget can consume the event
                // and pass the focus to this widget, don't drop focus for keyboard events

                if(!valid){
                    return false;
                }

                if(!focus()){
                    return false;
                }

                switch(event.key.key){
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
                            adjustCursorPLocX();
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
                            adjustCursorPLocX();
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            if(deleteInput()){
                                notifyInputChange();
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
                            const auto ime = Widget::evalInt(m_imeEnabled, this);
                            const char keyChar = SDLDeviceHelper::getKeyChar(event, true);

                            if(ime == IME_SYSTEM){
                                // when System IME is enabled
                                // SDL3 still dispatch SDL_EVENT_KEY_DOWN, need to ignore
                            }

                            else if((ime == IME_EMBEDED) && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
                                g_imeBoard->gainFocus("", str_printf("%c", keyChar), this, [this](std::string s)
                                {
                                    if(insertInput(s)){
                                        notifyInputChange();
                                    }
                                });
                            }
                            else if(keyChar != '\0'){
                                if(insertInput(std::string(1, keyChar))){
                                    notifyInputChange();
                                }
                            }

                            m_cursorBlink = 0.0;
                            return true;
                        }
                }
            }
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                if(!valid){
                    return consumeFocus(false);
                }

                if(!m.in(to_d(event.button.x), to_d(event.button.y))){
                    return consumeFocus(false);
                }

                if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                    const int eventX = to_d(event.button.x) - (m.x - m.ro->x) - tpx();
                    const int eventY = to_d(event.button.y) - (m.y - m.ro->y) - tpy();

                    const auto [cursorX, cursorY] = m_tpset.locCursor(eventX, eventY);
                    if(cursorY != 0){
                        throw fflpanic("cursor locates at wrong line");
                    }

                    m_cursor = cursorX;
                    adjustCursorPLocX();
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

bool InputLine::validateInput(const std::string &currentInput, const std::string &newInput) const
{
    return !m_validate || m_validate(currentInput, newInput);
}

bool InputLine::insertInput(const std::string &input)
{
    if(input.empty() || !validateInput(m_tpset.getRawString(), input)){
        return false;
    }

    m_cursor += m_tpset.insertUTF8String(m_cursor, 0, input.c_str());
    adjustCursorPLocX();
    return true;
}

bool InputLine::deleteInput()
{
    if(m_cursor <= 0){
        return false;
    }

    m_tpset.deleteToken(m_cursor - 1, 0, 1);
    m_cursor--;
    adjustCursorPLocX();
    return true;
}

void InputLine::notifyInputChange() const
{
    if(m_onChange){
        m_onChange(m_tpset.getRawString());
    }
}

void InputLine::adjustCursorPLocX()
{
    if(m_tpsetAlign.has_value()){
        return;
    }

    if((w() >= m_tpset.px() + m_tpset.pw()) || !Widget::evalBool(m_cursorArgs.lazy, this)){
        return;
    }

    const auto currWidth = w();
    const auto [cursorPLocX, _, cursorPLocW, _] = getCursorPLocInXMLTypeset();

    if(cursorPLocW > currWidth){
        m_tpsetXOpt = (currWidth - cursorPLocW) / 2 - cursorPLocX;
    }

    else{
        const auto tpsetXValue = tpsetXFromOpt();
        if(const auto cursorOffLeft = tpsetXValue + cursorPLocX; cursorOffLeft < 0){
            m_tpsetXOpt = tpsetXValue - cursorOffLeft;
        }
        else if(const auto cursorOffRight = tpsetXValue + cursorPLocX + cursorPLocW - currWidth; cursorOffRight > 0){
            m_tpsetXOpt = tpsetXValue - cursorOffRight;
        }
    }
}

int InputLine::tpsetXFromOpt() const
{
    if(m_tpsetXOpt.has_value()){
        return m_tpsetXOpt.value();
    }

    switch(Widget::evalDir(m_cursorArgs.align, this)){
        case DIR_LEFTUP:
        case DIR_LEFT:
        case DIR_LEFTDOWN:
            {
                return 0;
            }
        case DIR_UP:
        case DIR_NONE:
        case DIR_DOWN:
            {
                return (w() - Widget::evalSize(m_cursorArgs.w, this)) / 2;
            }
        case DIR_RIGHTUP:
        case DIR_RIGHT:
        case DIR_RIGHTDOWN:
            {
                return w() - Widget::evalSize(m_cursorArgs.w, this);
            }
        default:
            {
                std::unreachable();
            }
    }
}

void InputLine::setFocus(bool argFocus)
{
    Widget::setFocus(argFocus);
    if(focus() && (Widget::evalInt(m_imeEnabled, this) == IME_SYSTEM)){
        g_sdlDevice->enableSystemIME(id());
    }
    else{
        g_sdlDevice->disableSystemIME(id());
    }
    m_cursorBlink = 0.0;
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

    const int tpsetX = tpx();
    const int tpsetY = tpy();

    const auto needDraw = mathf::cropROI(
            &srcCropX, &srcCropY,
            &srcCropW, &srcCropH,
            &dstCropX, &dstCropY,

            w(),
            h(),

            tpsetX,
            tpsetY,

            m_tpset.px() + m_tpset.pw(),
            m_tpset.py() + m_tpset.ph());

    if(needDraw){
        m_tpset.draw({.x=dstCropX, .y=dstCropY, .ro{srcCropX - tpsetX, srcCropY - tpsetY, srcCropW, srcCropH}});
    }

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    if(!focus()){
        return;
    }

    auto [cursorX, cursorY, cursorW, cursorH] = getCursorPLoc();

    cursorX += (m.x - m.ro->x);
    cursorY += (m.y - m.ro->y);

    if(mathf::rectangleOverlapRegion(m.x, m.y, m.ro->w, m.ro->h, cursorX, cursorY, cursorW, cursorH)){
        g_sdlDevice->fillRectangle(Widget::evalU32(m_cursorArgs.color, this), cursorX, cursorY, cursorW, cursorH);
    }

    if(g_clientArgParser->debugDrawInputLine){
        g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), m.x, m.y, w(), h());
    }
}

void InputLine::deleteChar()
{
    deleteInput();
}

void InputLine::insertChar(char ch)
{
    insertInput(std::string(1, ch));
}

void InputLine::insertUTF8String(const char *utf8Str)
{
    if(str_haschar(utf8Str)){
        insertInput(utf8Str);
    }
}

void InputLine::clear()
{
    m_cursor = 0;
    adjustCursorPLocX();
    m_cursorBlink = 0.0;

    if(!m_tpset.empty()){
        m_tpset.clear();
        notifyInputChange();
    }
}

void InputLine::setValidateFunc(std::function<bool(std::string, std::string)> validate)
{
    m_validate = std::move(validate);
}

void InputLine::setInput(const char *utf8Str)
{
    m_cursor = 0;
    m_cursorBlink = 0.0;

    m_tpset.clear();
    if(str_haschar(utf8Str)){
        m_cursor = m_tpset.insertUTF8String(m_cursor, 0, utf8Str);
    }

    adjustCursorPLocX();
    notifyInputChange();
}

int InputLine::tpx() const
{
    if(varWOpt()){
        return 0;
    }

    if(m_tpsetAlign.has_value()){
        switch(Widget::evalDir(m_tpsetAlign.value(), this)){
            case DIR_LEFTUP:
            case DIR_LEFT:
            case DIR_LEFTDOWN:
                {
                    return 0;
                }
            case DIR_UP:
            case DIR_NONE:
            case DIR_DOWN:
                {
                    return (w() - (m_tpset.px() + m_tpset.pw())) / 2;
                }
            case DIR_RIGHTUP:
            case DIR_RIGHT:
            case DIR_RIGHTDOWN:
                {
                    return w() - (m_tpset.px() + m_tpset.pw());
                }
            default:
                {
                    std::unreachable();
                }
        }
    }

    switch(Widget::evalDir(m_cursorArgs.align, this)){
        case DIR_LEFTUP:
        case DIR_LEFT:
        case DIR_LEFTDOWN:
            {
                if((w() >= m_tpset.px() + m_tpset.pw()) || !Widget::evalBool(m_cursorArgs.lazy, this) || !m_tpsetXOpt.has_value()){
                    return 0;
                }
                else{
                    return m_tpsetXOpt.value();
                }
            }
        case DIR_UP:
        case DIR_NONE:
        case DIR_DOWN:
            {
                if((w() >= m_tpset.px() + m_tpset.pw()) || !Widget::evalBool(m_cursorArgs.lazy, this) || !m_tpsetXOpt.has_value()){
                    return (w() - Widget::evalSize(m_cursorArgs.w, this)) / 2;
                }
                else{
                    return m_tpsetXOpt.value();
                }
            }
        case DIR_RIGHTUP:
        case DIR_RIGHT:
        case DIR_RIGHTDOWN:
            {
                if((w() >= m_tpset.px() + m_tpset.pw()) || !Widget::evalBool(m_cursorArgs.lazy, this) || !m_tpsetXOpt.has_value()){
                    return w() - Widget::evalSize(m_cursorArgs.w, this);
                }
                else{
                    return m_tpsetXOpt.value();
                }
            }
        default:
            {
                std::unreachable();
            }
    }
}

int InputLine::tpy() const
{
    if(varHOpt()){
        return 0;
    }

    switch(Widget::evalDir(m_tpsetAlign.has_value() ? m_tpsetAlign.value() : m_cursorArgs.align, this)){
        case DIR_UP:
        case DIR_UPLEFT:
        case DIR_UPRIGHT:
            {
                return 0;
            }
        case DIR_DOWN:
        case DIR_DOWNLEFT:
        case DIR_DOWNRIGHT:
            {
                return h() - (m_tpset.py() + (m_tpset.empty() ? m_tpset.getDefaultFontHeight() : m_tpset.ph()));
            }
        case DIR_LEFT:
        case DIR_RIGHT:
        case DIR_NONE:
            {
                return (h() - (m_tpset.py() + (m_tpset.empty() ? m_tpset.getDefaultFontHeight() : m_tpset.ph()))) / 2;
            }
        default:
            {
                std::unreachable();
            }
    }
}

std::tuple<int, int, int, int> InputLine::getCursorPLocInXMLTypeset() const
{
    if(m_tpset.empty()){
        return
        {
            0,
            0,

            Widget::evalSize(m_cursorArgs.w, this), // this enlarges board width
            m_tpset.getDefaultFontHeight(),
        };
    }

    const auto cursorH = m_tpset.getTokenCursorHk(std::max<int>(m_cursor - 1, 0), 0);
    const auto cursorY = m_tpset.lineStartY(0) - std::get<0>(cursorH) + 1;

    if(m_cursor < m_tpset.lineTokenCount(0)){
        const auto tkptr = m_tpset.getToken(m_cursor, 0);
        const auto cursorW = std::min<int>(Widget::evalSize(m_cursorArgs.w, this), m_tpset.getToken(m_cursor, 0)->box.width());
        return
        {
            tkptr->box.state.x - tkptr->box.state.w1,
            cursorY,

            cursorW,
            std::get<0>(cursorH) + std::get<1>(cursorH),
        };
    }

    // cursor is after last token, always keep cursor inside XMLTypeset, for simplicity
    // it may be fine to kep cursor outside of XMLTypeset, if InputLine is defined wider than internal XMLTypeset

    const auto tkptr = m_tpset.getLineBackToken(0);
    const auto cursorW = std::min<int>(Widget::evalSize(m_cursorArgs.w, this), tkptr->box.width());
    return
    {
        tkptr->box.state.x + tkptr->box.info.w + tkptr->box.state.w2 - cursorW,
        cursorY,

        cursorW,
        std::get<0>(cursorH) + std::get<1>(cursorH),
    };
}

std::tuple<int, int, int, int> InputLine::getCursorPLoc() const
{
    auto ploc = getCursorPLocInXMLTypeset();
    std::get<0>(ploc) += tpx();
    std::get<1>(ploc) += tpy();
    return ploc;
}
