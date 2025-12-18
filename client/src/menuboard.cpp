#include "totype.hpp"
#include "colorf.hpp"
#include "sdldevice.hpp"
#include "menuboard.hpp"

extern SDLDevice *g_sdlDevice;

MenuBoard::MenuBoard(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argVarW,
        Widget::IntMargin argMargin,

        int argCorner,
        int argItemSpace,
        int argSeperatorSpace,

        std::initializer_list<std::tuple<Widget *, bool, bool>> argMenuItemList,
        std::function<void(Widget *)> argOnClickMenu,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .w = std::nullopt,
          .h = std::nullopt,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_itemSpace     (std::max<int>(0, argItemSpace     ))
    , m_separatorSpace(std::max<int>(0, argSeperatorSpace))

    , m_onClickMenu(std::move(argOnClickMenu))
    , m_canvas
      {{
          .fixed = Widget::transform(std::move(argVarW), [argMargin, this](int w)
          {
              return std::max<int>(0, w - argMargin.left - argMargin.right);
          }),
      }}

    , m_wrapper
      {{
          .wrapped{&m_canvas},
          .margin
          {
              .up    = argMargin[0],
              .down  = argMargin[1],
              .left  = argMargin[2],
              .right = argMargin[3],
          },

          .bgDrawFunc = [argCorner](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::BLACK_A255, dstDrawX, dstDrawY, self->w(), self->h(), std::max<int>(0, argCorner));
              g_sdlDevice->drawRectangle(colorf:: GREY_A255, dstDrawX, dstDrawY, self->w(), self->h(), std::max<int>(0, argCorner));
          },
          .parent{this},
      }}
{
    moveFront(&m_wrapper);
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
                    if(m_canvas.varW().has_value()){
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
                    return upperItemSpace(argWidget) + argWidget->h() + lowerItemSpace(argWidget) + (argAddSeparator ? m_separatorSpace : 0);
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
                        const int drawY      = drawDstY + upperItemSpace(argWidget) + argWidget->h() + m_separatorSpace / 2;

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
                },
            }
        },
    }}, true);
}
