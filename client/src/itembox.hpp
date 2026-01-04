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

    private:
        void doRemoveContainer(const MarginContainer *);
};

void ItemBox::clearItem(std::invocable<const Widget *, bool> auto func)
{
    m_canvas->foreachChild([func, this](auto container, bool)
    {
        if(auto mc = dynamic_cast<MarginContainer *>(container)){
            if(const auto [item, autoDelete] = mc->containedPair(); func(item, autoDelete)){
                mc->clearContained(true);
            }
        }
    });

    // update offset
    // doRemoveContainer does offset update

    m_canvas->foreachChild([this](auto container, bool)
    {
        if(auto mc = dynamic_cast<MarginContainer *>(container); !mc->contained()){
            doRemoveContainer(mc);
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
        const auto [item, autoDelete] = dynamic_cast<MarginContainer *>(container)->containedPair();
        return func(item, autoDelete);
    });
}

auto ItemBox::foreachItem(this auto && self, auto func)
{
    return self.foreachItem(true, func);
}
