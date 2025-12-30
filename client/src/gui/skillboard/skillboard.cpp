#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "skillpage.hpp"
#include "skillboard.hpp"
#include "tritexbutton.hpp"
#include "magiciconbutton.hpp"
#include "textshadowboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SkillBoard::SkillBoard(int argX, int argY, ProcessRun *runPtr, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = argX,
          .y = argY,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(fflcheck(runPtr))
    , m_bg
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X05000000); },
          .parent{this},
      }}

    , m_pageCanvas
      {{
          .x = SkillBoard::getPageRectange().x,
          .y = SkillBoard::getPageRectange().y,
          .w = SkillBoard::getPageRectange().w,
          .h = SkillBoard::getPageRectange().h,

          .parent{this},
      }}

    , m_skillPageList([this]() -> std::vector<SkillPage *>
      {
          std::vector<SkillPage *> pageList;
          pageList.reserve(8);

          for(int i = 0; i < 8; ++i){
              auto pagePtr = new SkillPage
              {{
                  .pageTexID = to_u32(0X05000010 + i),

                  .config = std::addressof(m_config),
                  .proc = m_processRun,

                  .parent
                  {
                      .widget = std::addressof(m_pageCanvas),
                      .autoDelete = true,
                  },
              }};

              for(const auto &iconGfx: m_iconGfxList){
                  if(i == getSkillPageIndex(iconGfx.magicID)){
                      pagePtr->addIcon(iconGfx.magicID);
                  }
              }

              pagePtr->setShow([i, this]
              {
                  return m_selectedTabIndex == i;
              });

              pageList.push_back(pagePtr);
          }
          return pageList;
      }())

    , m_tabButtonList([this]() -> std::vector<TritexButton *>
      {
          std::vector<TritexButton *> tabButtonList;
          tabButtonList.reserve(8);

          const int tabX = 45;
          const int tabY = 10;
          const int tabW = 34;

          // don't know WTF
          // can't compile if use std::vector<TritexButton>
          // looks for complicated parameter list gcc can't forward correctly ???

          for(int i = 0; i < 8; ++i){
              tabButtonList.push_back(new TritexButton
              {{
                  .x = tabX + tabW * i,
                  .y = tabY,

                  .texIDList
                  {
                      .on   = 0X05000020 + to_u32(i),
                      .down = 0X05000030 + to_u32(i),
                  },

                  .onOverIn = [i, this]
                  {
                      m_cursorOnTabIndex = i;
                  },

                  .onOverOut = [i, this]
                  {
                      if(i != m_cursorOnTabIndex){
                          return;
                      }
                      m_cursorOnTabIndex = -1;
                  },

                  .onTrigger = [i, this](int)
                  {
                      if(m_selectedTabIndex == i){
                          return;
                      }

                      m_cursorOnTabIndex = -1, // radio mode button won't call onOverOut when pressed
                      m_tabButtonList.at(m_selectedTabIndex)->setOff();

                      m_selectedTabIndex = i;
                      m_slider.setValue(0, false);

                      for(auto pagePtr: m_skillPageList){
                          pagePtr->moveTo(0, 0);
                      }
                  },

                  .onClickDone = false,
                  .radioMode = true,
                  .alterColor = colorf::RGBA(255, 200, 255, 255),

                  .parent{this},
              }});

              if(i == m_selectedTabIndex){
                  m_tabButtonList.at(i)->setDown();
              }
          }
          return tabButtonList;
      }())

    , m_slider
      {{
          .bar
          {
              .x = 326,
              .y = 74,
              .w = 6,
              .h = 266,
          },

          .index = 0,
          .onChange = [this](float value)
          {
              auto canvasH = SkillBoard::getPageRectange().h;
              auto pagePtr = m_skillPageList.at(m_selectedTabIndex);

              if(pagePtr->h() > canvasH){
                  pagePtr->moveTo(0, to_d((canvasH - pagePtr->h()) * value)); // coordinate in canvas
              }
          },

          .parent{this},
      }}

    , m_closeButton
      {{
          .x = 317,
          .y = 402,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](int)
          {
              setShow(false);
          },

          .parent{this},
      }}
{
    setShow(false);
    setSize([this]{ return m_bg.w(); },
            [this]{ return m_bg.h(); });
}

void SkillBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    drawChild(&m_bg, m);

    drawTabName(m);

    drawChild(&m_slider, m);
    drawChild(&m_closeButton, m);

    for(auto buttonPtr: m_tabButtonList){
        drawChild(buttonPtr, m);
    }

    drawChild(&m_pageCanvas, m);
}

bool SkillBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_closeButton.processEventParent(event, valid, m)){
        consumeFocus(false);
        return true;
    }

    bool tabConsumed = false;
    for(auto buttonPtr: m_tabButtonList){
        tabConsumed |= buttonPtr->processEventParent(event, valid && !tabConsumed, m);
    }

    if(tabConsumed){
        return consumeFocus(true);
    }

    if(m_slider.processEventParent(event, valid, m)){
        return true;
    }

    if(m_pageCanvas.processEventParent(event, valid, m)){
        return consumeFocus(true);
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    moveBy(event.motion.xrel, event.motion.yrel, Widget::makeROI(0, 0, g_sdlDevice->getRendererSize()));
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            return consumeFocus(m.in(event.button.x, event.button.y));
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        case SDL_MOUSEWHEEL:
            {
                if(m.create(m_pageCanvas.roi(this)).in(event.wheel.mouseX, event.wheel.mouseY)){
                    m_slider.addValue(event.wheel.y * -0.1f, true);
                }
                return consumeFocus(true);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void SkillBoard::drawTabName(Widget::ROIMap m) const
{
    const LabelBoard tabName
    {{
        .label = to_u8cstr([this]() -> std::u8string
        {
            if(m_cursorOnTabIndex >= 0){
                return str_printf(u8"元素【%s】", to_cstr(magicElemName(cursorOnElem())));
            }

            if(m_selectedTabIndex >= 0){
                for(const auto magicIconPtr: m_skillPageList.at(m_selectedTabIndex)->getMagicIconButtonList()){
                    if(magicIconPtr->cursorOn()){
                        if(const auto &mr = DBCOM_MAGICRECORD(magicIconPtr->magicID())){
                            return str_printf(u8"元素【%s】%s", str_haschar(mr.elem) ? to_cstr(mr.elem) : "无", to_cstr(mr.name));
                        }
                        else{
                            return str_printf(u8"元素【无】");
                        }
                    }
                }
                return str_printf(u8"元素【%s】", to_cstr(magicElemName(selectedElem())));
            }

            // fallback
            // shouldn't reach here
            return str_printf(u8"元素【无】");
        }()),

        .font
        {
            .id = 1,
            .size = 12,
        },
    }};

    drawAsChild(&tabName, DIR_UPLEFT, 30, 400, m);
}
