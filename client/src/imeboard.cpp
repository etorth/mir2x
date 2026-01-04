#include <algorithm>
#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "imeboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

IMEBoard::IMEBoard(IMEBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .parent = std::move(args.parent),
      }}

    , m_font(std::move(args.font))

    , m_fontColorHover  (std::move(args.fontColorHover  ))
    , m_fontColorPressed(std::move(args.fontColorPressed))
    , m_separatorColor  (std::move(args.separatorColor  ))

    , m_fontTokenHeight([this]() -> size_t
      {
          return LabelBoard{{.label = u8" ", .font{m_font}}}.h();
      }())

    , m_inputWidget(nullptr)
    , m_onCommit   (nullptr)

    , m_upLeftCorner
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X09000006); },
          .parent{this},
      }}

    , m_downRightCorner
      {{
          .dir = DIR_DOWNRIGHT,
          .x = [this]{ return w() - 1; },
          .y = [this]{ return h() - 1; },

          .texLoadFunc = []{ return g_progUseDB->retrieve(0X09000009); },
          .parent{this},
      }}

    , m_bgImg
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X09000100); }, // size: 338 x 53
      }}

    , m_bg
      {{
          .getter = &m_bgImg,
          .vr
          {
              m_upLeftCorner.w(),
              10,
              m_bgImg.w() - m_upLeftCorner.w() - m_downRightCorner.w(),
              m_bgImg.h() - 10 * 2,
          },

          .resize
          {
              [this]{ return w() - m_upLeftCorner.w() - m_downRightCorner.w(); },
              [this]{ return h() - 10 * 2; },
          },

          .fgDrawFunc = [this](int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawLine(Widget::evalU32(m_separatorColor, this),
                      drawDstX          , drawDstY + m_startY + m_fontTokenHeight + m_separatorSpace / 2,
                      drawDstX + w() - 1, drawDstY + m_startY + m_fontTokenHeight + m_separatorSpace / 2);
          },

          .parent{this},
      }}
{
    dropFocus();
    setSize([this]{ return std::max<int>(m_bgImg.w(), m_startX * 2 + totalLabelWidth()); },
            [this]{ return std::max<int>(m_bgImg.h(), m_startY * 2 + m_fontTokenHeight + m_separatorSpace + m_fontTokenHeight); });
}

void IMEBoard::updateDefault(double)
{
    // in processEventDefault we only post request to IME
    // all IME changes need to be polled in update(), or we can do it by callback

    // problem of callback is not by IME, it's by SDL
    // SDL texture creation is not thread-safe, if we add callback like onCandidateListChanged, we can't allocate texture inside

    if(const auto currCandidateList = m_ime.candidates(); currCandidateList == m_candidateList){
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
        for(auto &p: m_boardList){
            this->removeChild(p->id(), false);
        }

        m_boardList.clear();
        m_candidateList = currCandidateList;

        m_startIndex = 0;
        prepareLabelBoardList();
    }
}

void IMEBoard::prepareLabelBoardList()
{
    /**/  int startX = m_startX;
    const int startY = m_startY + m_fontTokenHeight + m_separatorSpace;

    for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
        if(i >= m_boardList.size()){
            m_boardList.resize(i + 1);
        }

        if(!m_boardList[i]){
            m_boardList[i] = std::unique_ptr<LabelBoard>(new LabelBoard
            {{
                .label = str_printf(u8"%zu. %s", i + 1 - m_startIndex, m_candidateList[i].c_str()).c_str(),
                .font = m_font,
                .parent{this},
            }});
        }

        m_boardList[i]->moveTo(startX, startY);
        startX += m_boardList[i]->w() + m_candidateSpace;
    }
}

size_t IMEBoard::totalLabelWidth() const
{
    if(m_startIndex >= m_candidateList.size()){
        return 0;
    }

    size_t totalWidth = m_boardList[m_startIndex]->w();
    for(size_t i = m_startIndex + 1; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
        totalWidth += (m_candidateSpace + m_boardList[i]->w());
    }

    return totalWidth;
}

bool IMEBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
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
                        m_boardList[i]->setFontColor(Widget::evalU32(m_font.color, this));
                        if(m.create(m_boardList.at(i)->roi()).in(event.button.x, event.button.y)){
                            m_ime.select(i);
                        }
                    }
                }
                return true;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!m.in(event.button.x, event.button.y)){
                    dropFocus();
                    return true;
                }

                for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
                    if(m.create(m_boardList.at(i)->roi()).in(event.button.x, event.button.y)){
                        m_boardList.at(i)->setFontColor(Widget::evalU32(m_fontColorPressed, this));
                    }
                    else{
                        m_boardList.at(i)->setFontColor(Widget::evalU32(m_font.color, this));
                    }
                }
                return true;
            }
        case SDL_MOUSEMOTION:
            {
                if(event.motion.state & SDL_BUTTON_LMASK){
                    const auto remapXDiff = m.x - m.ro->x;
                    const auto remapYDiff = m.y - m.ro->y;

                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
                }
                else if(m.in(event.motion.x, event.motion.y)){
                    for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
                        if(m.create(m_boardList.at(i)->roi()).in(event.motion.x, event.motion.y)){
                            m_boardList.at(i)->setFontColor(Widget::evalU32(m_fontColorHover, this));
                        }
                        else{
                            m_boardList.at(i)->setFontColor(Widget::evalU32(m_font.color, this));
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

void IMEBoard::drawDefault(Widget::ROIMap m) const
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

    if(!m.calibrate(this)){
        return;
    }

    drawChild(&m_bg, m);

    const LabelBoard imeResult
    {{
        .label = to_u8cstr(m_ime.result()),
        .font = m_font,
    }};

    drawAsChild(&imeResult, DIR_UPLEFT, m_startX, m_startY, m);
    for(size_t i = m_startIndex; i < std::min<size_t>(m_startIndex + 9, m_candidateList.size()); ++i){
        drawChild(m_boardList.at(i).get(), m);
    }

    drawChild(&m_upLeftCorner   , m);
    drawChild(&m_downRightCorner, m);
}

void IMEBoard::gainFocus(std::string prefix, std::string input, Widget *pwidget, std::function<void(std::string)> onCommit)
{
    for(auto &p: m_boardList){
        this->removeChild(p->id(), false);
    }

    m_boardList.clear();
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
    for(auto &p: m_boardList){
        this->removeChild(p->id(), false);
    }

    m_boardList.clear();
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
