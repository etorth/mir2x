#include <algorithm>
#include "imeboard.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

IMEBoard::IMEBoard(
        dir8_t argDir,
        int argX,
        int argY,

        uint8_t font,
        uint8_t fontSize,
        uint8_t fontStyle,

        uint32_t fontColor,
        uint32_t fontColorHover,
        uint32_t fontColorPressed,

        uint32_t separatorColor,

        Widget *parent,
        bool autoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          0,
          0,

          {},

          parent,
          autoDelete,
      }

    , m_font(font)
    , m_fontSize(fontSize)
    , m_fontStyle(fontStyle)

    , m_fontColor(fontColor)
    , m_fontColorHover(fontColorHover)
    , m_fontColorPressed(fontColorPressed)

    , m_separatorColor(separatorColor)

    , m_fontTokenHeight([this]() -> size_t
      {
          return LabelBoard(DIR_UPLEFT, 0, 0, u8" ", m_font, m_fontSize, m_fontStyle, m_fontColor).h();
      }())

    , m_inputWidget(nullptr)
    , m_onCommit(nullptr)
{
    dropFocus();
    updateSize();
}

void IMEBoard::update(double)
{
    // in processEventDefault we only post request to IME
    // all IME changes need to be polled in update(), or we can do it by callback

    // problem of callback is not by IME, it's by SDL
    // SDL texture creation is not thread-safe, if we add callback like onCandidateListChanged, we can't allocate texture inside

    if(const auto currCandidateList = m_ime.candidateList(); currCandidateList == m_candidateList){
        if(m_ime.empty()){
            dropFocus();
        }
        else if(m_ime.done()){
            if(m_onCommit){
                m_onCommit(m_ime.result());
            }
            dropFocus();
        }
    }
    else{
        for(auto &p: m_labelBoardList){
            this->removeChild(p.get(), false);
        }

        m_labelBoardList.clear();
        m_candidateList = currCandidateList;

        m_startIndex = 0;
        prepareLabelBoardList();
    }
}

void IMEBoard::updateSize()
{
    const auto [minW, minH] = []() -> std::tuple<int, int>
    {
        if(auto frame = g_progUseDB->retrieve(0X09000100)){
            return SDLDeviceHelper::getTextureSize(frame);
        }
        else{
            return {338, 53}; // texture size of 0X09000100
        }
    }();

    setW(std::max<int>(minW, m_startX * 2 + totalLabelWidth()));
    setH(std::max<int>(minH, m_startY * 2 + m_fontTokenHeight + m_separatorSpace + m_fontTokenHeight));
}

void IMEBoard::prepareLabelBoardList()
{
    int startX = m_startX;
    int startY = m_startY + m_fontTokenHeight + m_separatorSpace;

    for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
        if(i >= m_labelBoardList.size()){
            m_labelBoardList.resize(i + 1);
        }

        if(!m_labelBoardList[i]){
            m_labelBoardList[i] = std::unique_ptr<LabelBoard>(new LabelBoard
            {
                DIR_UPLEFT,
                0,
                0,

                str_printf(u8"%zu. %s", i + 1 - m_startIndex, m_candidateList[i].c_str()).c_str(),

                m_font,
                m_fontSize,
                m_fontStyle,
                m_fontColor,

                this,
                false,
            });
        }

        m_labelBoardList[i]->moveTo(startX, startY);
        startX += m_labelBoardList[i]->w() + m_candidateSpace;
    }

    updateSize();
}

size_t IMEBoard::totalLabelWidth() const
{
    if(m_startIndex >= m_candidateList.size()){
        return 0;
    }

    size_t totalWidth = m_labelBoardList[m_startIndex]->w();
    for(size_t i = m_startIndex + 1; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
        totalWidth += (m_candidateSpace + m_labelBoardList[i]->w());
    }

    return totalWidth;
}

bool IMEBoard::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!valid){
        if(focus()){
            dropFocus();
        }
        return false;
    }

    if(!focus()){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_UP:
                    case SDLK_LEFT:
                    case SDLK_PAGEUP:
                        {
                            if(m_startIndex >= 9){
                                m_startIndex -= 9;
                            }
                            else{
                                m_startIndex = 0;
                            }

                            prepareLabelBoardList();
                            return true;
                        }
                    case SDLK_DOWN:
                    case SDLK_RIGHT:
                    case SDLK_PAGEDOWN:
                        {
                            if(m_startIndex + 9 < m_candidateList.size()){
                                m_startIndex += 9;
                            }

                            prepareLabelBoardList();
                            return true;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_onCommit){
                                m_onCommit(m_ime.result());
                            }

                            dropFocus();
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            m_ime.backspace();
                            return true;
                        }
                    case SDLK_ESCAPE:
                        {
                            dropFocus();
                            return true;
                        }
                    case SDLK_SPACE:
                        {
                            m_ime.select(m_startIndex);
                            return true;
                        }
                    default:
                        {
                            if(const char keyChar = SDLDeviceHelper::getKeyChar(event, true); keyChar >= 'a' && keyChar <= 'z'){
                                m_ime.feed(keyChar);
                            }
                            else if(keyChar >= '1' && keyChar <= '9'){
                                m_ime.select(m_startIndex + keyChar - '1');
                            }
                            else if(keyChar != '\0'){
                                if(m_onCommit){
                                    m_onCommit(m_ime.result());
                                    m_onCommit(str_printf("%c", keyChar));
                                }
                                dropFocus();
                            }
                            return true;
                        }
                }
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(event.button.button == SDL_BUTTON_LEFT){
                    for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
                        m_labelBoardList[i]->setFontColor(m_fontColor);
                        if(m_labelBoardList[i]->parentIn(event.button.x, event.button.y, w(), h(), startDstX, startDstY, roiOpt.value())){
                            m_ime.select(i);
                        }
                    }
                }
                return true;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!in(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value())){
                    dropFocus();
                    return true;
                }

                for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
                    if(m_labelBoardList.at(i)->parentIn(event.button.x, event.button.y, w(), h(), startDstX, startDstY, roiOpt.value())){
                        m_labelBoardList.at(i)->setFontColor(m_fontColorPressed);
                    }
                    else{
                        m_labelBoardList.at(i)->setFontColor(m_fontColor);
                    }
                }
                return true;
            }
        case SDL_MOUSEMOTION:
            {
                if(event.motion.state & SDL_BUTTON_LMASK){
                    const auto remapXDiff = startDstX - roiOpt->x;
                    const auto remapYDiff = startDstY - roiOpt->y;

                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
                }
                else if(in(event.motion.x, event.motion.y, startDstX, startDstY, roiOpt.value())){
                    for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
                        if(m_labelBoardList.at(i)->parentIn(event.motion.x, event.motion.y, w(), h(), startDstX, startDstY, roiOpt.value())){
                            m_labelBoardList.at(i)->setFontColor(m_fontColorHover);
                        }
                        else{
                            m_labelBoardList.at(i)->setFontColor(m_fontColor);
                        }
                    }
                }
                return true;
            }
        default:
            {
                return true;
            }
    }
}

void IMEBoard::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    //        +---------------------------------------------------------------------------------- m_startX
    //        |
    //        |                                                                  +--------------- w - m_startX
    //        |                                                                  |
    //        v                                                                  v
    // +-----------+---------------------------------------------------------------------+
    // |           |                                                                     |
    // |   +-------+                                                                     |
    // |   |  +---------------+                                                          | <----- m_startY
    // |   |  |你好woshinidaye|                                                          |
    // +---+  +---------------+                                                          | ------
    // |                                                                                 |   ^
    // +---------------------------------------------------------------------------------+   |    m_separatorSpace
    // |                                                                                 |   v
    // |      +------+ +------+ +------+ +----+ +----+ +----+ +----+ +----+ +----+       | ------
    // |      |1.我是| |2.我司| |3.我市| |4.我| |5.恶| |6.额| |7.俄| |8.鳄| |9.娥|   +---+
    // |      +------+ +------+ +------+ +----+ +----+ +----+ +----+ +----+ +----+   |   |
    // |                                                                      +------+   |
    // |                                                                      |          |
    // +----------------------------------------------------------------------+----------+
    //            -->| |<-- m_candidateSpace

    if(auto frame = g_progUseDB->retrieve(0X09000100)){
        // tex  +-----+-----------------------+-----+
        //      |     |                       |     |
        //      +-----+-----------------------+-----+ size:(338, 53)
        //      |<--->|       repeatWidth     |<--->|
        //    marginWidth                   marginWidth
        //
        // ime  +-----+-----+-----+-----+-----+-----+
        //      |     |  0  |  1  |  2  | ... |     |
        //      +-----+-----+-----+-----+-----+-----+ size:(w(), h())
        //      |<--->|      coveredWidth     |<--->|
        //    marginWidth                   marginWidth

        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(frame);

        fflassert(w() >= texW, w(), texW);
        fflassert(h() >= texH, h(), texH);

        const auto repeatWidth = std::min<int>(300, texW);
        const auto marginWidth = (texW + 1 - repeatWidth) / 2;
        const auto coveredWidth = w() - marginWidth * 2;

        g_sdlDevice->drawTexture(frame, dstX,                     dstY, marginWidth, h(),                         0, 0, marginWidth, texH);
        g_sdlDevice->drawTexture(frame, dstX + w() - marginWidth, dstY, marginWidth, h(), marginWidth + repeatWidth, 0, marginWidth, texH);

        const auto repeat = (coveredWidth + 1) / repeatWidth;
        for(int i = 0; i < repeat; ++i){
            const auto dstCurrWidth = [i, repeat, coveredWidth]()
            {
                if(i + 1 == repeat){
                    return coveredWidth - (coveredWidth / repeat) * i;
                }
                else{
                    return coveredWidth / repeat;
                }
            }();
            g_sdlDevice->drawTexture(frame, dstX + marginWidth + (coveredWidth / repeat) * i, dstY, dstCurrWidth, h(), marginWidth, 0, repeatWidth, texH);
        }
    }

    LabelBoard(
            DIR_UPLEFT,
            dstX + to_d(m_startX),
            dstY + to_d(m_startY),

            to_u8cstr(m_ime.result()),

            m_font,
            m_fontSize,
            m_fontStyle,
            m_fontColor).drawRoot(0, 0);

    g_sdlDevice->drawLine(m_separatorColor,
            dstX      , dstY + m_startY + m_fontTokenHeight + m_separatorSpace / 2,
            dstX + w(), dstY + m_startY + m_fontTokenHeight + m_separatorSpace / 2);

    for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
        drawChildEx(m_labelBoardList.at(i).get(), dstX, dstY, roi);
    }

    if(auto tex = g_progUseDB->retrieve(0X09000006)){
        g_sdlDevice->drawTexture(tex, dstX, dstY);
    }

    if(auto tex = g_progUseDB->retrieve(0X09000009)){
        g_sdlDevice->drawTexture(tex, DIR_DOWNRIGHT,  dstX + w(), dstY + h());
    }
}

void IMEBoard::gainFocus(std::string prefix, std::string input, Widget *pwidget, std::function<void(std::string)> onCommit)
{
    for(auto &p: m_labelBoardList){
        this->removeChild(p.get(), false);
    }

    m_labelBoardList.clear();
    m_candidateList.clear();

    m_ime.assign(prefix, input);

    m_inputWidget = pwidget;
    m_onCommit = std::move(onCommit);

    if(m_inputWidget){
        m_inputWidget->setFocus(false);
    }

    setShow(true);
    setFocus(true);
}

void IMEBoard::dropFocus()
{
    for(auto &p: m_labelBoardList){
        this->removeChild(p.get(), false);
    }

    m_labelBoardList.clear();
    m_candidateList.clear();

    m_ime.clear();

    if(m_inputWidget){
        m_inputWidget->setFocus(true);
    }

    m_inputWidget = nullptr;
    m_onCommit = nullptr;

    setShow(false);
    setFocus(false);
}
