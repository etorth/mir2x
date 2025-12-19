#include <cmath>
#include <algorithm>
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

LayoutBoard::LayoutBoard(LayoutBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = [this]
          {
              int maxW = 0;
              for(const auto &node: m_parNodeList){
                  maxW = std::max<int>(maxW, node.margin[2] + node.tpset->pw() + node.margin[3]);
              }

              if(Widget::evalBool(m_canEdit, this)){
                  maxW = std::max<int>(maxW, m_cursorWidth);
              }
              return maxW;
          },

          .h = [this]
          {
              if(empty()){
                  if(Widget::evalBool(m_canEdit, this)){
                      throw fflerror("editable layout shall have at least one par");
                  }
                  return 0;
              }

              const auto &backNode = m_parNodeList.back();
              /* */ auto lastPerHeight = backNode.tpset->ph();

              if(Widget::evalBool(m_canEdit, this)){
                    lastPerHeight = std::max<int>(lastPerHeight, backNode.tpset->getDefaultFontHeight());
              }
              return backNode.startY + lastPerHeight + backNode.margin[1];
          },

          .attrs
          {
              .type
              {
                  .canSetSize = false,
              },
          },
          .parent = std::move(args.parent),
      }}

    , m_parNodeConfig
      {
          fflcheck(args.lineWidth, args.lineWidth >= 0),
          fflcheck(args.margin, std::ranges::all_of(std::views::iota(0, 4), [&args](auto i){ return args.margin[i] >= 0; })),

          args.canThrough,

          args.font.id,
          args.font.size,
          args.font.style,

          std::move(args.font.color),
          std::move(args.font.bgColor),

          fflcheck(args.lineAlign, args.lineAlign >= LALIGN_NONE && args.lineAlign < LALIGN_END),
          fflcheck(args.lineSpace, args.lineSpace >= 0),
          fflcheck(args.wordSpace, args.wordSpace >= 0),
      }

    , m_cursorClip
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *, int drawDstX, int drawDstY)
          {
              drawCursorBlink(drawDstX, drawDstY);
          },

          .parent{this},
      }}

    , m_canSelect (std::move(args.canSelect))
    , m_canEdit   (std::move(args.canEdit  ))
    , m_imeEnabled(std::move(args.enableIME))

    , m_cursorWidth(args.cursor.width)
    , m_cursorColor(std::move(args.cursor.color))

    , m_onTab(std::move(args.onTab))
    , m_onCR(std::move(args.onCR))
    , m_onCursorMove(std::move(args.onCursorMove))
    , m_eventCB(std::move(args.onClickText))
{
    if(str_haschar(args.initXML)){
        loadXML(args.initXML, args.parLimit);
    }

    if(Widget::evalBool(m_canEdit, this)){
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
    addLayoutXML(0, m_parNodeConfig.margin, xmlString, parLimit);
}

void LayoutBoard::addPar(int loc, const Widget::IntMargin &parMargin, const tinyxml2::XMLNode *node)
{
    if(loc < 0 || loc > parCount()){
        throw fflerror("invalid location: %d", loc);
    }

    if(!std::ranges::all_of(std::views::iota(0, 4), [&parMargin](auto i){ return parMargin[i] >= 0; })){
        throw fflerror("invalid margin: %d %d %d %d", parMargin[0], parMargin[1], parMargin[2], parMargin[3]);
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
        // even we use default size
        // we won't let it create one-line mode by default
        // user should explicitly note that they want this special mode

        int lineWidth = m_parNodeConfig.lineWidth;
        elemNode->QueryIntAttribute("lineWidth", &lineWidth);

        if(lineWidth >= 0){
            return lineWidth;
        }
        throw fflerror("invalid line width: %d", lineWidth);
    }();

    const auto lineAlign = [elemNode, this]() -> int
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

        if(fontSize >= 0 && fontSize < 255){
            return fontSize;
        }
        throw fflerror("invalid font size: %d", fontSize);
    }();

    const uint8_t fontStyle = m_parNodeConfig.fontStyle; // TODO

    auto fontColor = [elemNode, this]() -> Widget::VarU32
    {
        if(const char *val = elemNode->Attribute("color")){
            return colorf::string2RGBA(val);
        }
        return m_parNodeConfig.fontColor;
    }();

    auto fontBGColor = [elemNode, this]() -> Widget::VarU32
    {
        if(const char *val = elemNode->Attribute("bgcolor")){
            return colorf::string2RGBA(val);
        }
        return m_parNodeConfig.fontBGColor;
    }();

    const int lineSpace = [elemNode, this]()
    {
        int lineSpace = m_parNodeConfig.lineSpace;
        elemNode->QueryIntAttribute("lineSpace", &lineSpace);

        if(lineSpace >= 0){
            return lineSpace;
        }
        throw fflerror("invalid line space: %d", lineSpace);
    }();

    const int wordSpace = [elemNode, this]()
    {
        int wordSpace = m_parNodeConfig.lineSpace;
        elemNode->QueryIntAttribute("wordSpace", &wordSpace);

        if(wordSpace >= 0){
            return wordSpace;
        }
        throw fflerror("invalid word space: %d", wordSpace);
    }();

    auto parNodePtr = std::make_unique<XMLTypeset>
    (
        lineWidth,
        lineAlign,
        canThrough,
        font,
        fontSize,
        fontStyle,
        std::move(fontColor),
        std::move(fontBGColor),
        colorf::WHITE_A255,
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

void LayoutBoard::addParXML(int loc, const Widget::IntMargin &parMargin, const char *xmlString)
{
    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    addPar(loc, parMargin, xmlDoc.RootElement());
}

size_t LayoutBoard::addLayoutXML(int loc, const Widget::IntMargin &parMargin, const char *xmlString, size_t parLimit)
{
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

        addPar(loc++, parMargin, p);
        addedParCount++;
    }
    return addedParCount;
}

void LayoutBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    for(const auto &node: m_parNodeList){
        int dstXCrop = m.x;
        int dstYCrop = m.y;
        int srcXCrop = m.ro->x;
        int srcYCrop = m.ro->y;
        int srcWCrop = m.ro->w;
        int srcHCrop = m.ro->h;

        if(!mathf::cropROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    node.margin[2], node.startY, node.tpset->pw(), node.tpset->ph())){
            continue;
        }
        node.tpset->draw({.x=dstXCrop, .y=dstYCrop, .ro{srcXCrop - node.margin[2], srcYCrop - node.startY, srcWCrop, srcHCrop}});
    }

    if(Widget::evalBool(m_canEdit, this) && focus()){
        Widget::drawDefault(m); // draw cursor
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

void LayoutBoard::setFontColor(Widget::VarU32 argFontColor)
{
    m_parNodeConfig.fontColor = std::move(argFontColor);
    updateGfx();
}

void LayoutBoard::setFontBGColor(Widget::VarU32 argFontBGColor)
{
    m_parNodeConfig.fontBGColor = std::move(argFontBGColor);
    updateGfx();
}

void LayoutBoard::setLineWidth(int argLineWidth)
{
    const auto cursorOff = Widget::evalBool(m_canEdit, this) ? ithParIterator(m_cursorLoc.par)->tpset->cursorLoc2Off(m_cursorLoc.x, m_cursorLoc.y) : -1;
    m_parNodeConfig.lineWidth = argLineWidth;

    for(auto &node: m_parNodeList){
        node.tpset->setLineWidth(argLineWidth);
    }

    setupStartY(0);
    if(Widget::evalBool(m_canEdit, this)){
        std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->cursorOff2Loc(cursorOff);
        if(m_onCursorMove){
            m_onCursorMove();
        }
    }
}

bool LayoutBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(m_parNodeList.empty()){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(!focus()){
                    return false;
                }

                if(!Widget::evalBool(m_canEdit, this)){
                    return false;
                }

                if(!valid){
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

                                if(m_onCursorMove){
                                    m_onCursorMove();
                                }
                            }
                            else{
                                if(m_onCR){
                                    m_onCR();
                                }
                            }

                            m_cursorBlink = 0.0;
                            return true;
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

                            if(m_onCursorMove){
                                m_onCursorMove();
                            }

                            m_cursorBlink = 0.0;
                            return true;
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

                            if(m_onCursorMove){
                                m_onCursorMove();
                            }

                            m_cursorBlink = 0.0;
                            return true;
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

                            if(m_onCursorMove){
                                m_onCursorMove();
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
                            const auto fnInsertString = [this](std::string s)
                            {
                                ithParIterator(m_cursorLoc.par)->tpset->insertUTF8String(m_cursorLoc.x, m_cursorLoc.y, s.c_str());
                                for(size_t i = 0, count = utf8::distance(s.begin(), s.end()); i < count; ++i){
                                    std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->nextCursorLoc(m_cursorLoc.x, m_cursorLoc.y);
                                }

                                setupStartY(m_cursorLoc.par);

                                if(m_onCursorMove){
                                    m_onCursorMove();
                                }
                            };

                            if(!g_clientArgParser->disableIME && Widget::evalBool(m_imeEnabled, this) && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
                                g_imeBoard->gainFocus("", str_printf("%c", keyChar), this, [fnInsertString, this](std::string s)
                                {
                                    fnInsertString(std::move(s));
                                });
                            }
                            else if(keyChar != '\0'){
                                fnInsertString(str_printf("%c", keyChar));
                            }

                            m_cursorBlink = 0.0;
                            return true;
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
                const auto fnHandleEvent = [&event, newEvent, eventPX, eventPY, m, this](ParNode *node, bool currValid) -> bool
                {
                    if(!currValid){
                        node->tpset->clearEvent(-1);
                        return false;
                    }

                    const int xOff = eventPX - (m.x + node->margin[2]);
                    const int yOff = eventPY - (m.y + node->startY);

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
                    takeEvent = m.in(eventPX, eventPY);
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
    if(!Widget::evalBool(m_canEdit, this)){
        return;
    }

    if(!cursorLocValid(m_cursorLoc)){
        throw fflerror("invalid cursor location: par %d, x %d, y %d", m_cursorLoc.par, m_cursorLoc.x, m_cursorLoc.y);
    }

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    const auto [cursorPX, cursorPY, cursorPW, cursorPH] = getCursorPLoc();
    g_sdlDevice->fillRectangle(colorf::RED + colorf::A_SHF(255), drawDstX + cursorPX, drawDstY + cursorPY, cursorPW, cursorPH);
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

std::string LayoutBoard::getText() const
{
    std::vector<std::string> lines;
    for(const auto &node: m_parNodeList){
        if(node.tpset->empty()){
            lines.push_back({});
        }
        else{
            lines.push_back(node.tpset->getText());
        }
    }
    return str_join(lines, "\n");
}

const char * LayoutBoard::findAttrValue(const std::unordered_map<std::string, std::string> &attrList, const char *key, const char *valDefault)
{
    if(auto p = attrList.find(key); p != attrList.end()){
        return p->second.c_str();
    }
    return valDefault;
}

std::tuple<int, int> LayoutBoard::getCursorOff() const
{
    if(!cursorLocValid(m_cursorLoc)){
        throw fflerror("invalid cursor location: par %d, x %d, y %d", m_cursorLoc.par, m_cursorLoc.x, m_cursorLoc.y);
    }
    return {m_cursorLoc.par, ithParIterator(m_cursorLoc.par)->tpset->cursorLoc2Off(m_cursorLoc.x, m_cursorLoc.y)};
}

std::tuple<int, int, int> LayoutBoard::getCursorLoc() const
{
    return {m_cursorLoc.par, m_cursorLoc.x, m_cursorLoc.y};
}

std::tuple<int, int, int, int> LayoutBoard::getCursorPLoc() const
{
    if(auto par = ithParIterator(m_cursorLoc.par); par->tpset->empty()){
        return
        {
            par->margin[2],
            par->startY,

            m_cursorWidth,
            par->tpset->getDefaultFontHeight(),
        };
    }
    else if(m_cursorLoc.x < par->tpset->lineTokenCount(m_cursorLoc.y)){
        const auto tokenPtr = par->tpset->getToken(m_cursorLoc.x, m_cursorLoc.y);
        return
        {
            par->margin[2] + tokenPtr->box.state.x - tokenPtr->box.state.w1,
            par->startY    + tokenPtr->box.state.y,

            m_cursorWidth,
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

            m_cursorWidth,
            tokenPtr->box.info.h,
        };
    }
}
