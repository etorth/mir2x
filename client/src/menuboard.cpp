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

        int argCorner,
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
          nullptr,

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

          [argCorner](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::BLACK + colorf::A_SHF(255), dstDrawX, dstDrawY, self->w(), self->h(), std::max<int>(0, argCorner));
              g_sdlDevice->drawRectangle(colorf::GREY  + colorf::A_SHF(255), dstDrawX, dstDrawY, self->w(), self->h(), std::max<int>(0, argCorner));
          },

          this,
          false,
      }
{
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
    else if(p->second){
        return 0; // separator space not included
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
    m_canvas.addChild((new Widget
    {
        DIR_UPLEFT, // ignore
        0,
        0,

        {},
        {},
        {
            {new ShapeCropBoard
            {
                DIR_UPLEFT,
                0,
                0,
                [this](const Widget *)
                {
                    if(Widget::hasSize(m_canvas.varw())){
                        return m_canvas.w();
                    }

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
                    return upperItemSpace(argWidget) + argWidget->h() + lowerItemSpace(argWidget) + (argAddSeparator ? m_separatorSpace : 0);
                },

                [argWidget, argAddSeparator, this](const Widget *self, int drawDstX, int drawDstY)
                {
                    if(self->parent()->focus()){
                        g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(100), drawDstX, drawDstY + upperItemSpace(argWidget), self->w(), argWidget->h());
                    }

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
    })->setProcessEvent([this](Widget *self, const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
    {
        const auto roiOpt = self->cropDrawROI(startDstX, startDstY, roi);
        if(!roiOpt.has_value()){
            return false;
        }

        if(!valid){
            return self->consumeFocus(false);
        }

        Widget *menuWidget = nullptr;
        ShapeCropBoard *background = nullptr;

        self->foreachChild([&menuWidget, &background](Widget *child, bool)
        {

            if(auto p = dynamic_cast<ShapeCropBoard *>(child)){
                background = p;
            }
            else{
                menuWidget = child;
            }
        });

        if(menuWidget->processParentEvent(event, valid, w(), h(), startDstX, startDstY, roiOpt.value())){
            return self->consumeFocus(true, menuWidget);
        }

        switch(event.type){
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN:
                {
                    const auto [eventX, eventY] = SDLDeviceHelper::getEventPLoc(event).value();
                    if(background->parentIn(eventX, eventY, w(), h(), startDstX, startDstY, roiOpt.value())){
                        if(event.type == SDL_MOUSEMOTION){
                            return self->consumeFocus(true);
                        }
                        else{
                            if(m_onClickMenu){
                                m_onClickMenu(menuWidget);
                            }

                            setFocus(false);
                            setShow(false);
                            return true;
                        }
                    }
                    else{
                        return self->consumeFocus(false);
                    }
                }
            default:
                {
                    return false;
                }
        }
    }),
    true);
}
