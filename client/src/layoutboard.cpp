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
#include "clientargparser.hpp"

extern Log *g_log;
extern FontexDB *g_fontexDB;
extern IMEBoard *g_imeBoard;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

LayoutBoard::LayoutBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        int argLineWidth,

        const char *argInitXML,
        size_t argParLimit,

        std::array<int, 4> argMargin,

        bool argCanSelect,
        bool argCanEdit,
        bool argIMEEnabled,
        bool argCanThrough,

        uint8_t  argFont,
        uint8_t  argFontSize,
        uint8_t  argFontStyle,
        uint32_t argFontColor,
        uint32_t argFontBGColor,

        int argLineAlign,
        int argLineSpace,
        int argWordSpace,

        int      argCursorWidth,
        uint32_t argCursorColor,

        std::function<void()> argOnTab,
        std::function<void()> argOnCR,
        std::function<void(const std::unordered_map<std::string, std::string> &, int)> argEventCB,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          [this](const Widget *)
          {
              int maxW = 0;
              for(const auto &node: m_parNodeList){
                  maxW = std::max<int>(maxW, node.margin[2] + node.tpset->pw() + node.margin[3]);
              }

              if(m_canEdit){
                  maxW = std::max<int>(maxW, m_cursorWidth);
              }

              return maxW;
          },

          [this](const Widget *)
          {
              if(empty()){
                  if(m_canEdit){
                      throw fflerror("editable layout shall have at least one par");
                  }
                  return 0;
              }

              const auto &backNode = m_parNodeList.back();
              return backNode.startY + std::max<int>(backNode.tpset->ph(), m_canEdit ? backNode.tpset->getDefaultFontHeight() : 0) + backNode.margin[1];
          },

          {},

          argParent,
          argAutoDelete,
      }

    , m_parNodeConfig
      {
          argLineWidth,
          argMargin,
          argCanThrough,
          argFont,
          argFontSize,
          argFontStyle,
          argFontColor,
          argFontBGColor,
          argLineAlign,
          argLineSpace,
          argWordSpace,
      }

    , m_cursorClip
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return this->w(); },
          [this](const Widget *){ return this->h(); },

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              drawCursorBlink(drawDstX, drawDstY);
          },

          this,
          false,
      }

    , m_canSelect(argCanSelect)
    , m_canEdit(argCanEdit)
    , m_imeEnabled(argIMEEnabled)

    , m_cursorWidth(argCursorWidth)
    , m_cursorColor(argCursorColor)

    , m_onTab(std::move(argOnTab))
    , m_onCR(std::move(argOnCR))
    , m_eventCB(std::move(argEventCB))
{
    disableSetSize();

    for(size_t i = 0; i < m_parNodeConfig.margin.size(); ++i){
        if(m_parNodeConfig.margin[i] < 0){
            throw fflerror("invalid parNodeConfig::margin[%zu]: %d", i, m_parNodeConfig.margin[i]);
        }
    }

    if((m_parNodeConfig.lineWidth > 0) && (m_parNodeConfig.lineWidth <= m_parNodeConfig.margin[2] + m_parNodeConfig.margin[3])){
        throw fflerror("invalid default paragraph parameters");
    }

    if(argInitXML){
        loadXML(argInitXML, argParLimit);
    }

    if(m_canEdit){
        if(empty()){
            loadXML("<layout><par></par></layout>");
        }

        m_cursorLoc.par = parCount() - 1;
        std::tie(m_cursorLoc.x, m_cursorLoc.y) = m_parNodeList.rbegin()->tpset->lastCursorLoc();
    }
}

void LayoutBoard::updateGfx()
{
    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    for(int parIndex = 0, parTotalCount = parCount(); parIndex < parTotalCount; ++parIndex){
        auto parNodeIter = ithParIterator(parIndex);
        auto xmlNode = parNodeIter->tpset->getXMLNode()->DeepClone(&xmlDoc);

        m_parNodeList.erase(parNodeIter);
        addPar(parIndex, m_parNodeConfig.margin, xmlNode);
    }
}

void LayoutBoard::loadXML(const char *xmlString, size_t parLimit)
{
    fflassert(str_haschar(xmlString));

    m_parNodeList.clear();
    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);

    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    bool layoutXML = false;
    const auto rootElem = xmlDoc.RootElement();

    for(const char *cstr: {"layout", "Layout", "LAYOUT"}){
        if(to_sv(rootElem->Name()) == cstr){
            layoutXML = true;
            break;
        }
    }

    if(!layoutXML){
        throw fflerror("string is not layout xml");
    }

    if(rootElem->FirstAttribute()){
        g_log->addLog(LOGTYPE_WARNING, "Layout XML doesn't accept attributes, ignored");
    }

    size_t addedParCount = 0;
    for(auto p = rootElem->FirstChild(); p && (parLimit == 0 || addedParCount < parLimit); p = p->NextSibling()){
        if(!p->ToElement()){
            g_log->addLog(LOGTYPE_WARNING, "Not an element: %s", p->Value());
            continue;
        }

        bool parXML = false;
        for(const char *cstr: {"par", "Par", "PAR"}){
            if(to_sv(p->Value()) == cstr){
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
        if(to_sv(elemNode->Name()) == cstr){
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
            if(to_sv(val) == "left"       ) return LALIGN_LEFT;
            if(to_sv(val) == "right"      ) return LALIGN_RIGHT;
            if(to_sv(val) == "center"     ) return LALIGN_CENTER;
            if(to_sv(val) == "justify"    ) return LALIGN_JUSTIFY;
            if(to_sv(val) == "distributed") return LALIGN_DISTRIBUTED;
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

void LayoutBoard::setFont(uint8_t argFont)
{
    m_parNodeConfig.font = argFont;
    updateGfx();
}

void LayoutBoard::setFontSize(uint8_t argFontSize)
{
    m_parNodeConfig.fontSize = argFontSize;
    updateGfx();
}

void LayoutBoard::setFontStyle(uint8_t argFontStyle)
{
    m_parNodeConfig.fontStyle = argFontStyle;
    updateGfx();
}

void LayoutBoard::setFontColor(uint32_t argFontColor)
{
    m_parNodeConfig.fontColor = argFontColor;
    updateGfx();
}

void LayoutBoard::setFontBGColor(uint32_t argFontBGColor)
{
    m_parNodeConfig.fontBGColor = argFontBGColor;
    updateGfx();
}

void LayoutBoard::setLineWidth(int argLineWidth)
{
    const auto cursorOff = m_canEdit ? ithParIterator(m_cursorLoc.par)->tpset->cursorLoc2Off(m_cursorLoc.x, m_cursorLoc.y) : -1;
    m_parNodeConfig.lineWidth = argLineWidth;

    for(auto &node: m_parNodeList){
        node.tpset->setLineWidth(argLineWidth);
    }

    setupStartY(0);
    if(m_canEdit){
        std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->cursorOff2Loc(cursorOff);
    }
}

bool LayoutBoard::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY)
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

                            if(!g_clientArgParser->disableIME && m_imeEnabled && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
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
                const auto fnHandleEvent = [&event, newEvent, eventPX, eventPY, startDstX, startDstY, this](ParNode *node, bool currValid) -> bool
                {
                    if(!currValid){
                        node->tpset->clearEvent(-1);
                        return false;
                    }

                    const int xOff = eventPX - (startDstX + node->margin[2]);
                    const int yOff = eventPY - (startDstY + node->startY);

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

                    const auto attrListPtr = node->tpset->leafEvent(leafID);
                    if(attrListPtr && m_eventCB){
                        if(auto eventiter = buttonState2Event.find({oldEvent, newEvent}); eventiter != buttonState2Event.end()){
                            m_eventCB(*attrListPtr, eventiter->second);
                        }
                    }

                    node->tpset->clearEvent(leafID);
                    if(!attrListPtr && event.type == SDL_MOUSEMOTION){
                        // it's not an event text, and no click happens
                        // don't take the event
                        return false;
                    }
                    return true;
                };

                bool takeEvent = false;
                for(auto &node: m_parNodeList){
                    takeEvent |= fnHandleEvent(&node, valid && !takeEvent);
                }

                if(!takeEvent && event.type != SDL_MOUSEMOTION){
                    takeEvent = in(eventPX, eventPY, startDstX, startDstY);
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
                par->margin[2] + tokenPtr->box.state.x - tokenPtr->box.state.w1,
                par->startY    + tokenPtr->box.state.y,

                par->tpset->getToken(std::max<int>(0, m_cursorLoc.x - 1), m_cursorLoc.y)->box.info.h,
            };
        }
        else{
            // cursor is after last token
            // we should draw it inside widget

            const auto tokenPtr = par->tpset->getToken(m_cursorLoc.x - 1, m_cursorLoc.y);
            return
            {
                par->margin[2] + tokenPtr->box.state.x + tokenPtr->box.info.w + tokenPtr->box.state.w2 - m_cursorWidth,
                par->startY    + tokenPtr->box.state.y,

                tokenPtr->box.info.h,
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

std::string LayoutBoard::getText(bool textOnly) const
{
    std::string text;
    for(const auto &node: m_parNodeList){
        if(!text.empty()){
            text.append(" ");
        }
        text.append(node.tpset->getText(textOnly));
    }
    return text;
}

const char * LayoutBoard::findAttrValue(const std::unordered_map<std::string, std::string> &attrList, const char *key, const char *valDefault)
{
    if(auto p = attrList.find(key); p != attrList.end()){
        return p->second.c_str();
    }
    return valDefault;
}
