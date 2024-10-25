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

void WidgetTreeNode::addChild(Widget *widget, bool autoDelete)
{
    fflassert(widget);
    WidgetTreeNode *treeNode = widget;

    if(treeNode->m_parent){
        treeNode->m_parent->removeChild(widget, false);
    }

    treeNode->m_parent = static_cast<Widget *>(this);
    m_childList.emplace_back(widget, autoDelete);
}

void WidgetTreeNode::addChild(Widget *argWidget, WidgetVarDir argDir, WidgetVarOffset argX, WidgetVarOffset argY, bool argAutoDelete)
{
    fflassert(argWidget);
    addChild(argWidget, argAutoDelete);
    argWidget->moveAt(std::move(argDir), std::move(argX), std::move(argY));
}

void WidgetTreeNode::removeChild(Widget *widget, bool triggerDelete)
{
    for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
        if(WidgetTreeNode * treeNode = p->widget; p->widget == widget){
            p->widget = nullptr;
            treeNode->m_parent = nullptr;

            if(triggerDelete && p->autoDelete){
                widget->execDeath();
                m_delayList.push_back(widget);
            }
            return;
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
