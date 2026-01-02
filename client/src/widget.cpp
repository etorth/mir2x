#include <atomic>
#include <fstream>
#include "utf8f.hpp"
#include "pathf.hpp"
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
    // if construct a widget failed
    // destructor will be called, we need to automatically remove it from parent

    // otherwise later parent destructor will try to call all its children's destructor again
    // which causes double free

    if(m_parent){
        m_parent->removeChild(static_cast<Widget *>(this), false);
    }

    clearChild();
    for(auto widget: m_delayList){
        delete widget;
    }
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

void WidgetTreeNode::doAddChild(Widget *argWidget, bool argAutoDelete, bool ignoreCanAddChild)
{
    fflassert(argWidget);
    WidgetTreeNode *treeNode = argWidget;

    if(ignoreCanAddChild || static_cast<Widget *>(this)->m_attrs.type.canAddChild){
        if(treeNode->m_parent){
            treeNode->m_parent->removeChild(argWidget, false);
        }

        treeNode->m_parent = static_cast<Widget *>(this);
        m_childList.emplace_back(argWidget, argAutoDelete); // only place to add child to m_childList
    }
    else{
        throw fflerror("widget %s forbids to add child", name());
    }
}

void WidgetTreeNode::addChild(Widget *argWidget, bool argAutoDelete)
{
    doAddChild(argWidget, argAutoDelete, false);
}

void WidgetTreeNode::addChildAt(Widget *argWidget, WidgetTreeNode::VarDir argDir, WidgetTreeNode::VarInt argX, WidgetTreeNode::VarInt argY, bool argAutoDelete)
{
    doAddChild(argWidget, argAutoDelete, false);
    argWidget->moveAt(std::move(argDir), std::move(argX), std::move(argY));
}

void WidgetTreeNode::removeChild(Widget *argWidget, bool argTriggerDelete)
{
    if(argWidget){
        for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
            if(p->widget == argWidget){
                doRemoveChild(*p, argTriggerDelete);
                return;
            }
        }
    }
}

void WidgetTreeNode::doRemoveChild(WidgetTreeNode::ChildElement &argElement, bool argTriggerDelete)
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

dir8_t Widget::evalDir(const Widget::VarDir &varDir, const Widget *widget, const void *arg)
{
    const auto fnValidDir = [](dir8_t argDir)
    {
        return pathf::dirValid(argDir) ? argDir : DIR_NONE;
    };

    return std::visit(VarDispatcher
    {
        [&fnValidDir](dir8_t varg)
        {
            return fnValidDir(varg);
        },

        [&fnValidDir](const std::function<dir8_t()> &varg)
        {
            return varg ? fnValidDir(varg()) : DIR_NONE;
        },

        [&fnValidDir, widget](const std::function<dir8_t(const Widget *)> &varg)
        {
            return varg ? fnValidDir(varg(widget)) : DIR_NONE;
        },

        [&fnValidDir, widget, arg](const std::function<dir8_t(const Widget *, const void *)> &varg)
        {
            return varg ? fnValidDir(varg(widget, arg)) : DIR_NONE;
        },
    },

    varDir);
}

int Widget::evalInt(const Widget::VarInt &varOffset, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](int varg)
        {
            return varg;
        },

        [](const std::function<int()> &varg)
        {
            return varg ? varg() : 0;
        },

        [widget](const std::function<int(const Widget *)> &varg)
        {
            return varg ? varg(widget) : 0;
        },

        [widget, arg](const std::function<int(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : 0;
        },
    },

    varOffset);
}

uint32_t Widget::evalU32(const Widget::VarU32 &varU32, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](uint32_t varg)
        {
            return varg;
        },

        [](const std::function<uint32_t()> &varg)
        {
            return varg ? varg() : 0;
        },

        [widget](const std::function<uint32_t(const Widget *)> &varg)
        {
            return varg ? varg(widget) : 0;
        },

        [widget, arg](const std::function<uint32_t(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : 0;
        },
    },

    varU32);
}

float Widget::evalDecimal(const Widget::VarDecimal &varDecimal, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](float varg)
        {
            return varg;
        },

        [](const std::function<float()> &varg)
        {
            return varg ? varg() : 0.0f;
        },

        [widget](const std::function<float(const Widget *)> &varg)
        {
            return varg ? varg(widget) : 0.0f;
        },

        [widget, arg](const std::function<float(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : 0.0f;
        },
    },

    varDecimal);
}

int Widget::evalSize(const Widget::VarSize &varSize, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](int varg)
        {
            return std::max<int>(0, varg);
        },

        [](const std::function<int()> &varg)
        {
            return varg ? std::max<int>(0, varg()) : 0;
        },

        [widget](const std::function<int(const Widget *)> &varg)
        {
            return varg ? std::max<int>(0, varg(widget)) : 0;
        },

        [widget, arg](const std::function<int(const Widget *, const void *)> &varg)
        {
            return varg ? std::max<int>(0, varg(widget, arg)) : 0;
        },
    },

    varSize);
}

bool Widget::evalBool(const Widget::VarBool &varFlag, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](bool varg)
        {
            return varg;
        },

        [](const std::function<bool()> &varg)
        {
            return varg ? varg() : false;
        },

        [widget](const std::function<bool(const Widget *)> &varg)
        {
            return varg ? varg(widget) : false;
        },

        [widget, arg](const std::function<bool(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : false;
        },
    },

    varFlag);
}

SDL_BlendMode Widget::evalBlendMode(const Widget::VarBlendMode &varBlendMode, const Widget *widget, const void *arg)
{
    const auto fnValidMode = [](SDL_BlendMode argMode)
    {
        switch(argMode){
            case SDL_BLENDMODE_ADD:
            case SDL_BLENDMODE_MOD:
            case SDL_BLENDMODE_MUL:
            case SDL_BLENDMODE_BLEND: return argMode;
            default                 : return SDL_BLENDMODE_NONE;
        }
    };

    return std::visit(VarDispatcher
    {
        [&fnValidMode](SDL_BlendMode varg)
        {
            return fnValidMode(varg);
        },

        [&fnValidMode](const std::function<SDL_BlendMode()> &varg)
        {
            return varg ? fnValidMode(varg()) : SDL_BLENDMODE_NONE;
        },

        [&fnValidMode, widget](const std::function<SDL_BlendMode(const Widget *)> &varg)
        {
            return varg ? fnValidMode(varg(widget)) : SDL_BLENDMODE_NONE;
        },

        [&fnValidMode, widget, arg](const std::function<SDL_BlendMode(const Widget *, const void *)> &varg)
        {
            return varg ? fnValidMode(varg(widget, arg)) : SDL_BLENDMODE_NONE;
        },
    },

    varBlendMode);
}

std::string Widget::evalStrFunc(const Widget::VarStrFunc &varStrFunc, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](const std::string &varg)
        {
            return varg;
        },

        [](const std::function<std::string()> &varg)
        {
            return varg ? varg() : std::string();
        },

        [widget](const std::function<std::string(const Widget *)> &varg)
        {
            return varg ? varg(widget) : std::string();
        },

        [widget, arg](const std::function<std::string(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : std::string();
        },
    },

    varStrFunc);
}

SDL_Texture *Widget::evalTexLoadFunc(const Widget::VarTexLoadFunc &varTexLoadFunc, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](SDL_Texture *varg)
        {
            return varg;
        },

        [](const std::function<SDL_Texture *()> &varg)
        {
            return varg ? varg() : nullptr;
        },

        [widget](const std::function<SDL_Texture *(const Widget *)> &varg)
        {
            return varg ? varg(widget) : nullptr;
        },

        [widget, arg](const std::function<SDL_Texture *(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : nullptr;
        },
    },

    varTexLoadFunc);
}

bool Widget::hasDrawFunc(const Widget::VarDrawFunc &varDrawFunc)
{
    return std::visit(VarDispatcher
    {
        [](const std::function<void(                        int, int)> &varg) -> bool { return !!varg; },
        [](const std::function<void(const Widget *,         int, int)> &varg) -> bool { return !!varg; },
        [](const std::function<void(const Widget *, void *, int, int)> &varg) -> bool { return !!varg; },

        [](std::nullptr_t){ return false; },
    },

    varDrawFunc);
}

void Widget::execDrawFunc(const Widget::VarDrawFunc &varDrawFunc, const Widget *widget, int argX, int argY)
{
    Widget::execDrawFunc(varDrawFunc, widget, nullptr, argX, argY);
}

void Widget::execDrawFunc(const Widget::VarDrawFunc &varDrawFunc, const Widget *widget, void *argPtr, int argX, int argY)
{
    std::visit(VarDispatcher
    {
        [                argX, argY](const std::function<void(                        int, int)> &varg) { if(varg){ varg(                argX, argY); }},
        [widget,         argX, argY](const std::function<void(const Widget *,         int, int)> &varg) { if(varg){ varg(widget,         argX, argY); }},
        [widget, argPtr, argX, argY](const std::function<void(const Widget *, void *, int, int)> &varg) { if(varg){ varg(widget, argPtr, argX, argY); }},

        [](std::nullptr_t){},
    },

    varDrawFunc);
}

Widget::Widget(Widget::InitArgs args)
    : WidgetTreeNode(args.parent.widget, args.parent.autoDelete)
    , m_dir(std::move(args.dir))

    , m_x(std::make_pair(std::move(args.x), 0))
    , m_y(std::make_pair(std::move(args.y), 0))
    , m_w(std::move(args.w))
    , m_h(std::move(args.h))

    , m_attrs(std::move(args.attrs))
{
    for(auto &[childPtr, offDir, offX, offY, autoDelete]: args.childList){
        if(childPtr){
            doAddChild(childPtr, autoDelete, true);
            childPtr->moveAt(std::move(offDir), std::move(offX), std::move(offY));
        }
    }
}

void Widget::update(double fUpdateTime)
{
    if(m_attrs.inst.update){
        return m_attrs.inst.update(this, fUpdateTime);
    }
    else{
        return updateDefault(fUpdateTime);
    }
}

void Widget::updateDefault(double fUpdateTime)
{
    foreachChild(false, [fUpdateTime, this](Widget *widget, bool)
    {
        widget->update(fUpdateTime);
    });
}

bool Widget::processEvent(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(m_attrs.inst.processEvent){
        return m_attrs.inst.processEvent(this, event, valid, m);
    }
    else{
        return processEventDefault(event, valid, m);
    }
}

bool Widget::processEventRoot(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    fflassert(!parent());

    m.x += dx();
    m.y += dy();

    return processEvent(event, valid, m);
}

bool Widget::processEventParent(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    const auto par = parent();
    fflassert(par);

    if(!m.calibrate(par)){
        return false;
    }

    return processEvent(event, valid, m.create(this->roi(par)));
}

bool Widget::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    bool took = false;
    uint64_t focusedWidgetID = 0;

    foreachChild(false, [&event, valid, &took, &focusedWidgetID, &m, this](Widget *widget, bool)
    {
        if(widget->show()){
            const bool validEvent = valid && !took;
            const bool takenEvent = widget->processEventParent(event, validEvent, m);

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

    if(focusedWidgetID && m_attrs.inst.moveOnFocus){
        if(auto widget = hasChild(focusedWidgetID)){
            moveBack(widget);
        }
    }
    return took;
}

void Widget::draw(Widget::ROIMap m) const
{
    if(m_attrs.inst.draw){
        m_attrs.inst.draw(this, m);
    }
    else{
        drawDefault(m);
    }
}

void Widget::drawRoot(Widget::ROIMap m) const
{
    fflassert(!parent());

    m.x += dx();
    m.y += dy();

    draw(m);
}

void Widget::drawChild(const Widget *child, Widget::ROIMap m) const
{
    fflassert(child);
    fflassert(hasChild(child->id()));
    drawAsChild(child, DIR_UPLEFT, child->dx(), child->dy(), m);
}

void Widget::drawAsChild(const Widget *gfxWidget, dir8_t gfxDir, int gfxDx, int gfxDy, Widget::ROIMap m) const
{
    if(!gfxWidget){
        return;
    }

    if(!m.calibrate(this)){
        return;
    }

    gfxWidget->draw(m.create(Widget::ROI
    {
        .x = gfxDx - xSizeOff(gfxDir, [gfxWidget]{ return gfxWidget->w(); }),
        .y = gfxDy - ySizeOff(gfxDir, [gfxWidget]{ return gfxWidget->h(); }),

        .w = gfxWidget->w(),
        .h = gfxWidget->h(),
    }));
}

void Widget::drawDefault(Widget::ROIMap m) const
{
    foreachChild([&m, this](const Widget *widget, bool)
    {
        if(widget->show()){
            drawChild(widget, m);
        }
    });
}

void Widget::afterResize()
{
    if(m_attrs.inst.afterResize){
        m_attrs.inst.afterResize(this);
    }
    else{
        afterResizeDefault();
    }
}

void Widget::afterResizeDefault()
{
    foreachChild([](Widget *child, bool){ child->afterResize(); });
}

int Widget::w() const
{
    const RecursionDetector hDetect(m_wCalc, name(), "w()");
    return Widget::evalSizeOpt(m_w, this, [this]{ return maxChildCoverWExcept(nullptr); });
}

int Widget::h() const
{
    const RecursionDetector hDetect(m_hCalc, name(), "h()");
    return Widget::evalSizeOpt(m_h, this, [this]{ return maxChildCoverHExcept(nullptr); });
}

int Widget::maxChildCoverWExcept(const Widget *except) const
{
    if(except){
        fflassert(hasChild(except->id()));
    }

    int maxW = 0;
    foreachChild([&maxW, except](const Widget *widget, bool)
    {
        if(widget != except && widget->localShow()){
            maxW = std::max<int>(maxW, widget->dx() + widget->w());
        }
    });
    return maxW;
}

int Widget::maxChildCoverHExcept(const Widget *except) const
{
    if(except){
        fflassert(hasChild(except->id()));
    }

    int maxH = 0;
    foreachChild([&maxH, except](const Widget *widget, bool)
    {
        if(widget != except && widget->localShow()){
            maxH = std::max<int>(maxH, widget->dy() + widget->h());
        }
    });
    return maxH;
}

int Widget::dx() const
{
    return Widget::evalInt(m_x.first, this) + m_x.second - xSizeOff(Widget::evalDir(m_dir, this), [this]{ return w(); });
}

int Widget::dy() const
{
    return Widget::evalInt(m_y.first, this) + m_y.second - ySizeOff(Widget::evalDir(m_dir, this), [this]{ return h(); });
}

static int _rd_helper(const Widget *a, const Widget *b, const auto func)
{
    const auto fnTraverse = [&func](const Widget *w) -> std::tuple<const Widget *, int>
    {
        const Widget *root= nullptr;
        int off = 0;

        while(w){
            off += func(w);
            if(const auto par = w->parent()){
                w = par;
            }
            else{
                root = w;
                break;
            }
        }

        return {root, off};
    };

    const auto [ra, offa] = fnTraverse(a);
    const auto [rb, offb] = fnTraverse(b);

    if(ra == rb){
        return offa - offb;
    }

    throw fflerror("widgets from different trees: %p vs %p", to_cvptr(a), to_cvptr(b));
}


int Widget::rdx(const Widget* other) const
{
    if(!other || (other == this)){
        return 0;
    }

    return _rd_helper(this, other, [](const Widget *w) { return w->dx(); });
}

int Widget::rdy(const Widget* other) const
{
    if(!other || (other == this)){
        return 0;
    }

    return _rd_helper(this, other, [](const Widget *w) { return w->dy(); });
}

bool Widget::focus() const
{
    return focusedChild();
}

bool Widget::localFocus() const
{
    return m_attrs.inst.focus;
}

void Widget::flipFocus()
{
    setFocus(!focus());
}

void Widget::setFocus(bool argFocus)
{
    if(auto focusedWidget = focusedChild()){
        if(focusedWidget == this){
            if(argFocus){
                return;
            }
            else{
                m_attrs.inst.focus = false;
                return;
            }
        }
        else{
            if(argFocus){
                focusedWidget->setFocus(false);
                m_attrs.inst.focus = true;
                return;
            }
            else{
                focusedWidget->setFocus(false);
                return;
            }
        }
    }
    else{
        // current widget and all its children not focused
        // need to check if its parent has focus

        if(argFocus){
            for(auto par = parent(); par; par = par->parent()){
                par->setFocus(false);
            }

            m_attrs.inst.focus = true;
            return;
        }
        else{
            return;
        }
    }
}

// focus helper
// we have tons of code like:
//
//     if(...){
//         p->focus(true);  // get focus
//         return true;     // since the event changes focus, then event is consumed
//     }
//     else{
//         p->focus(false); // event doesn't help to move focus to the widget
//         return false;    // not consumed, try next widget
//     }
//
// this function helps to simplify the code to:
//
//     return p->consumeFocus(...)

bool Widget::consumeFocus(bool argFocus, Widget *descendant)
{
    if(argFocus){
        if(descendant){
            if(hasDescendant(descendant->id())){
                if(auto focusedWidget = focusedDescendant()){
                    if(focusedWidget == descendant){
                        return true;
                    }
                    else{
                        focusedWidget->setFocus(true); // will clean focus of descendant's all ancestors
                        return true;
                    }
                }
                else{
                    descendant->setFocus(true);
                    return true;
                }
            }
            else{
                throw fflerror("widget has no descendant: %s", descendant->name());
            }
        }
        else{
            setFocus(true);
            return true;
        }
    }
    else{
        if(descendant){
            throw fflerror("unexpected descendant: %s", descendant->name());
        }
        else{
            setFocus(false);
            return false;
        }
    }
}

bool Widget::show() const
{
    // unlike active(), don't check if parent shows
    // i.e. in a item list page, we usually setup the page as auto-scaling mode to automatically updates its width/height
    //
    //  +-------------------+ <---- page
    //  | +---------------+ |
    //  | |       0       | | <---- item0
    //  | +---------------+ |
    //  | |       1       | | <---- item1
    //  | +---------------+ |
    //  |        ...        |
    //
    // when appending a new item, say item2, auto-scaling mode check current page height and append the new item at proper start:
    //
    //     page->addChild(item2, DIR_UPLEFT, 0, page->h(), true);
    //
    // if implementation of show() checks if parent shows or not
    // and if page is not shown, page->h() always return 0, even there is item0 and item1 inside
    //
    // this is possible
    // we may hide a widget before it's ready
    //
    // because item0->show() always return false, so does item1->show()
    // this makes auto-scaling fail
    //
    // we still setup m_show for each child widget
    // but when drawing, widget skips itself and all its child widgets if this->show() returns false

    if(m_parent && !m_parent->show()){
        return false;
    }
    return localShow();
}

bool Widget::localShow() const
{
    return Widget::evalBool(m_show.first, this) != m_show.second;
}

void Widget::flipShow()
{
    m_show.second = !m_show.second;
}

void Widget::setShow(Widget::VarBool argShow)
{
    m_show = std::make_pair(std::move(argShow), false);
}

bool Widget::active() const
{
    if(m_parent && !m_parent->active()){
        return false;
    }
    return localActive();
}

bool Widget::localActive() const
{
    return Widget::evalBool(m_active.first, this) != m_active.second;
}

void Widget::flipActive()
{
    m_active.second = !m_active.second;
}

void Widget::setActive(Widget::VarBool argActive)
{
    m_active = std::make_pair(std::move(argActive), false);
}

void Widget::moveXTo(Widget::VarInt arg)
{
    m_x = std::make_pair(std::move(arg), 0);
}

void Widget::moveYTo(Widget::VarInt arg)
{
    m_y = std::make_pair(std::move(arg), 0);
}

void Widget::moveTo(Widget::VarInt argX, Widget::VarInt argY)
{
    moveXTo(std::move(argX));
    moveYTo(std::move(argY));
}

void Widget::moveBy(Widget::VarInt argDX, Widget::VarInt argDY)
{
    const auto fnOp = [](std::pair<Widget::VarInt, int> &offset, Widget::VarInt update)
    {
        if(update.index() == 0){
            offset.second += std::get<int>(update);
        }
        else if(offset.first.index() == 0){
            offset.second += std::get<int>(offset.first);
            offset.first   = std::move(update);
        }
        else{
            offset.first = [u = std::move(offset.first), v = std::move(update)](const Widget *widgetPtr)
            {
                return std::get<std::function<int(const Widget *)>>(u)(widgetPtr)
                     + std::get<std::function<int(const Widget *)>>(v)(widgetPtr);
            };
        }
    };

    fnOp(m_x, std::move(argDX));
    fnOp(m_y, std::move(argDY));
}

void Widget::moveBy(Widget::VarInt argDX, Widget::VarInt argDY, const Widget::ROI &r)
{
    moveBy(std::move(argDX), std::move(argDY));
    if(const auto t = dx(); r.x > t){
        m_x.second += (r.x - t);
    }

    if(const auto t = dx() + w(); t > r.x + r.w){
        m_x.second -= (t - (r.x + r.w));
    }

    if(const auto t = dy(); r.y > t){
        m_y.second += (r.y - t);
    }

    if(const auto t = dy() + h(); t > r.y + r.h){
        m_y.second -= (t - (r.y + r.h));
    }
}

void Widget::moveAt(Widget::VarDir argDir, Widget::VarInt argX, Widget::VarInt argY)
{
    m_dir = std::move(argDir);
    moveTo(std::move(argX), std::move(argY));
}

void Widget::setW(Widget::VarSizeOpt argSize)
{
    if(m_attrs.type.canSetSize){
        m_w = std::move(argSize);
    }
    else{
        throw fflerror("can not resize %s", name());
    }
}

void Widget::setH(Widget::VarSizeOpt argSize)
{
    if(m_attrs.type.canSetSize){
        m_h = std::move(argSize);
    }
    else{
        throw fflerror("can not resize %s", name());
    }
}

void Widget::setSize(Widget::VarSizeOpt argW, Widget::VarSizeOpt argH)
{
    setW(std::move(argW));
    setH(std::move(argH));
}

std::string Widget::dumpTree() const
{
    std::vector<std::string> attrs;

    attrs.push_back(str_printf(R"("id":%llu)", to_llu(id())));
    attrs.push_back(str_printf(R"("name":"%s")", name()));
    attrs.push_back(str_printf(R"("dx":%d)", dx()));
    attrs.push_back(str_printf(R"("dy":%d)", dy()));
    attrs.push_back(str_printf(R"("w":%d)", w()));
    attrs.push_back(str_printf(R"("h":%d)", h()));
    attrs.push_back(str_printf(R"("show":%s)", to_boolcstr(show())));
    attrs.push_back(str_printf(R"("localShow":%s)", to_boolcstr(localShow())));
    attrs.push_back(str_printf(R"("active":%s)", to_boolcstr(active())));
    attrs.push_back(str_printf(R"("localActive":%s)", to_boolcstr(localActive())));
    attrs.push_back(str_printf(R"("focus":%s)", to_boolcstr(focus())));
    attrs.push_back(str_printf(R"("localFocus":%s)", to_boolcstr(localFocus())));

    if(!m_childList.empty()){
        std::vector<std::string> childAttrs;
        for(const auto &child: m_childList){
            if(child.widget){
                childAttrs.push_back(child.widget->dumpTree());
            }
        }
        attrs.push_back(str_printf(R"("children":[%s])", str_join(childAttrs, ",").c_str()));
    }

    if(auto extraAttrs = dumpTreeExt(); !extraAttrs.empty()){
        attrs.insert(attrs.end(), extraAttrs.begin(), extraAttrs.end());
    }

    return str_printf("{%s}", str_join(attrs, ",").c_str());
}

void Widget::dumpJsonFile(const char *path) const
{
    const auto json = dumpTree();

    int indent = 0;
    bool inString = false;

    std::ofstream ofs(path);

    std::string lc; // last character
    std::string cc; // curr character

    for(size_t begin = 0; begin < json.size();){
        lc = std::move(cc);
        cc = utf8f::peekUTF8Str(json.data() + begin, json.data() + json.length());
        begin += cc.length();

        if(cc == "\"" && (begin == 0 || lc != "\\")){
            inString = !inString;
        }

        if(!inString){
            if(cc == "{" || cc == "["){
                ofs << cc;
                ofs << '\n';
                indent++;
                ofs << std::string(indent * 4, ' ');
            }

            else if(cc == "}" || cc == "]"){
                ofs << '\n';
                indent--;
                ofs << std::string(indent * 4, ' ');
                ofs << cc;
            }

            else if(cc == ","){
                ofs << cc;
                ofs << '\n';
                ofs << std::string(indent * 4, ' ');
            }

            else if(cc == ":"){
                ofs << cc;
                ofs << ' ';
            }

            else if(cc != " "){
                ofs << cc;
            }
        }

        else{
            ofs << cc;
        }
    }
}
