#include "itemflex.hpp"

ItemFlex::ItemFlex(ItemFlex::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .childList
          {
              {
                  .widget = new Widget
                  {{
                      .attrs
                      {
                          .inst
                          {
                              .moveOnFocus = false,
                          },
                      },
                  }},
                  .autoDelete = true,
              },
          },

          .attrs
          {
              .type
              {
                  .setSize = false,
                  .addChild = false,
                  .removeChild = false,
              },
          },

          .parent = std::move(args.parent),
      }}

    , m_vbox(args.v)
    , m_align(args.align)

    , m_canvas(firstChild())
    , m_itemSpace(std::move(args.itemSpace))
    , m_fixedEdgeSize(std::move(args.fixed))
{
    m_canvas->setSize([this]{ return canvasW(); },
                      [this]{ return canvasH(); });

    for(auto [widget, autoDelete]: args.childList){
        addItem(widget, autoDelete);
    }
}

void ItemFlex::addItem(Widget *argWidget, bool argAutoDelete)
{
    if(!argWidget){
        return;
    }

    const auto fnGetOffset = [argWidget, this]
    {
        int offset = 0;
        int itemSpace = Widget::evalSize(m_itemSpace, this);

        m_canvas->foreachChild([argWidget, &offset, itemSpace, this](const Widget *child, bool)
        {
            if(child == argWidget){
                return true;
            }

            if(!child->localShow()){
                return false;
            }

            offset += (m_vbox ? child->h() : child->w());
            offset += itemSpace;

            return false;
        });

        return offset;
    };

    Widget::VarDir d {};
    Widget::VarInt x {};
    Widget::VarInt y {};

    switch(m_align){
        case ItemAlign::UPLEFT:
            {
                d = DIR_UPLEFT;
                x =  m_vbox ? Widget::VarInt(0) : fnGetOffset;
                y = !m_vbox ? Widget::VarInt(0) : fnGetOffset;

                break;
            }
        case ItemAlign::DOWNRIGHT:
            {
                d =  m_vbox ? DIR_UPRIGHT : DIR_DOWNLEFT;
                x =  m_vbox ? Widget::VarInt{[this]{ return m_canvas->w() - 1; }} : fnGetOffset;
                y = !m_vbox ? Widget::VarInt{[this]{ return m_canvas->h() - 1; }} : fnGetOffset;

                break;
            }
        case ItemAlign::CENTER:
            {
                d =  m_vbox ? DIR_UP : DIR_LEFT;
                x =  m_vbox ? Widget::VarInt{[this]{ return m_canvas->w() / 2; }} : fnGetOffset;
                y = !m_vbox ? Widget::VarInt{[this]{ return m_canvas->h() / 2; }} : fnGetOffset;

                break;
            }
        default:
            {
                std::unreachable();
            }
    }

    m_canvas->addChildAt(argWidget, std::move(d), std::move(x), std::move(y), argAutoDelete);
}

void ItemFlex::removeItem(uint64_t argChildID, bool argTriggerAutoDelete)
{
    if(!argChildID){
        return;
    }

    m_canvas->removeChild(argChildID, argTriggerAutoDelete);
}

bool ItemFlex::hasShowItem() const
{
    return m_canvas->foreachChild([](const Widget *child, bool)
    {
        return child->localShow();
    });
}

void ItemFlex::flipItemShow(uint64_t childID)
{
    if(auto child = m_canvas->hasChild(childID)){
        child->flipShow();
    }
}

void ItemFlex::buildLayout()
{
    // empty function
}

int ItemFlex::canvasW() const
{
    if(m_vbox){
        return Widget::evalSizeOpt(m_fixedEdgeSize, this, [this]
        {
            int maxW = 0;
            m_canvas->foreachChild([&maxW](const Widget *child, bool)
            {
                if(child->localShow()){
                    maxW = std::max<int>(maxW, child->w());
                }
            });
            return maxW;
        });
    }
    else{
        if(const auto lastWidget = lastShowChild()){
            return lastWidget->dx() + lastWidget->w();
        }
        else{
            return 0;
        }
    }
}

int ItemFlex::canvasH() const
{
    if(!m_vbox){
        return Widget::evalSizeOpt(m_fixedEdgeSize, this, [this]
        {
            int maxH = 0;
            m_canvas->foreachChild([&maxH](const Widget *child, bool)
            {
                if(child->localShow()){
                    maxH = std::max<int>(maxH, child->h());
                }
            });
            return maxH;
        });
    }
    else{
        if(const auto lastWidget = lastShowChild()){
            return lastWidget->dy() + lastWidget->h();
        }
        else{
            return 0;
        }
    }
}

const Widget *ItemFlex::lastShowChild() const
{
    const Widget *lastWidget = nullptr;
    m_canvas->foreachChild(false, [&lastWidget](const Widget *child, bool) -> bool
    {
        if(child->localShow()){
            lastWidget = child;
        }
        return lastWidget;
    });
    return lastWidget;
}
