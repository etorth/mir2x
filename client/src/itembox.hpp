#pragma once
#include <utility>
#include <concepts>
#include <initializer_list>
#include "widget.hpp"
#include "itemalign.hpp"
#include "margincontainer.hpp"

class ItemBox: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt fixed = std::nullopt; // the other side to flexible edge

            bool v = true;
            ItemAlign align = ItemAlign::UPLEFT;

            Widget::VarSize itemSpace = 0;

            std::initializer_list<std::pair<Widget *, bool>> childList {};
            Widget::WADPair parent {};
        };

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

    public:
        void    addItem(Widget *, bool);
        void removeItem(uint64_t, bool);

    public:
        bool hasShowItem() const;
        void flipItemShow(uint64_t);

    public:
        void buildLayout(); // recalculate everything

    public:
        void clearItem(std::invocable<const Widget *, bool> auto func)
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

        void clearItem()
        {
            clearItem([](const Widget *, bool){ return true; });
        }

    public:
        auto foreachItem(this auto && self, bool forward, auto func)
        {
            return self.m_canvas->foreachChild(forward, [func](auto container, bool)
            {
                return container->foreachChild([func](auto item, bool autoDelete)
                {
                    return func(item, autoDelete);
                });
            });
        }

        auto foreachItem(this auto && self, auto func)
        {
            return self.foreachItem(true, func);
        }

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
