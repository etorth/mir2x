#include <cmath>
#include <utf8.h>
#include "colorf.hpp"
#include "fflerror.hpp"
#include "fontexdb.hpp"
#include "layoutboard.hpp"
#include "imeboard.hpp"
#include "sdldevice.hpp"
#include "log.hpp"
#include "mathf.hpp"
#include "strf.hpp"
#include "bevent.hpp"
#include "xmltypeset.hpp"

extern Log *g_log;
extern FontexDB *g_fontexDB;
extern IMEBoard *g_imeBoard;
extern SDLDevice *g_sdlDevice;

void LayoutBoard::loadXML(const char *xmlString, size_t parLimit)
{
    if(!xmlString){
        throw fflerror("null xmlString");
    }

    m_parNodeList.clear();
    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);

    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    bool layoutXML = false;
    const auto rootElem = xmlDoc.RootElement();

    for(const char *cstr: {"layout", "Layout", "LAYOUT"}){
        if(std::string(rootElem->Name()) == cstr){
            layoutXML = true;
            break;
        }
    }

    if(!layoutXML){
        throw fflerror("string is not layout xml");
    }

    if(rootElem->FirstAttribute()){
        g_log->addLog(LOGTYPE_WARNING, "Layout XML doesn't accept attributes, ignored.");
    }

    size_t addedParCount = 0;
    for(auto p = rootElem->FirstChild(); p && (parLimit == 0 || addedParCount < parLimit); p = p->NextSibling()){
        if(!p->ToElement()){
            g_log->addLog(LOGTYPE_WARNING, "Not an element: %s", p->Value());
            continue;
        }

        bool parXML = false;
        for(const char *cstr: {"par", "Par", "PAR"}){
            if(std::string(p->Value()) == cstr){
                parXML = true;
                break;
            }
        }

        if(!parXML){
            g_log->addLog(LOGTYPE_WARNING, "Not a paragraph element: %s", p->Value());
            continue;
        }

        addPar(parCount(), m_parNodeConfig.margin, p);
        addedParCount++;
    }
}

void LayoutBoard::addPar(int loc, const std::array<int, 4> &parMargin, const tinyxml2::XMLNode *node)
{
    if(loc < 0 || loc > parCount()){
        throw fflerror("invalid location: %d", loc);
    }

    if(!node){
        throw fflerror("null xml node");
    }

    const auto elemNode = node->ToElement();
    if(!elemNode){
        throw fflerror("can't cast to XMLElement");
    }

    bool parXML = false;
    for(const char *cstr: {"par", "Par", "PAR"}){
        if(std::string(elemNode->Name()) == cstr){
            parXML = true;
            break;
        }
    }

    if(!parXML){
        throw fflerror("not a paragraph node");
    }

    const int lineWidth = [elemNode, this]()
    {
        if(m_parNodeConfig.lineWidth <= 0){
            return -1;
        }

        // even we use default size
        // we won't let it create one-line mode by default
        // user should explicitly note that they want this special mode

        if(int lineWidth = m_parNodeConfig.lineWidth - m_parNodeConfig.margin[2] - m_parNodeConfig.margin[3]; lineWidth <= 0){
            throw fflerror("invalid default paragraph parameters");
        }
        else{
            elemNode->QueryIntAttribute("lineWidth", &lineWidth);
            if(lineWidth < -1){
                throw fflerror("invalid line width: %d", lineWidth);
            }
            return lineWidth;
        }
    }();

    const int lineAlign = [elemNode, this]()
    {
        if(const auto val = elemNode->Attribute("align")){
            if(std::string(val) == "left"       ) return LALIGN_LEFT;
            if(std::string(val) == "right"      ) return LALIGN_RIGHT;
            if(std::string(val) == "center"     ) return LALIGN_CENTER;
            if(std::string(val) == "justify"    ) return LALIGN_JUSTIFY;
            if(std::string(val) == "distributed") return LALIGN_DISTRIBUTED;
        }
        return m_parNodeConfig.align;
    }();

    const bool canThrough = [elemNode, this]()
    {
        bool canThrough = m_parNodeConfig.canThrough;
        elemNode->QueryBoolAttribute("canThrough", &canThrough);
        return canThrough;
    }();

    const uint8_t font = [elemNode, this]()
    {
        int font = m_parNodeConfig.font;
        if(const auto val = elemNode->Attribute("font")){
            try{
                font = std::stoi(val);
            }
            catch(...){
                font = g_fontexDB->findFontName(val);
            }
        }

        if(g_fontexDB->hasFont(font)){
            return font;
        }
        return 0;
    }();

    const uint8_t fontSize = [elemNode, this]()
    {
        int fontSize = m_parNodeConfig.fontSize;
        elemNode->QueryIntAttribute("size", &fontSize);
        return fontSize;
    }();

    const uint32_t fontColor = [elemNode, this]()
    {
        if(const char *val = elemNode->Attribute("color")){
            return colorf::string2RGBA(val);
        }
        return m_parNodeConfig.fontColor;
    }();

    const uint32_t fontBGColor = [elemNode, this]()
    {
        if(const char *val = elemNode->Attribute("bgcolor")){
            return colorf::string2RGBA(val);
        }
        return m_parNodeConfig.fontBGColor;
    }();

    const uint8_t fontStyle = m_parNodeConfig.fontStyle;

    const int lineSpace = [elemNode, this]()
    {
        int lineSpace = m_parNodeConfig.lineSpace;
        elemNode->QueryIntAttribute("lineSpace", &lineSpace);
        return lineSpace;
    }();

    const int wordSpace = [elemNode, this]()
    {
        int wordSpace = m_parNodeConfig.lineSpace;
        elemNode->QueryIntAttribute("wordSpace", &wordSpace);
        return wordSpace;
    }();

    auto parNodePtr = std::make_unique<XMLTypeset>
    (
        lineWidth,
        lineAlign,
        canThrough,
        font,
        fontSize,
        fontStyle,
        fontColor,
        fontBGColor,
        colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF),
        lineSpace,
        wordSpace
    );

    parNodePtr->loadXMLNode(node);
    auto currNode = m_parNodeList.insert(ithParIterator(loc), {-1, parMargin, std::move(parNodePtr)});

    if(currNode == m_parNodeList.begin()){
        currNode->startY = currNode->margin[0];
    }
    else{
        const auto prevNode = std::prev(currNode);
        currNode->startY = prevNode->startY + std::max<int>(prevNode->tpset->ph(), prevNode->tpset->getDefaultFontHeight()) + prevNode->margin[1] + currNode->margin[0];
    }

    const int extraH = std::max<int>(currNode->tpset->ph(), currNode->tpset->getDefaultFontHeight()) + currNode->margin[0] + currNode->margin[1];
    for(auto p = std::next(currNode); p != m_parNodeList.end(); ++p){
        p->startY += extraH;
    }
}

void LayoutBoard::setupStartY(int fromPar)
{
    fflassert(fromPar >= 0, fromPar);
    if(fromPar < parCount()){
        int offsetY = [this, fromPar]
        {
            if(fromPar == 0){
                return 0;
            }

            auto prevNode = std::prev(ithParIterator(fromPar));
            return prevNode->startY + std::max<int>(prevNode->tpset->ph(), prevNode->tpset->getDefaultFontHeight()) + prevNode->margin[1];
        }();

        for(auto par = ithParIterator(fromPar); par != m_parNodeList.end(); ++par){
            offsetY += par->margin[0];
            par->startY = offsetY;

            offsetY += par->margin[1];
            offsetY += std::max<int>(par->tpset->ph(), par->tpset->getDefaultFontHeight());
        }
    }
}

void LayoutBoard::addParXML(int loc, const std::array<int, 4> &parMargin, const char *xmlString)
{
    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    addPar(loc, parMargin, xmlDoc.RootElement());
}

void LayoutBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    for(const auto &node: m_parNodeList){
        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(!mathf::cropROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    node.margin[2], node.startY, node.tpset->pw(), node.tpset->ph())){
            continue;
        }
        node.tpset->drawEx(dstXCrop, dstYCrop, srcXCrop - node.margin[2], srcYCrop - node.startY, srcWCrop, srcHCrop);
    }

    if(m_canEdit && focus()){
        Widget::drawEx(dstX, dstY, srcX, srcY, srcW, srcH); // draw cursor
    }
}

void LayoutBoard::setLineWidth(int lineWidth)
{
    m_parNodeConfig.lineWidth = lineWidth;
    for(auto &node: m_parNodeList){
        node.tpset->setLineWidth(lineWidth);
    }
    setupStartY(0);
}

bool LayoutBoard::processEventDefault(const SDL_Event &event, bool valid)
{
    if(m_parNodeList.empty()){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(!focus()){
                    return false;
                }

                if(!m_canEdit){
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
                            if((event.key.keysym.mod & KMOD_LSHIFT) || (event.key.keysym.mod & KMOD_RSHIFT)){
                                auto currPar = ithParIterator(m_cursorLoc.par);
                                auto  newPar = currPar->tpset->split(m_cursorLoc.x, m_cursorLoc.y);

                                m_parNodeList.insert(currPar, {-1, currPar->margin, std::unique_ptr<XMLTypeset>(newPar)});
                                setupStartY(m_cursorLoc.par);

                                m_cursorLoc.par++;
                                m_cursorLoc.x = 0;
                                m_cursorLoc.y = 0;
                            }
                            else{
                                if(m_onCR){
                                    m_onCR();
                                }
                            }

                            m_cursorBlink = 0.0;
                            return consumeFocus(true);
                        }
                    case SDLK_LEFT:
                        {
                            if(m_cursorLoc.x == 0 && m_cursorLoc.y == 0){
                                if(m_cursorLoc.par == 0){
                                }
                                else{
                                    m_cursorLoc.par--;
                                    std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->lastCursorLoc();
                                }
                            }
                            else{
                                std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->prevCursorLoc(m_cursorLoc.x, m_cursorLoc.y);
                            }

                            m_cursorBlink = 0.0;
                            return consumeFocus(true);
                        }
                    case SDLK_RIGHT:
                        {
                            if(auto par = ithParIterator(m_cursorLoc.par); std::tie(m_cursorLoc.x, m_cursorLoc.y) == par->tpset->lastCursorLoc()){
                                if(m_cursorLoc.par + 1 >= parCount()){
                                }
                                else{
                                    m_cursorLoc.par++;
                                    std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->firstCursorLoc();
                                }
                            }
                            else{
                                std::tie(m_cursorLoc.x, m_cursorLoc.y) = par->tpset->nextCursorLoc(m_cursorLoc.x, m_cursorLoc.y);
                            }

                            m_cursorBlink = 0.0;
                            return consumeFocus(true);
                        }
                    case SDLK_BACKSPACE:
                        {
                            if(m_cursorLoc.x > 0){
                                ithParIterator(m_cursorLoc.par)->tpset->deleteToken(m_cursorLoc.x - 1, m_cursorLoc.y, 1);
                                if(m_cursorLoc.x > 1 || m_cursorLoc.y == 0){
                                    m_cursorLoc.x--;
                                }
                                else{
                                    m_cursorLoc.y--;
                                    m_cursorLoc.x = ithParIterator(m_cursorLoc.par)->tpset->lineTokenCount(m_cursorLoc.y);
                                }
                            }
                            else{
                                if(m_cursorLoc.y != 0){
                                    throw fflerror("invalid cursor location: par %d, x %d, y %d", m_cursorLoc.par, m_cursorLoc.x, m_cursorLoc.y);
                                }

                                if(m_cursorLoc.par > 0){
                                    m_parNodeList.erase(ithParIterator(m_cursorLoc.par));
                                    m_cursorLoc.par--;
                                    std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->lastCursorLoc();
                                }
                            }

                            setupStartY(m_cursorLoc.par);
                            m_cursorBlink = 0.0;
                            return consumeFocus(true);
                        }
                    case SDLK_ESCAPE:
                        {
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            const char keyChar = SDLDeviceHelper::getKeyChar(event, true);
                            const auto fnInsertString = [this](std::string s)
                            {
                                ithParIterator(m_cursorLoc.par)->tpset->insertUTF8String(m_cursorLoc.x, m_cursorLoc.y, s.c_str());
                                for(size_t i = 0, count = utf8::distance(s.begin(), s.end()); i < count; ++i){
                                    std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->nextCursorLoc(m_cursorLoc.x, m_cursorLoc.y);
                                }
                                setupStartY(m_cursorLoc.par);
                            };

                            if(m_imeEnabled && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
                                g_imeBoard->gainFocus("", str_printf("%c", keyChar), this, [fnInsertString, this](std::string s)
                                {
                                    fnInsertString(std::move(s));
                                });
                            }
                            else if(keyChar != '\0'){
                                fnInsertString(str_printf("%c", keyChar));
                            }

                            m_cursorBlink = 0.0;
                            return consumeFocus(true);
                        }
                }
            }
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                const auto newEvent = [&event]
                {
                    if(event.type == SDL_MOUSEMOTION) return (event.motion.state & SDL_BUTTON_LMASK) ? BEVENT_DOWN : BEVENT_ON;
                    else                              return (event.type == SDL_MOUSEBUTTONDOWN)     ? BEVENT_DOWN : BEVENT_ON;
                }();

                const auto [eventPX, eventPY] = SDLDeviceHelper::getEventPLoc(event).value();
                const auto fnHandleEvent = [newEvent, eventPX, eventPY, this](ParNode *node, bool currValid) -> bool
                {
                    if(!currValid){
                        node->tpset->clearEvent(-1);
                        return false;
                    }

                    const int xOff = eventPX - (x() + node->margin[2]);
                    const int yOff = eventPY - (y() + node->startY);

                    const auto [tokenX, tokenY] = node->tpset->locToken(xOff, yOff, true);

                    if(!node->tpset->tokenLocValid(tokenX, tokenY)){
                        node->tpset->clearEvent(-1);
                        return false;
                    }

                    const auto leafID = node->tpset->getToken(tokenX, tokenY)->leaf;
                    const auto oldEvent = node->tpset->markLeafEvent(leafID, newEvent);

                    const static std::map<std::pair<int, int>, int> buttonState2Event
                    {
                        {{BEVENT_OFF , BEVENT_ON  }, BEVENT_ENTER  },
                        {{BEVENT_ON  , BEVENT_OFF }, BEVENT_LEAVE  },
                        {{BEVENT_ON  , BEVENT_DOWN}, BEVENT_PRESS  },
                        {{BEVENT_DOWN, BEVENT_ON  }, BEVENT_RELEASE},
                        {{BEVENT_ON  , BEVENT_ON  }, BEVENT_HOVER  },
                    };

                    if(const auto attrListPtr = node->tpset->leafEvent(leafID); attrListPtr && m_eventCB){
                        if(auto eventiter = buttonState2Event.find({oldEvent, newEvent}); eventiter != buttonState2Event.end()){
                            m_eventCB(*attrListPtr, eventiter->second);
                        }
                    }
                    node->tpset->clearEvent(leafID);
                    return true;
                };

                bool takeEvent = false;
                for(auto &node: m_parNodeList){
                    takeEvent |= fnHandleEvent(&node, valid && !takeEvent);
                }

                if(!takeEvent){
                    takeEvent = in(eventPX, eventPY);
                }

                if(takeEvent){
                    return consumeFocus(true);
                }
                return false;
            }

        default:
            {
                // layout board only handle mouse motion/click events
                // ignore any other unexcepted events

                // for GNOME3 I found sometimes here comes SDL_KEYMAPCHANGED after SDL_MOUSEBUTTONDOWN
                // need to ignore this event, don't call clearEvent()
                return false;
            }
    }
}

void LayoutBoard::drawCursorBlink(int drawDstX, int drawDstY) const
{
    if(!m_canEdit){
        return;
    }

    if(!cursorLocValid(m_cursorLoc)){
        throw fflerror("invalid cursor location: par %d, x %d, y %d", m_cursorLoc.par, m_cursorLoc.x, m_cursorLoc.y);
    }

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    const auto [cursorPX, cursorPY, cursorPH] = [this]() -> std::tuple<int, int, int>
    {
        auto par = ithParIterator(m_cursorLoc.par);
        if(par->tpset->empty()){
            return
            {
                par->margin[2],
                par->startY,

                par->tpset->getDefaultFontHeight(),
            };
        }
        else if(m_cursorLoc.x < par->tpset->lineTokenCount(m_cursorLoc.y)){
            const auto tokenPtr = par->tpset->getToken(m_cursorLoc.x, m_cursorLoc.y);
            return
            {
                par->margin[2] + tokenPtr->Box.State.X - tokenPtr->Box.State.W1,
                par->startY    + tokenPtr->Box.State.Y,

                par->tpset->getToken(std::max<int>(0, m_cursorLoc.x - 1), m_cursorLoc.y)->Box.Info.H,
            };
        }
        else{
            // cursor is after last token
            // we should draw it inside widget

            const auto tokenPtr = par->tpset->getToken(m_cursorLoc.x - 1, m_cursorLoc.y);
            return
            {
                par->margin[2] + tokenPtr->Box.State.X + tokenPtr->Box.Info.W + tokenPtr->Box.State.W2 - m_cursorWidth,
                par->startY    + tokenPtr->Box.State.Y,

                tokenPtr->Box.Info.H,
            };
        }
    }();

    g_sdlDevice->fillRectangle(colorf::RED + colorf::A_SHF(255), drawDstX + cursorPX, drawDstY + cursorPY, m_cursorWidth, cursorPH);
}

std::string LayoutBoard::getXML() const
{
    std::string xmlString = "<layout>";
    for(const auto &node: m_parNodeList){
        xmlString.append(node.tpset->getXML());
    }

    xmlString.append("</layout>");
    return xmlString;
}

const char * LayoutBoard::findAttrValue(const std::unordered_map<std::string, std::string> &attrList, const char *key, const char *valDefault)
{
    if(auto p = attrList.find(key); p != attrList.end()){
        return p->second.c_str();
    }
    return valDefault;
}
