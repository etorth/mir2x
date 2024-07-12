#include "log.hpp"
#include "totype.hpp"
#include "strf.hpp"
#include "xmlf.hpp"
#include "xmltypeset.hpp"
#include "labelboard.hpp"

extern Log *g_log;

void LabelBoard::setText(const char8_t *format, ...)
{
    std::u8string text;
    str_format(format, text);
    loadXML(xmlf::toParString("%s", text.empty() ? "" : to_cstr(text)).c_str());
}

void LabelBoard::loadXML(const char *xmlString)
{
    // use the fallback values of m_tpset
    // don't need to specify the font/size/style info here

    m_tpset.loadXML(xmlString);
    m_w = m_tpset.px() + m_tpset.pw();
    m_h = m_tpset.py() + m_tpset.ph();
}

bool LabelBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if(focus() && in(event.motion.x, event.motion.y)){
                    return consumeFocus(true);
                }
                return false;
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(in(event.button.x, event.button.y));
            }
        default:
            {
                return false;
            }
    }
}
