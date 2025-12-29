#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "menuboard.hpp"

extern SDLDevice *g_sdlDevice;

MenuBoard::MenuBoard(MenuBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .parent = std::move(args.parent),
      }}

    , m_itemSpace     (std::move(args.itemSpace     ))
    , m_separatorSpace(std::move(args.separatorSpace))

    , m_onClickMenu(std::move(args.onClick))
    , m_canvas
      {{
          .fixed = Widget::transform(std::move(args.fixed), [left = args.margin.left, right = args.margin.right, this](int w)
          {
              return w - Widget::evalSize(left, this) - Widget::evalSize(right, this);
          }),
      }}

    , m_wrapper
      {{
          .wrapped{&m_canvas},
          .margin = std::move(args.margin),

          .bgDrawFunc = [corner = args.corner](const Widget *self, int dstDrawX, int dstDrawY)
          {
              const int w = self->w();
              const int h = self->h();
              const int c = Widget::evalSize(corner, self);

              g_sdlDevice->fillRectangle(colorf::BLACK_A255, dstDrawX, dstDrawY, w, h, c);
              g_sdlDevice->drawRectangle(colorf:: GREY_A255, dstDrawX, dstDrawY, w, h, c);
          },
          .parent{this},
      }}
{
    moveFront(&m_wrapper);
    for(auto [widget, addSeparator, autoDelete]: args.itemList){
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
        return Widget::evalSize(m_itemSpace, this) / 2;
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
        return (Widget::evalSize(m_itemSpace, this) + 1) / 2;
    }
}

void MenuBoard::appendMenu(Widget *argWidget, bool argAddSeparator, bool argAutoDelete)
{
    if(!argWidget){
        return;
    }

    m_itemList.emplace_back(argWidget, argAddSeparator);
    m_canvas.addChild(new Widget
    {{
        .w = std::nullopt,
        .h = std::nullopt,

        .childList
        {
            {new GfxShapeBoard
            {{
                .w = [this]
                {
                    if(m_canvas.varWOpt().has_value()){
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

                .h = [argWidget, argAddSeparator, this]
                {
                    return upperItemSpace(argWidget) + argWidget->h() + lowerItemSpace(argWidget) + (argAddSeparator ? Widget::evalSize(m_separatorSpace, this) : 0);
                },

                .drawFunc = [argWidget, argAddSeparator, this](const Widget *self, int drawDstX, int drawDstY)
                {
                    if(self->parent()->focus()){
                        g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(100), drawDstX, drawDstY + upperItemSpace(argWidget), self->w(), argWidget->h());
                    }

                    if(argAddSeparator){
                        const int xOff = 2;
                        const int drawXOff = (self->w() > xOff * 2) ? xOff : 0;

                        const int drawXStart = drawDstX + drawXOff;
                        const int drawXEnd   = drawDstX + self->w() - drawXOff;
                        const int drawY      = drawDstY + upperItemSpace(argWidget) + argWidget->h() + Widget::evalSize(m_separatorSpace, this) / 2;

                        g_sdlDevice->drawLine(colorf::WHITE + colorf::A_SHF(100), drawXStart, drawY, drawXEnd, drawY);
                    }
                },
            }}, DIR_UPLEFT, 0, 0, true},

            {argWidget, DIR_UPLEFT, 0, [argWidget, this]
            {
                return upperItemSpace(argWidget);
            }, argAutoDelete},
        },

        .attrs
        {
            .inst
            {
                .processEvent = [this](Widget *self, const SDL_Event &event, bool valid, Widget::ROIMap m)
                {
                    if(!m.calibrate(self)){
                        return false;
                    }

                    if(!valid){
                        return self->consumeFocus(false);
                    }

                    Widget *menuWidget = nullptr;
                    GfxShapeBoard *background = nullptr;

                    self->foreachChild([&menuWidget, &background](Widget *child, bool)
                    {

                        if(auto p = dynamic_cast<GfxShapeBoard *>(child)){
                            background = p;
                        }
                        else{
                            menuWidget = child;
                        }
                    });

                    if(menuWidget->processEventParent(event, valid, m)){
                        return self->consumeFocus(true, menuWidget);
                    }

                    switch(event.type){
                        case SDL_MOUSEMOTION:
                        case SDL_MOUSEBUTTONUP:
                        case SDL_MOUSEBUTTONDOWN:
                            {
                                if(m.create(background->roi()).in(SDLDeviceHelper::getEventPLoc(event).value())){
                                    if(event.type == SDL_MOUSEMOTION){
                                        return self->consumeFocus(true);
                                    }
                                    else{
                                        MenuBoard::evalClickCBFunc(m_onClickMenu, menuWidget);
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
                },
            }
        },
    }}, true);
}
