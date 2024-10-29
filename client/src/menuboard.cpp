#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "menuboard.hpp"

extern SDLDevice *g_sdlDevice;

MenuBoard::MenuBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argVarW,
        std::array<int, 4> argMargin,

        int argItemSpace,
        int argSeperatorSpace,

        std::initializer_list<std::tuple<Widget *, bool, bool>> argMenuItemList,
        std::function<void(Widget *)> argOnClickMenu,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }

    , m_itemSpace     (std::max<int>(0, argItemSpace     ))
    , m_separatorSpace(std::max<int>(0, argSeperatorSpace))

    , m_onClickMenu(std::move(argOnClickMenu))

    , m_canvas
      {
          DIR_UPLEFT, // ignored
          0,
          0,

          [argVarW = std::move(argVarW), argMargin, this]() -> Widget::VarSize
          {
              fflassert(argMargin[2] >= 0, argMargin);
              fflassert(argMargin[3] >= 0, argMargin);

              if(Widget::hasIntSize(argVarW)){
                  return std::max<int>(0, Widget::asIntSize(argVarW) - argMargin[2] - argMargin[3]);
              }
              else if(Widget::hasFuncSize(argVarW)){
                  return [argVarW = std::move(argVarW), argMargin, this](const Widget *)
                  {
                      return std::max<int>(0, Widget::asFuncSize(argVarW)(this) - argMargin[2] - argMargin[3]);
                  };
              }
              else{
                  return {};
              }
          }(),

          false,
      }

    , m_wrapper
      {
          DIR_UPLEFT,
          0,
          0,

          &m_canvas,
          false,

          argMargin,

          this,
          false,
      }

    , m_background
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return m_wrapper.w(); },
          [this](const Widget *){ return m_wrapper.h(); },

          [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::BLACK + colorf::A_SHF(255), dstDrawX, dstDrawY, self->w(), self->h());
          },

          this,
          false,
      }

    , m_frame
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return m_wrapper.w(); },
          [this](const Widget *){ return m_wrapper.h(); },

          [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->drawRectangle(colorf::GREY + colorf::A_SHF(128), dstDrawX, dstDrawY, self->w(), self->h());
          },

          this,
          false,
      }
{
    moveFront(&m_frame);
    moveFront(&m_wrapper);
    moveFront(&m_background);

    for(auto [widget, addSeparator, autoDelete]: argMenuItemList){
        appendMenu(widget, addSeparator, autoDelete);
    }
}

int MenuBoard::upperItemSpace(const Widget *argWidget) const
{
    const auto p = std::find_if(m_itemList.begin(), m_itemList.end(), [argWidget](const auto &pr)
    {
        return pr.first == argWidget;
    });

    if(p == m_itemList.end()){
        throw fflerror("can not find child widget: %p", to_cvptr(argWidget));
    }
    else if(p == m_itemList.begin()){
        return 0;
    }
    else if(std::prev(p)->second){
        return 0;
    }
    else{
        return m_itemSpace / 2;
    }
}

int MenuBoard::lowerItemSpace(const Widget *argWidget) const
{
    const auto p = std::find_if(m_itemList.begin(), m_itemList.end(), [argWidget](const auto &pr)
    {
        return pr.first == argWidget;
    });

    if(p == m_itemList.end()){
        throw fflerror("can not find child widget: %p", to_cvptr(argWidget));
    }
    else if(std::next(p) == m_itemList.end()){
        return 0;
    }
    else if(std::next(p)->second){
        return 0;
    }
    else{
        return m_itemSpace - m_itemSpace / 2;
    }
}

void MenuBoard::appendMenu(Widget *argWidget, bool argAddSeparator, bool argAutoDelete)
{
    if(!argWidget){
        return;
    }

    m_itemList.emplace_back(argWidget, argAddSeparator);
    m_canvas.appendItem(new Widget
    {
        DIR_UPLEFT, // ignore
        0,
        0,

        {},
        {},
        {
            {new ShapeClipBoard
            {
                DIR_UPLEFT,
                0,
                0,
                [this](const Widget *)
                {
                    if(m_itemList.empty()){
                        return 0;
                    }

                    auto foundIter = std::max_element(m_itemList.begin(), m_itemList.end(), [](const auto &item1, const auto &item2)
                    {
                        return item1.first->w() < item2.first->w();
                    });

                    return foundIter->first->w();
                },

                [argWidget, argAddSeparator, this](const Widget *)
                {
                    return upperItemSpace(argWidget) + argWidget->h() + lowerItemSpace(argWidget);
                },

                [argWidget, argAddSeparator, this](const Widget *self, int drawDstX, int drawDstY)
                {
                    g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(100), drawDstX, drawDstY + upperItemSpace(argWidget), self->w(), argWidget->h());
                    if(argAddSeparator){
                        const int xOff = 2;
                        const int drawXOff = (self->w() > xOff * 2) ? xOff : 0;

                        const int drawXStart = drawDstX + drawXOff;
                        const int drawXEnd   = drawDstX + self->w() - drawXOff;
                        const int drawY      = drawDstY + upperItemSpace(argWidget) + argWidget->h() + m_separatorSpace / 2;

                        g_sdlDevice->drawLine(colorf::WHITE + colorf::A_SHF(100), drawXStart, drawY, drawXEnd, drawY);
                    }
                },
            }, DIR_UPLEFT, 0, 0, true},

            {argWidget, DIR_UPLEFT, 0, [argWidget, this](const Widget *)
            {
                return upperItemSpace(argWidget);
            }, argAutoDelete},
        },
    },

    true);
}

// bool MenuBoard::processEvent(const SDL_Event &event, bool valid)
// {
//     if(!valid){
//         return consumeFocus(false);
//     }
//
//     if(!show()){
//         return consumeFocus(false);
//     }
//
//     if(Widget::processEvent(event, valid)){
//         if(event.type == SDL_MOUSEBUTTONDOWN){
//             if(auto p = focusedChild()){
//                 if(m_onClickMenu){
//                     m_onClickMenu(p);
//                 }
//
//                 setShow(false);
//                 setFocus(false);
//             }
//         }
//         return true;
//     }
//
//     switch(event.type){
//         case SDL_MOUSEMOTION:
//         case SDL_MOUSEBUTTONUP:
//         case SDL_MOUSEBUTTONDOWN:
//             {
//                 const auto [eventX, eventY] = SDLDeviceHelper::getEventPLoc(event).value();
//                 if(!in(eventX, eventY)){
//                     setFocus(false);
//                     return event.type == SDL_MOUSEMOTION;
//                 }
//
//                 // event drops into margin
//                 // we should drop focus but still consume the event
//
//                 if(!mathf::pointInRectangle<int>(
//                             eventX,
//                             eventY,
//
//                             x() + m_margin[2],
//                             y() + m_margin[0],
//
//                             w() - m_margin[2] - m_margin[3],
//                             h() - m_margin[0] - m_margin[1])){
//                     return !consumeFocus(false);
//                 }
//
//                 return foreachChild([&event, eventX, eventY, this](Widget *widget, bool)
//                 {
//                     if(mathf::pointInRectangle(eventX, eventY, widget->x(), widget->y() - m_itemSpace / 2, w() - m_margin[2] - m_margin[3], widget->h() + m_itemSpace)){
//                         if(event.type == SDL_MOUSEBUTTONDOWN){
//                             if(m_onClickMenu){
//                                 m_onClickMenu(widget);
//                             }
//
//                             setShow(false);
//                             setFocus(false);
//                         }
//                         else{
//                             consumeFocus(true, widget);
//                         }
//                         return true;
//                     }
//                     return false;
//                 });
//             }
//         default:
//             {
//                 return false;
//             }
//     }
// }
