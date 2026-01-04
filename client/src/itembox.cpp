#include "itembox.hpp"
#include "margincontainer.hpp"

ItemBox::ItemBox(ItemBox::InitArgs args)
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
                              .name = "Canvas",
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
    m_canvas->setSize([this]{ return  m_vbox ? m_fixedEdgeSizeEval : m_flexibleEdgeSizeEval; },
                      [this]{ return !m_vbox ? m_fixedEdgeSizeEval : m_flexibleEdgeSizeEval; });

    for(auto [widget, autoDelete]: args.childList){
        if(widget){
            m_canvas->addChild(new MarginContainer
            {{
                .dir = [this]
                {
                    switch(m_align){
                        case ItemAlign::UPLEFT   : return          DIR_UPLEFT  ;
                        case ItemAlign::DOWNRIGHT: return m_vbox ? DIR_UPRIGHT : DIR_LEFTDOWN;
                        case ItemAlign::CENTER   : return m_vbox ? DIR_UP      : DIR_LEFT;
                        default: std::unreachable();
                    }
                }(),

                .contained
                {
                    .dir = DIR_LEFT,
                    .widget = widget,
                    .autoDelete = autoDelete,
                },
            }},

            true);
        }
    }

    buildLayout();
}

void ItemBox::addItem(Widget *argWidget, bool argAutoDelete)
{
    if(!argWidget){
        return;
    }

    const auto widgetW = argWidget->w();
    const auto widgetH = argWidget->h();

    const auto widgetFlexEdge  = m_vbox ? widgetH : widgetW;
    const auto widgetFixedEdge = m_vbox ? widgetW : widgetH;

    if(hasShowItem()){
        m_flexibleEdgeSizeEval += m_itemSpaceEval; // update to new widget start position
    }
    else{
        fflassert(m_flexibleEdgeSizeEval == 0);
    }

    if(!m_fixedEdgeSize.has_value() && (widgetFixedEdge > m_fixedEdgeSizeEval)){
        m_fixedEdgeSizeEval = widgetFixedEdge;
    }

    dir8_t d {};
    int    x {};
    int    y {};

    switch(m_align){
        case ItemAlign::UPLEFT:
            {
                d = DIR_UPLEFT;
                x =  m_vbox ? 0 : m_flexibleEdgeSizeEval;
                y = !m_vbox ? 0 : m_flexibleEdgeSizeEval;

                break;
            }
        case ItemAlign::DOWNRIGHT:
            {
                d =  m_vbox ? DIR_UPRIGHT : DIR_DOWNLEFT;
                x =  m_vbox ? m_fixedEdgeSizeEval - 1 : m_flexibleEdgeSizeEval;
                y = !m_vbox ? m_fixedEdgeSizeEval - 1 : m_flexibleEdgeSizeEval;

                break;
            }
        case ItemAlign::CENTER:
            {
                d =  m_vbox ? DIR_UP : DIR_LEFT;
                x =  m_vbox ? m_fixedEdgeSizeEval / 2 : m_flexibleEdgeSizeEval;
                y = !m_vbox ? m_fixedEdgeSizeEval / 2 : m_flexibleEdgeSizeEval;

                break;
            }
        default:
            {
                std::unreachable();
            }
    }

    m_flexibleEdgeSizeEval += widgetFlexEdge; // update to new widget end position
    m_canvas->addChild(new MarginContainer
    {{
        .dir = d,

        .x = x,
        .y = y,

        .w = widgetW,
        .h = widgetH,

        .contained
        {
            .dir = DIR_LEFT,
            .widget = argWidget,
            .autoDelete = argAutoDelete,
        },

        .attrs
        {
            .name = "ItemContainer",
        },
    }},

    true);
}

void ItemBox::removeItem(uint64_t argItemID, bool argTriggerAutoDelete)
{
    if(!argItemID){
        return;
    }

    MarginContainer *container = nullptr;
    m_canvas->foreachChild([argItemID, &container](Widget *child, bool) -> bool
    {
        if(auto mc = dynamic_cast<MarginContainer *>(child)){
            if(mc->contained()->id() == argItemID){
                container = mc;
            }
        }
        return container;
    });

    if(!container){
        return;
    }

    container->clearContained(argTriggerAutoDelete);
    doRemoveContainer(container);
}

bool ItemBox::hasShowItem() const
{
    return lastShowContainer();
}

void ItemBox::flipItemShow(uint64_t childID)
{
    if(auto child = m_canvas->hasDescendant(childID)){
        if(auto container = dynamic_cast<MarginContainer *>(child->parent())){

            bool needUpdateFixedEdgeSize   = false;
            bool needUpdateFixedEdgeOffset = false;

            if(container->localShow()){
                if(!m_fixedEdgeSize.has_value() && (m_vbox ? container->w() : container->h()) >= m_fixedEdgeSizeEval){ // actually cannot be greater
                    needUpdateFixedEdgeSize = true;
                    needUpdateFixedEdgeOffset = true;
                }
            }
            else{
                if(!m_fixedEdgeSize.has_value()){
                    if(const auto fixedEdge = m_vbox ? container->w() : container->h(); fixedEdge > m_fixedEdgeSizeEval){
                        m_fixedEdgeSizeEval = fixedEdge;
                        needUpdateFixedEdgeOffset = true;
                    }
                }
            }

            container->flipShow();
            if(needUpdateFixedEdgeSize){
                updateFixedEdgeSize();
            }

            if(needUpdateFixedEdgeOffset && (m_align != ItemAlign::UPLEFT)){
                updateFixedEdgeOffset();
            }

            updateFlexEdgeOffset(container);
        }
    }
}

void ItemBox::buildLayout()
{
    m_itemSpaceEval = Widget::evalSize(m_itemSpace, this);
    if(const auto firstShow = firstShowContainer()){
        updateMarginContainers();

        updateFlexEdgeOffset(firstShow);
        updateFlexEdgeSize(); // side determined by offset

        updateFixedEdgeSize();
        updateFixedEdgeOffset(); // offset determined by size
    }
    else{
        if(m_fixedEdgeSize.has_value()){
            m_fixedEdgeSizeEval = Widget::evalSize(m_fixedEdgeSize.value(), this);
        }
        else{
            m_fixedEdgeSizeEval = 0;
        }
        m_flexibleEdgeSizeEval = 0;
    }
}

void ItemBox::updateMarginContainers()
{
    m_canvas->foreachChild([](Widget *child, bool)
    {
        if(child->localShow()){
            if(auto mc = dynamic_cast<MarginContainer *>(child)){
                mc->setSize(mc->contained()->w(), mc->contained()->h());
            }
        }
    });
}

void ItemBox::updateFlexEdgeSize()
{
    if(const auto lastShow = lastShowContainer()){
        if(m_vbox) m_flexibleEdgeSizeEval = lastShow->dy() + lastShow->h();
        else       m_flexibleEdgeSizeEval = lastShow->dx() + lastShow->w();
    }
    else{
        m_flexibleEdgeSizeEval = 0;
    }
}

void ItemBox::updateFixedEdgeSize()
{
    // always re-evaluate fixed edge size
    // if m_fixedEdgeSize has value, don't call this function except in buildLayout(), which re-evaluates everything

    if(m_fixedEdgeSize.has_value()){
        m_fixedEdgeSizeEval = Widget::evalSize(m_fixedEdgeSize.value(), this);
    }
    else{
        m_fixedEdgeSizeEval = 0;
        m_canvas->foreachChild([this](const Widget *child, bool)
        {
            if(child->localShow()){
                m_fixedEdgeSizeEval = std::max<int>(m_fixedEdgeSizeEval, m_vbox ? child->w() : child->h());
            }
        });
    }
}

void ItemBox::updateFlexEdgeOffset(const Widget *container)
{
    fflassert(container);
    fflassert(m_canvas->hasChild(container->id()));

    bool found = false;
    Widget *lastShow = nullptr;

    m_canvas->foreachChild([container, &found, &lastShow, this](Widget *child, bool)
    {
        if(child == container){
            found = true;
        }

        if(found){
            if(child->localShow()){
                if(m_vbox) child->moveYTo(lastShow ? (lastShow->dy() + lastShow->h() + m_itemSpaceEval) : 0);
                else       child->moveXTo(lastShow ? (lastShow->dx() + lastShow->w() + m_itemSpaceEval) : 0);
            }
        }

        if(child->localShow()){
            lastShow = child;
        }
    });
}

void ItemBox::updateFixedEdgeOffset()
{
    m_canvas->foreachChild([this](Widget *child, bool)
    {
        switch(m_align){
            case ItemAlign::UPLEFT:
                {
                    if(m_vbox) child->moveXTo(0);
                    else       child->moveYTo(0);

                    break;
                }
            case ItemAlign::DOWNRIGHT:
                {
                    if(m_vbox) child->moveXTo(m_fixedEdgeSizeEval - 1);
                    else       child->moveYTo(m_fixedEdgeSizeEval - 1);

                    break;
                }
            case ItemAlign::CENTER:
                {
                    if(m_vbox) child->moveXTo(m_fixedEdgeSizeEval / 2);
                    else       child->moveYTo(m_fixedEdgeSizeEval / 2);

                    break;
                }
            default:
                {
                    std::unreachable();
                }
        }
    });
}

const Widget *ItemBox::findShowContainer(bool forward) const
{
    const Widget *container = nullptr;
    m_canvas->foreachChild(forward, [&container](const Widget *child, bool) -> bool
    {
        if(child->localShow()){
            container = child;
        }
        return container;
    });
    return container;
}

void ItemBox::doRemoveContainer(const MarginContainer *container)
{
    if(!container){
        return;
    }

    fflassert(!container->contained());
    fflassert(m_canvas->hasChild(container->id()));

    const auto widgetShow = container->localShow();
    const auto nextContainer = m_canvas->nextChild(container->id());

    const auto widgetW = widgetShow ? container->w() : 0;
    const auto widgetH = widgetShow ? container->h() : 0;

    const auto widgetFlexEdge  = m_vbox ? widgetH : widgetW;
    const auto widgetFixedEdge = m_vbox ? widgetW : widgetH;

    m_canvas->removeChild(container->id(), true);

    if(!widgetShow){
        return;
    }

    if(hasShowItem()){
        m_flexibleEdgeSizeEval -= widgetFlexEdge;
        m_flexibleEdgeSizeEval -= m_itemSpaceEval;

        if(nextContainer){
            updateFlexEdgeOffset(nextContainer);
        }

        if(!m_fixedEdgeSize.has_value() && (widgetFixedEdge >= m_fixedEdgeSizeEval)){
            updateFixedEdgeSize();
            updateFixedEdgeOffset();
        }
    }
    else{
        if(!m_fixedEdgeSize.has_value()){
            m_fixedEdgeSizeEval = 0;
        }
        m_flexibleEdgeSizeEval = 0;
    }
}
