#pragma once
#include <utility>
#include <initializer_list>
#include "widget.hpp"

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
            Widget::VarSize itemSpace = 0;

            std::initializer_list<std::pair<Widget *, bool>> childList {};
            Widget::WADPair parent {};
        };

    private:
        const bool m_vbox;

    private:
        Widget::VarSize m_itemSpace;

    public:
        ItemBox(ItemBox::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .w =  args.v ? std::move(args.fixed) : Widget::VarSize([this]{ return calcFlexSize(); }),
                  .h = !args.v ? std::move(args.fixed) : Widget::VarSize([this]{ return calcFlexSize(); }),

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
            Widget *firstWidget = nullptr;
            for(auto [widget, autoDelete]: args.childList){
                if(widget){
                    if(!firstWidget){
                        firstWidget = widget;
                    }
                    Widget::addChild(widget, autoDelete);
                }
            }

            if(firstWidget){
                updateOffsetFrom(firstWidget);
            }
        }

    public:
        void addChild(Widget *argWidget, bool argAutoDelete) override
        {
            if(argWidget){
                Widget::addChild(argWidget, argAutoDelete);
                updateOffsetFrom(argWidget);
            }
        }

        void removeChild(Widget *argWidget, bool argTriggerDelete) override
        {
            if(argWidget){
                auto nextWidget = nextChild(argWidget->id());
                Widget::removeChild(argWidget, argTriggerDelete);

                if(nextWidget){
                    updateOffsetFrom(nextWidget);
                }
            }
        }

    public:
        void flipChildShow(uint64_t childID)
        {
            if(auto child = hasChild(childID)){
                child->flipShow();
                updateOffsetFrom(child);
            }
        }

    public:
        void updateOffset()
        {
            if(auto firstWidget = firstChild()){
                updateOffsetFrom(firstWidget);
            }
        }

    private:
        void addChildAt(Widget *, WidgetTreeNode::VarDir, WidgetTreeNode::VarInt, WidgetTreeNode::VarInt, bool) override
        {
            throw fflerror("ItemBox::addChildAt");
        }

        void updateOffsetFrom(Widget *child)
        {
            fflassert(child);
            fflassert(hasChild(child->id()));

            bool found = false;
            Widget *lastShow = nullptr;

            foreachChild([itemSpace = Widget::evalSize(m_itemSpace, this), child, &found, &lastShow, this](Widget *w, bool)
            {
                if(w == child){
                    found = true;
                }

                if(found){
                    if(w->localShow()){
                        if(lastShow){
                            w->moveAt(DIR_UPLEFT, m_vbox ? 0 : (lastShow->dx() + lastShow->w() + itemSpace),
                                                 !m_vbox ? 0 : (lastShow->dy() + lastShow->h() + itemSpace));
                        }
                        else{
                            w->moveAt(DIR_UPLEFT, 0, 0);
                        }
                    }
                }

                if(w->localShow()){
                    lastShow = w;
                }
            });
        }

        int calcFlexSize() const
        {
            const Widget *lastShow = nullptr;
            foreachChild(false, [&lastShow](const Widget *w, bool) -> bool
            {
                if(w->localShow()){
                    lastShow = w;
                }
                return lastShow;
            });

            if(lastShow){
                if(m_vbox) return lastShow->dy() + lastShow->h();
                else       return lastShow->dx() + lastShow->w();
            }
            else{
                return 0;
            }
        }
};
