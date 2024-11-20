#pragma once
#include <vector>
#include <initializer_list>
#include "widget.hpp"

class ItemFlex: public Widget
{
    private:
        const bool m_hbox;

    private:
        Widget::VarOff m_itemSpace;
        std::vector<Widget *> m_origChildList;

    public:
        ItemFlex(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget::VarSize argVarSize,

                bool argHBox,
                Widget::VarOff argItemSpace = 0,

                std::initializer_list<std::pair<Widget *, bool>> argChildList = {},

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),

                  /**/  argHBox ? Widget::VarSize{} : std::move(argVarSize),
                  /**/ !argHBox ? Widget::VarSize{} : std::move(argVarSize),

                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_hbox(argHBox)
            , m_itemSpace(std::move(argItemSpace))
        {
            for(auto [widget, autoDelete]: argChildList){
                addChild(widget, autoDelete);
            }
        }

    public:
        void addChild(Widget *argWidget, bool argAutoDelete) override
        {
            m_origChildList.push_back(argWidget);
            if(m_hbox){
                addChildAt(argWidget, DIR_UPLEFT, [this](const Widget *self)
                {
                    const int itemSpace = std::max<int>(0, Widget::evalOff(m_itemSpace, this));
                    int offset = 0;

                    for(auto widget: m_origChildList){
                        if(widget == self){
                            break;
                        }

                        offset += widget->w();
                        offset += itemSpace;
                    }

                    return offset;
                },

                0,
                argAutoDelete);
            }
            else{
                addChildAt(argWidget, DIR_UPLEFT, 0, [this](const Widget *self)
                {
                    const int itemSpace = std::max<int>(0, Widget::evalOff(m_itemSpace, this));
                    int offset = 0;

                    for(auto widget: m_origChildList){
                        if(widget == self){
                            break;
                        }

                        offset += widget->h();
                        offset += itemSpace;
                    }

                    return offset;
                },

                argAutoDelete);
            }
        }

    public:
        void removeChildElement(Widget::ChildElement &argChild, bool argTrigger) override
        {
            std::erase(m_origChildList, argChild.widget);
            Widget::removeChildElement(argChild, argTrigger);
        }
};
