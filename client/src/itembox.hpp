#pragma once
#include <vector>
#include <initializer_list>
#include "widget.hpp"

class ItemBox: public Widget
{
    private:
        const bool m_hbox;
        std::vector<Widget *> m_origChildList;

    public:
        ItemBox(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget::VarSize argVarSize,
                bool argHBox,

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
        {
            for(auto [widget, autoDelete]: argChildList){
                appendItem(widget, autoDelete);
            }
        }

    public:
        void appendItem(Widget *argWidget, bool argAutoDelete)
        {
            m_origChildList.push_back(argWidget);
            if(m_hbox){
                addChild(argWidget, DIR_UPLEFT, [this](const Widget *self)
                {
                    int offset = 0;
                    for(auto widget: m_origChildList){
                        if(widget == self){
                            break;
                        }
                        offset += widget->w();
                    }
                    return offset;
                },

                0,
                argAutoDelete);
            }
            else{
                addChild(argWidget, DIR_UPLEFT, 0, [this](const Widget *self)
                {
                    int offset = 0;
                    for(auto widget: m_origChildList){
                        if(widget == self){
                            break;
                        }
                        offset += widget->h();
                    }
                    return offset;
                },

                argAutoDelete);
            }
        }
};
