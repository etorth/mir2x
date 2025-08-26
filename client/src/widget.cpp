#include <atomic>
#include "widget.hpp"

WidgetTreeNode::WidgetTreeNode(Widget *argParent, bool argAutoDelete)
    : m_id([]
      {
          static std::atomic<uint64_t> s_widgetSeqID = 1;
          return s_widgetSeqID.fetch_add(1);
      }())
    , m_parent(argParent)
{
    if(m_parent){
        m_parent->addChild(static_cast<Widget *>(this), argAutoDelete);
    }
}

WidgetTreeNode::~WidgetTreeNode()
{
    clearChild();
    for(auto widget: m_delayList){
        delete widget;
    }
}

Widget * WidgetTreeNode::parent(unsigned level)
{
    auto widptr = this;
    while(widptr && (level > 0)){
        widptr = widptr->m_parent;
        level--;
    }
    return static_cast<Widget *>(widptr);
}

const Widget * WidgetTreeNode::parent(unsigned level) const
{
    auto widptr = this;
    while(widptr && (level > 0)){
        widptr = widptr->m_parent;
        level--;
    }
    return static_cast<const Widget *>(widptr);
}

void WidgetTreeNode::moveFront(const Widget *widget)
{
    if(m_inLoop){
        throw fflerror("can not modify child list while in loop");
    }

    auto pivot = std::find_if(m_childList.begin(), m_childList.end(), [widget](const auto &x)
    {
        return x.widget == widget;
    });

    if(pivot == m_childList.end()){
        throw fflerror("can not find child widget");
    }

    std::rotate(m_childList.begin(), pivot, std::next(pivot));
}

void WidgetTreeNode::moveBack(const Widget *widget)
{
    if(m_inLoop){
        throw fflerror("can not modify child list while in loop");
    }

    auto pivot = std::find_if(m_childList.begin(), m_childList.end(), [widget](const auto &x)
    {
        return x.widget == widget;
    });

    if(pivot == m_childList.end()){
        throw fflerror("can not find child widget");
    }

    std::rotate(pivot, std::next(pivot), m_childList.end());
}

void WidgetTreeNode::execDeath() noexcept
{
    for(auto &child: m_childList){
        if(child.widget){
            child.widget->execDeath();
        }
    }
    onDeath();
}

void WidgetTreeNode::doAddChild(Widget *argWidget, bool argAutoDelete)
{
    fflassert(argWidget);
    WidgetTreeNode *treeNode = argWidget;

    if(treeNode->m_parent){
        treeNode->m_parent->removeChild(argWidget, false);
    }

    treeNode->m_parent = static_cast<Widget *>(this);
    m_childList.emplace_back(argWidget, argAutoDelete);
}

void WidgetTreeNode::addChild(Widget *argWidget, bool argAutoDelete)
{
    doAddChild(argWidget, argAutoDelete);
}

void WidgetTreeNode::addChildAt(Widget *argWidget, WidgetTreeNode::VarDir argDir, WidgetTreeNode::VarOff argX, WidgetTreeNode::VarOff argY, bool argAutoDelete)
{
    doAddChild(argWidget, argAutoDelete);
    argWidget->moveAt(std::move(argDir), std::move(argX), std::move(argY));
}

void WidgetTreeNode::removeChild(Widget *argWidget, bool argTriggerDelete)
{
    if(argWidget){
        for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
            if(p->widget == argWidget){
                removeChildElement(*p, argTriggerDelete);
                return;
            }
        }
    }
}

void WidgetTreeNode::removeChildElement(WidgetTreeNode::ChildElement &argElement, bool argTriggerDelete)
{
    if(auto widptr = argElement.widget){
        widptr->m_parent = nullptr;
        argElement.widget = nullptr;

        if(argElement.autoDelete && argTriggerDelete){
            widptr->execDeath();
            m_delayList.push_back(widptr);
        }
    }
}

void WidgetTreeNode::purge()
{
    if(m_inLoop){
        throw fflerror("can not modify child list while in loop");
    }

    foreachChild([](Widget *widget, bool)
    {
        widget->purge();
    });

    for(auto widget: m_delayList){
        delete widget;
    }

    m_delayList.clear();
    m_childList.remove_if([](const auto &x) -> bool
    {
        return x.widget == nullptr;
    });
}

bool WidgetTreeNode::hasChild() const
{
    return firstChild();
}

Widget *WidgetTreeNode::hasChild(uint64_t argID)
{
    for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
        if(p->widget && p->widget->id() == argID){
            return p->widget;
        }
    }
    return nullptr;
}

const Widget *WidgetTreeNode::hasChild(uint64_t argID) const
{
    for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
        if(p->widget && p->widget->id() == argID){
            return p->widget;
        }
    }
    return nullptr;
}

Widget *WidgetTreeNode::hasDescendant(uint64_t argID)
{
    for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
        if(p->widget){
            if(p->widget->id() == argID){
                return p->widget;
            }
            else if(auto descendant = p->widget->hasDescendant(argID)){
                return descendant;
            }
        }
    }
    return nullptr;
}

const Widget *WidgetTreeNode::hasDescendant(uint64_t argID) const
{
    for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
        if(p->widget){
            if(p->widget->id() == argID){
                return p->widget;
            }
            else if(auto descendant = p->widget->hasDescendant(argID)){
                return descendant;
            }
        }
    }
    return nullptr;
}

bool Widget::processEventDefault(const SDL_Event &event, bool valid)
{
    if(!show()){
        return false;
    }

    bool took = false;
    uint64_t focusedWidgetID = 0;

    foreachChild(false, [&event, valid, &took, &focusedWidgetID, this](Widget *widget, bool)
    {
        if(widget->show()){
            const bool validEvent = valid && !took;
            const bool takenEvent = widget->processEvent(event, validEvent);

            if(!validEvent && takenEvent){
                throw fflerror("widget %s takes invalid event", widget->name());
            }

            // it's possible that a widget takes event but doesn't get focus
            // i.e. press a button to pop up a modal window, but still abort here for easier maintenance

            if(widget->focus()){
                if(focusedWidgetID){
                    if(auto focusedWidget = hasChild(focusedWidgetID); focusedWidget && focusedWidget->focus()){
                        // a widget with focus can drop events
                        // i.e. a focused slider ignores mouse motion if button released
                        focusedWidget->setFocus(false);
                    }
                }
                focusedWidgetID = widget->id();
            }

            took |= takenEvent;
        }
    });

    if(auto widget = hasChild(focusedWidgetID)){
        moveBack(widget);
    }

    return took;
}

void Widget::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    foreachChild([srcX, srcY, dstX, dstY, srcW, srcH, this](const Widget *widget, bool)
    {
        if(widget->show()){
            int srcXCrop = srcX;
            int srcYCrop = srcY;
            int dstXCrop = dstX;
            int dstYCrop = dstY;
            int srcWCrop = srcW;
            int srcHCrop = srcH;

            if(mathf::cropChildROI(
                        &srcXCrop, &srcYCrop,
                        &srcWCrop, &srcHCrop,
                        &dstXCrop, &dstYCrop,

                        w(),
                        h(),

                        widget->dx(),
                        widget->dy(),
                        widget-> w(),
                        widget-> h())){
                widget->drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
            }
        }
    });
}

void Widget::afterResizeDefault()
{
    foreachChild([](Widget *child, bool){ child->afterResize(); });
}
