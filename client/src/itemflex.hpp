#pragma once
#include <vector>
#include <utility>
#include <initializer_list>
#include "widget.hpp"

class ItemFlex: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt fixed = std::nullopt; // the other side to flexible edge

            bool v = true;
            Widget::VarSize itemSpace = 0;

            std::initializer_list<std::pair<Widget *, bool>> childList {};
            Widget::WADPair parent {};
        };

    private:
        const bool m_vbox;

    private:
        Widget::VarSize m_itemSpace;

    public:
        ItemFlex(ItemFlex::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .w =  args.v ? std::move(args.fixed) : Widget::VarSizeOpt{},
                  .h = !args.v ? std::move(args.fixed) : Widget::VarSizeOpt{},

                  .attrs
                  {
                      .inst
                      {
                          .moveOnFocus = false,
                      },
                  },
                  .parent = std::move(args.parent),
              }}

            , m_vbox(args.v)
            , m_itemSpace(std::move(args.itemSpace))
        {
            for(auto [widget, autoDelete]: args.childList){
                addChild(widget, autoDelete);
            }
        }

    public:
        void addChild(Widget *argWidget, bool argAutoDelete) override
        {
            if(!argWidget){
                return;
            }

            const auto fnGetOffset = [argWidget, this]
            {
                int offset = 0;
                int itemSpace = Widget::evalSize(m_itemSpace, this);

                foreachChild([argWidget, &offset, itemSpace, this](const Widget *child, bool)
                {
                    if(child == argWidget){
                        return true;
                    }

                    if(!child->show()){
                        return false;
                    }

                    offset += (m_vbox ? child->h() : child->w());
                    offset += itemSpace;

                    return false;
                });

                return offset;
            };

            Widget::addChildAt(argWidget, DIR_UPLEFT, m_vbox ? Widget::VarInt(0) : fnGetOffset,
                                                     !m_vbox ? Widget::VarInt(0) : fnGetOffset, argAutoDelete);
        }

    private:
        void addChildAt(Widget *, WidgetTreeNode::VarDir, WidgetTreeNode::VarInt, WidgetTreeNode::VarInt, bool) override
        {
            throw fflerror("ItemFlex::addChildAt");
        }
};
