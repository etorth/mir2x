#pragma once
#include <utility>
#include <concepts>
#include <initializer_list>
#include "widget.hpp"
#include "itemalign.hpp"
#include "margincontainer.hpp"

class ItemBox: public Widget
{
    #include "itemflex.api.hpp"

    private:
        const bool m_vbox;
        const ItemAlign m_align;

    private:
        Widget *m_canvas;

    private:
        Widget::VarSize m_itemSpace;
        Widget::VarSizeOpt m_fixedEdgeSize;

    private:
        int m_itemSpaceEval = 0;
        int m_fixedEdgeSizeEval = 0;
        int m_flexibleEdgeSizeEval = 0;

    public:
        ItemBox(ItemBox::InitArgs);

    private:
        void updateMarginContainers();

        void updateFlexEdgeSize();
        void updateFixedEdgeSize();

        void updateFlexEdgeOffset(const Widget *);
        void updateFixedEdgeOffset();

    private:
        const Widget *firstShowContainer() const { return findShowContainer(true ); }
        const Widget * lastShowContainer() const { return findShowContainer(false); }

    private:
        const Widget *findShowContainer(bool foward) const;
};

void ItemBox::clearItem(std::invocable<const Widget *, bool> auto func)
{
    m_canvas->foreachChild([func, this](auto container, bool)
    {
        const bool match = container->foreachChild([func](auto item, bool autoDelete)
        {
            return func(item, autoDelete);
        });

        if(match){
            dynamic_cast<MarginContainer *>(container)->clearContained();
            m_canvas->removeChild(container->id(), true);
        }
    });
}

void ItemBox::clearItem()
{
    clearItem([](const Widget *, bool){ return true; });
}

auto ItemBox::foreachItem(this auto && self, bool forward, auto func)
{
    return self.m_canvas->foreachChild(forward, [func](auto container, bool)
    {
        return container->foreachChild([func](auto item, bool autoDelete)
        {
            return func(item, autoDelete);
        });
    });
}

auto ItemBox::foreachItem(this auto && self, auto func)
{
    return self.foreachItem(true, func);
}
