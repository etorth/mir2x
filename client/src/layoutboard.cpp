#include "colorf.hpp"
#include "fflerror.hpp"
#include "fontexdb.hpp"
#include "layoutboard.hpp"
#include "log.hpp"
#include "mathf.hpp"
#include "strf.hpp"
#include "bevent.hpp"
#include "xmltypeset.hpp"

extern Log *g_log;
extern FontexDB *g_fontexDB;

void LayoutBoard::loadXML(const char *xmlString)
{
    if(!xmlString){
        throw fflerror("null xmlString");
    }

    m_parNodeList.clear();
    tinyxml2::XMLDocument xmlDoc;

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

    for(auto p = rootElem->FirstChild(); p; p = p->NextSibling()){
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

        addPar(parCount(), m_parNodeConfig.margin, p, false);
    }

    setupSize();
}

void LayoutBoard::addPar(int loc, const std::array<int, 4> &parMargin, const tinyxml2::XMLNode *node, bool updateSize)
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

    const int lineWidth = [elemNode, &parMargin, this]()
    {
        // even we use default size
        // we won't let it create one-line mode by default
        // user should explicitly note that they want this special mode

        int lineWidth = std::max<int>(m_parNodeConfig.lineWidth - (m_parNodeConfig.margin[2] + m_parNodeConfig.margin[3]), 1);
        elemNode->QueryIntAttribute("lineWidth", &lineWidth);

        // TODO should I force all lineWidth to be less than defaut lineWidth?
        //      should I jsut disable to support lineWidth attribute and force all typeset uses global lineWidth?

        if(lineWidth < -1){
            throw fflerror("invalid line width: %d", lineWidth);
        }
        return lineWidth;
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
        currNode->startY = parMargin[1];
    }
    else{
        const auto prevNode = std::prev(currNode);
        currNode->startY = prevNode->startY + prevNode->margin[0] + prevNode->margin[1] + prevNode->tpset->ph();
    }

    const int extraH = currNode->tpset->ph() + currNode->margin[0] + currNode->margin[1];
    for(auto p = std::next(currNode); p != m_parNodeList.end(); ++p){
        p->startY += extraH;
    }

    if(updateSize){
        setupSize();
    }
}

void LayoutBoard::addParXML(int loc, const std::array<int, 4> &parMargin, const char *xmlString)
{
    tinyxml2::XMLDocument xmlDoc;
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    addPar(loc, parMargin, xmlDoc.RootElement(), true);
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

        if(!mathf::ROICrop(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    0, node.startY, node.tpset->pw(), node.tpset->ph(), 0, 0, -1, -1)){
            continue;
        }
        node.tpset->drawEx(dstXCrop, dstYCrop, srcXCrop - node.margin[2], srcYCrop - node.startY, srcWCrop, srcHCrop);
    }
}

void LayoutBoard::setupSize()
{
    m_w = [this]() -> int
    {
        int maxW = 0;
        for(const auto &node: m_parNodeList){
            maxW = std::max<int>(maxW, node.margin[2] + node.tpset->pw() + node.margin[3]);
        }
        return maxW;
    }();

    m_h = [this]() -> int
    {
        if(empty()){
            return 0;
        }

        const auto &backNode = m_parNodeList.back();
        return backNode.startY + backNode.margin[0] + backNode.margin[1] + backNode.tpset->ph();
    }();
}

void LayoutBoard::setLineWidth(int lineWidth)
{
    m_parNodeConfig.lineWidth = lineWidth;
    for(auto &node: m_parNodeList){
        node.tpset->setLineWidth(lineWidth);
    }

    for(auto p = m_parNodeList.begin(); p != m_parNodeList.end(); ++p){
        if(p == m_parNodeList.begin()){
            p->startY = p->margin[0];
        }
        else{
            const auto prevNode = std::prev(p);
            p->startY = prevNode->startY + prevNode->margin[0] + prevNode->margin[1] + prevNode->tpset->ph();
        }
    }

    setupSize();
}

bool LayoutBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(m_parNodeList.empty()){
        return false;
    }

    const auto fnHandleEvent = [&event, this](parNode *node, bool currValid) -> bool
    {
        if(!currValid){
            node->tpset->clearEvent(-1);
            return false;
        }

        switch(event.type){
            case SDL_MOUSEMOTION:
                {
                    const int dx = event.motion.x - (x()                + node->margin[2]);
                    const int dy = event.motion.y - (y() + node->startY + node->margin[0]);

                    const auto [tokenX, tokenY] = node->tpset->locToken(dx, dy, true);
                    if(!node->tpset->tokenLocValid(tokenX, tokenY)){
                        node->tpset->clearEvent(-1);
                        return false;
                    }

                    const auto newEvent = (event.motion.state & SDL_BUTTON_LMASK) ? BEVENT_DOWN : BEVENT_ON;
                    const auto leafID = node->tpset->getToken(tokenX, tokenY)->leaf;

                    const auto oldEvent = node->tpset->markLeafEvent(leafID, newEvent);
                    if(const auto attrListPtr = node->tpset->leafEvent(leafID); attrListPtr && m_eventCB){
                        m_eventCB(*attrListPtr, oldEvent, newEvent);
                    }
                    node->tpset->clearEvent(leafID);
                    return true;
                }
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN:
                {
                    const int dx = event.button.x - (x()                + node->margin[2]);
                    const int dy = event.button.y - (y() + node->startY + node->margin[0]);

                    const auto [tokenX, tokenY] = node->tpset->locToken(dx, dy, true);
                    if(!node->tpset->tokenLocValid(tokenX, tokenY)){
                        node->tpset->clearEvent(-1);
                        return false;
                    }

                    const auto newEvent = (event.type == SDL_MOUSEBUTTONUP) ? BEVENT_ON : BEVENT_DOWN;
                    const auto leafID = node->tpset->getToken(tokenX, tokenY)->leaf;

                    const auto oldEvent = node->tpset->markLeafEvent(leafID, newEvent);
                    if(const auto attrListPtr = node->tpset->leafEvent(leafID); attrListPtr && m_eventCB){
                        m_eventCB(*attrListPtr, oldEvent, newEvent);
                    }
                    node->tpset->clearEvent(leafID);
                    return true;
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
    };

    bool takeEvent = false;
    for(auto &node: m_parNodeList){
        takeEvent |= fnHandleEvent(&node, valid && !takeEvent);
    }

    return takeEvent;
}
