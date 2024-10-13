// class Widget has no resize(), only setSize()
// widget has no box concept like gtk, it can't calculate size in parent

#pragma once
#include <any>
#include <list>
#include <utility>
#include <vector>
#include <concepts>
#include <tuple>
#include <array>
#include <functional>
#include <variant>
#include <cstdint>
#include <optional>
#include <initializer_list>
#include <SDL2/SDL.h>
#include "mathf.hpp"
#include "lalign.hpp"
#include "bevent.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"

class Widget;        // size concept
class WidgetTreeNode // tree concept, used by class Widget only
{
    private:
        friend class Widget;

    private:
        struct WidgetChildElement
        {
            Widget *widget     = nullptr;
            bool    autoDelete = false;
        };

    private:
        const uint64_t m_id;

    private:
        mutable bool m_inLoop = false;

    private:
        Widget * m_parent;

    private:
        std::list<WidgetChildElement> m_childList; // widget shall NOT access this list directly

    private:
        std::vector<Widget *> m_delayList;

    private:
        WidgetTreeNode(Widget * = nullptr, bool = false); // can only be constructed by Widget::Widget()

    public:
        virtual ~WidgetTreeNode();

    public:
        /**/  Widget * parent(unsigned = 1);
        const Widget * parent(unsigned = 1) const;

    public:
        uint64_t id() const
        {
            return m_id;
        }

    public:
        template<std::invocable<const Widget *, bool, const Widget *, bool> F> void sort(F f)
        {
            m_childList.sort(m_childList.begin(), m_childList.end(), [&f](const auto &x, const auto &y)
            {
                if(x.widget && y.widget){
                    return f(x.widget, x.autoDelete, y.widget, y.autoDelete);
                }
                else if(x.widget){
                    return true;
                }
                else{
                    return false;
                }
            });
        }

    public:
        auto foreachChild(bool forward, std::invocable<Widget *, bool> auto f)
        {
            const ValueKeeper keepValue(m_inLoop, true);
            constexpr bool hasBoolResult = std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>;

            if(forward){
                for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                    if(p->widget){
                        if constexpr (hasBoolResult){
                            if(f(p->widget, p->autoDelete)){
                                return true;
                            }
                        }
                        else{
                            f(p->widget, p->autoDelete);
                        }
                    }
                }

                if constexpr (hasBoolResult){
                    return false;
                }
            }
            else{
                for(auto p = m_childList.rbegin(); p != m_childList.rend(); ++p){
                    if(p->widget){
                        if constexpr (hasBoolResult){
                            if(f(p->widget, p->autoDelete)){
                                return true;
                            }
                        }
                        else{
                            f(p->widget, p->autoDelete);
                        }
                    }
                }

                if constexpr (hasBoolResult){
                    return false;
                }
            }
        }

        auto foreachChild(bool forward, std::invocable<const Widget *, bool> auto f) const
        {
            const ValueKeeper keepValue(m_inLoop, true);
            constexpr bool hasBoolResult = std::is_same_v<std::invoke_result_t<decltype(f), const Widget *, bool>, bool>;

            if(forward){
                for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                    if(p->widget){
                        if constexpr (hasBoolResult){
                            if(f(p->widget, p->autoDelete)){
                                return true;
                            }
                        }
                        else{
                            f(p->widget, p->autoDelete);
                        }
                    }
                }

                if constexpr (hasBoolResult){
                    return false;
                }
            }
            else{
                for(auto p = m_childList.rbegin(); p != m_childList.rend(); ++p){
                    if(p->widget){
                        if constexpr (hasBoolResult){
                            if(f(p->widget, p->autoDelete)){
                                return true;
                            }
                        }
                        else{
                            f(p->widget, p->autoDelete);
                        }
                    }
                }

                if constexpr (hasBoolResult){
                    return false;
                }
            }
        }

    public:
        auto foreachChild(std::invocable<Widget *, bool> auto f)
        {
            if constexpr (std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>){
                return foreachChild(true, f);
            }
            else{
                foreachChild(true, f);
            }
        }

        auto foreachChild(std::invocable<const Widget *, bool> auto f) const
        {
            if constexpr (std::is_same_v<std::invoke_result_t<decltype(f), const Widget *, bool>, bool>){
                return foreachChild(true, f);
            }
            else{
                foreachChild(true, f);
            }
        }

    private:
        void execDeath() noexcept;

    public:
        void moveFront(const Widget *);
        void moveBack (const Widget *);

    public:
        virtual void onDeath() noexcept {}

    public:
        /**/  Widget *firstChild()       { for(auto &child: m_childList){ if(child.widget){ return child.widget; } } return nullptr; }
        const Widget *firstChild() const { for(auto &child: m_childList){ if(child.widget){ return child.widget; } } return nullptr; }

    public:
        /**/  Widget *lastChild()       { for(auto p = m_childList.rbegin(); p != m_childList.rend(); ++p){ if(p->widget){ return p->widget; } } return nullptr; }
        const Widget *lastChild() const { for(auto p = m_childList.rbegin(); p != m_childList.rend(); ++p){ if(p->widget){ return p->widget; } } return nullptr; }

    public:
        void clearChild(std::invocable<const Widget *, bool> auto f)
        {
            for(auto &child: m_childList){
                if(child.widget){
                    if(f(child.widget, child.autoDelete)){
                        if(child.autoDelete){
                            m_delayList.push_back(child.widget);
                        }
                        child.widget = nullptr;
                    }
                }
            }
        }

        void clearChild()
        {
            clearChild([](const Widget *, bool){ return true; });
        }

    public:
        virtual void purge();
        virtual void addChild(Widget *, bool);
        virtual void removeChild(Widget *, bool);

    public:
        bool hasChild() const;

    public:
        /**/  Widget *hasChild(uint64_t);
        const Widget *hasChild(uint64_t) const;

    public:
        Widget *hasChild(std::invocable<const Widget *, bool> auto f)
        {
            for(auto &child: m_childList){
                if(child.widget && f(child.widget, child.autoDelete)){
                    return child.widget;
                }
            }
            return nullptr;
        }

        const Widget *hasChild(std::invocable<const Widget *, bool> auto f) const
        {
            for(auto &child: m_childList){
                if(child.widget && f(child.widget, child.autoDelete)){
                    return child.widget;
                }
            }
            return nullptr;
        }
};

class Widget: public WidgetTreeNode
{
    public:
        using VarDir    = std::variant<             dir8_t, std::function<dir8_t(const Widget *)>>;
        using VarOffset = std::variant<                int, std::function<   int(const Widget *)>>;
        using VarSize   = std::variant<std::monostate, int, std::function<   int(const Widget *)>>;
        using VarFlag   = std::variant<               bool, std::function<  bool(const Widget *)>>;

    public:
        static bool hasIntDir(const Widget::VarDir &varDir)
        {
            return varDir.index() == 0;
        }

        static bool hasFuncDir(const Widget::VarDir &varDir)
        {
            return varDir.index() == 1;
        }

        static dir8_t  asIntDir(const Widget::VarDir &varDir) { return std::get<dir8_t>(varDir); }
        static dir8_t &asIntDir(      Widget::VarDir &varDir) { return std::get<dir8_t>(varDir); }

        static const std::function<dir8_t(const Widget *)> &asFuncDir(const Widget::VarDir &varDir) { return std::get<std::function<dir8_t(const Widget *)>>(varDir); }
        static       std::function<dir8_t(const Widget *)> &asFuncDir(      Widget::VarDir &varDir) { return std::get<std::function<dir8_t(const Widget *)>>(varDir); }

        static dir8_t evalDir(const Widget::VarDir &varDir, const Widget *widgetPtr)
        {
            return std::visit(VarDispatcher
            {
                [](const dir8_t &arg)
                {
                    return arg;
                },

                [widgetPtr](const auto &arg)
                {
                    return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
                },
            }, varDir);
        }

    public:
        static bool hasIntOffset(const Widget::VarOffset &varOff)
        {
            return varOff.index() == 0;
        }

        static bool hasFuncOffset(const Widget::VarOffset &varOff)
        {
            return varOff.index() == 1;
        }

        static int  asIntOffset(const Widget::VarOffset &varOff) { return std::get<int>(varOff); }
        static int &asIntOffset(      Widget::VarOffset &varOff) { return std::get<int>(varOff); }

        static const std::function<int(const Widget *)> &asFuncOffset(const Widget::VarOffset &varOff) { return std::get<std::function<int(const Widget *)>>(varOff); }
        static       std::function<int(const Widget *)> &asFuncOffset(      Widget::VarOffset &varOff) { return std::get<std::function<int(const Widget *)>>(varOff); }

        static int evalOffset(const Widget::VarOffset &varOffset, const Widget *widgetPtr)
        {
            return std::visit(VarDispatcher
            {
                [](const int &arg)
                {
                    return arg;
                },

                [widgetPtr](const auto &arg)
                {
                    return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
                },
            }, varOffset);
        }

    public:
        static bool hasSize(const Widget::VarSize &varSize)
        {
            return varSize.index() != 0;
        }

        static bool hasIntSize(const Widget::VarSize &varSize)
        {
            return varSize.index() == 1;
        }

        static bool hasFuncSize(const Widget::VarSize &varSize)
        {
            return varSize.index() == 2;
        }

        static int  asIntSize(const Widget::VarSize &varSize) { return std::get<int>(varSize); }
        static int &asIntSize(      Widget::VarSize &varSize) { return std::get<int>(varSize); }

        static const std::function<int(const Widget *)> &asFuncSize(const Widget::VarSize &varSize) { return std::get<std::function<int(const Widget *)>>(varSize); }
        static       std::function<int(const Widget *)> &asFuncSize(      Widget::VarSize &varSize) { return std::get<std::function<int(const Widget *)>>(varSize); }

    public:
        static bool hasBoolFlag(const Widget::VarFlag &varFlag)
        {
            return varFlag.index() == 0;
        }

        static bool hasFuncFlag(const Widget::VarFlag &varFlag)
        {
            return varFlag.index() == 1;
        }

        static bool  asBoolFlag(const Widget::VarFlag &varFlag) { return std::get<bool>(varFlag); }
        static bool &asBoolFlag(      Widget::VarFlag &varFlag) { return std::get<bool>(varFlag); }

        static const std::function<bool(const Widget *)> &asFuncFlag(const Widget::VarFlag &varFlag) { return std::get<std::function<bool(const Widget *)>>(varFlag); }
        static       std::function<bool(const Widget *)> &asFuncFlag(      Widget::VarFlag &varFlag) { return std::get<std::function<bool(const Widget *)>>(varFlag); }

        static bool evalFlag(const Widget::VarFlag &varFlag, const Widget *widgetPtr)
        {
            return std::visit(VarDispatcher
            {
                [](const bool &arg)
                {
                    return arg;
                },

                [widgetPtr](const auto &arg)
                {
                    return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
                },
            }, varFlag);
        }

    protected:
        std::pair<Widget::VarFlag, bool> m_show   {true, false};
        std::pair<Widget::VarFlag, bool> m_active {true, false};

    protected:
        bool m_focus  = false;

    protected:
        std::any m_data;

    private:
        Widget::VarDir m_dir;

    private:
        Widget::VarOffset m_x;
        Widget::VarOffset m_y;

    protected:
        Widget::VarSize m_w;
        Widget::VarSize m_h;

    public:
        Widget(Widget::VarDir argDir,

                Widget::VarOffset argX,
                Widget::VarOffset argY,

                Widget::VarSize argW = {},
                Widget::VarSize argH = {},

                std::initializer_list<std::tuple<Widget *, Widget::VarDir, Widget::VarOffset, Widget::VarOffset, bool>> argChildList = {},

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : WidgetTreeNode(argParent, argAutoDelete)
            , m_dir(std::move(argDir))
            , m_x  (std::move(argX))
            , m_y  (std::move(argY))
            , m_w  (std::move(argW))
            , m_h  (std::move(argH))
        {
            // don't check if w/h is a function
            // because it may refers to sub-widget which has not be initialized yet

            if(Widget::hasFuncDir   (m_dir)){ fflassert(Widget::asFuncDir   (m_dir), m_dir); }
            if(Widget::hasFuncOffset(m_x  )){ fflassert(Widget::asFuncOffset(m_x  ), m_x  ); }
            if(Widget::hasFuncOffset(m_y  )){ fflassert(Widget::asFuncOffset(m_y  ), m_y  ); }
            if(Widget::hasFuncSize  (m_w  )){ fflassert(Widget::asFuncSize  (m_w  ), m_w  ); }
            if(Widget::hasFuncSize  (m_h  )){ fflassert(Widget::asFuncSize  (m_h  ), m_h  ); }

            if(Widget::hasIntSize(m_w)){ fflassert(Widget::asIntSize(m_w) >= 0, m_w); }
            if(Widget::hasIntSize(m_h)){ fflassert(Widget::asIntSize(m_h) >= 0, m_h); }

            for(const auto &[childPtr, offDir, offX, offY, autoDelete]: argChildList){
                if(childPtr){
                    addChild(childPtr, autoDelete);
                    childPtr->moveAt(offDir, offX, offY);
                }
            }
        }

    public:
        void draw() const
        {
            if(show()){
                drawEx(x(), y(), 0, 0, w(), h());
            }
        }

    public:
        virtual void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
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

    private:
        static int sizeOff(int size, int index)
        {
            /**/ if(index <  0) return        0;
            else if(index == 0) return size / 2;
            else                return size - 1;
        }

        static int xSizeOff(dir8_t argDir, int argW)
        {
            switch(argDir){
                case DIR_UPLEFT   : return sizeOff(argW, -1);
                case DIR_UP       : return sizeOff(argW,  0);
                case DIR_UPRIGHT  : return sizeOff(argW,  1);
                case DIR_RIGHT    : return sizeOff(argW,  1);
                case DIR_DOWNRIGHT: return sizeOff(argW,  1);
                case DIR_DOWN     : return sizeOff(argW,  0);
                case DIR_DOWNLEFT : return sizeOff(argW, -1);
                case DIR_LEFT     : return sizeOff(argW, -1);
                default           : return sizeOff(argW,  0);
            }
        }

        static int ySizeOff(dir8_t argDir, int argH)
        {
            switch(argDir){
                case DIR_UPLEFT   : return sizeOff(argH, -1);
                case DIR_UP       : return sizeOff(argH, -1);
                case DIR_UPRIGHT  : return sizeOff(argH, -1);
                case DIR_RIGHT    : return sizeOff(argH,  0);
                case DIR_DOWNRIGHT: return sizeOff(argH,  1);
                case DIR_DOWN     : return sizeOff(argH,  1);
                case DIR_DOWNLEFT : return sizeOff(argH,  1);
                case DIR_LEFT     : return sizeOff(argH,  0);
                default           : return sizeOff(argH,  0);
            }
        }

    public:
        void drawAt(
                dir8_t dstDir,
                int    dstX,
                int    dstY,

                int dstRgnX = 0,
                int dstRgnY = 0,

                int dstRgnW = -1,
                int dstRgnH = -1) const
        {
            int srcXCrop = 0;
            int srcYCrop = 0;
            int dstXCrop = dstX - xSizeOff(dstDir, w());
            int dstYCrop = dstY - ySizeOff(dstDir, h());
            int srcWCrop = w();
            int srcHCrop = h();

            if(mathf::cropROI(
                        &srcXCrop, &srcYCrop,
                        &srcWCrop, &srcHCrop,
                        &dstXCrop, &dstYCrop,

                        w(),
                        h(),

                        0, 0, -1, -1,
                        dstRgnX, dstRgnY, dstRgnW, dstRgnH)){
                drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
            }
        }

    public:
        virtual void update(double fUpdateTime)
        {
            foreachChild(false, [fUpdateTime, this](Widget *widget, bool)
            {
                widget->update(fUpdateTime);
            });
        }

    public:
        //  valid: this event has been consumed by other widget
        // return: does current widget take this event?
        //         always return false if given event has been take by previous widget
        virtual bool processEvent(const SDL_Event &event, bool valid)
        {
            // this function alters the draw order
            // if a widget has children having overlapping then be careful

            if(!show()){
                return false;
            }

            bool took = false;
            uint64_t focusedWidgetID = 0;

            foreachChild(false, [&event, valid, &took, &focusedWidgetID](Widget *widget, bool)
            {
                if(widget->show()){
                    took |= widget->processEvent(event, valid && !took);
                    if(widget->focus()){
                        if(focusedWidgetID){
                            throw fflerror("multiple widget focused by one event");
                        }
                        else{
                            focusedWidgetID = widget->id();
                        }
                    }
                }
            });

            if(auto widget = hasChild(focusedWidgetID)){
                moveBack(widget);
            }

            return took;
        }

    public:
        static void handleEvent(const SDL_Event &event, bool &valid, std::initializer_list<Widget *> widgetList)
        {
            bool tookEvent = false;
            for(auto &w: widgetList){
                tookEvent |= w->processEvent(event, valid && !tookEvent);
            }
            valid = valid && !tookEvent;
        }

    public:
        virtual dir8_t dir() const
        {
            return Widget::evalDir(m_dir, this);
        }

        virtual int x() const
        {
            return Widget::evalOffset(m_x, this) + (m_parent ? m_parent->x() : 0) - xSizeOff(dir(), w());
        }

        virtual int y() const
        {
            return Widget::evalOffset(m_y, this) + (m_parent ? m_parent->y() : 0) - ySizeOff(dir(), h());
        }

        virtual int w() const
        {
            const auto width = std::visit(VarDispatcher
            {
                [](const int &arg)
                {
                    return arg;
                },

                [this](const std::function<int(const Widget *)> &arg)
                {
                    return arg ? arg(this) : throw fflerror("invalid argument");
                },

                [this](const auto &)
                {
                    int maxW = 0;
                    foreachChild([&maxW](const Widget *widget, bool)
                    {
                        if(widget->show()){
                            maxW = std::max<int>(maxW, widget->dx() + widget->w());
                        }
                    });
                    return maxW;
                }
            }, m_w);

            fflassert(width >= 0, width, m_w);
            return width;
        }

        virtual int h() const
        {
            const auto height = std::visit(VarDispatcher
            {
                [](const int &arg)
                {
                    return arg;
                },

                [this](const std::function<int(const Widget *)> &arg)
                {
                    return arg ? arg(this) : throw fflerror("invalid argument");
                },

                [this](const auto &)
                {
                    int maxH = 0;
                    foreachChild([&maxH](const Widget *widget, bool)
                    {
                        if(widget->show()){
                            maxH = std::max<int>(maxH, widget->dy() + widget->h());
                        }
                    });

                    return maxH;
                }
            }, m_h);

            fflassert(height >= 0, m_h);
            return height;
        }

    public:
        virtual int dx() const { return Widget::evalOffset(m_x, this) - xSizeOff(dir(), w()); }
        virtual int dy() const { return Widget::evalOffset(m_y, this) - ySizeOff(dir(), h()); }

    public:
        std::any &data()
        {
            return m_data;
        }

        Widget *setData(std::any argData)
        {
            m_data = std::move(argData);
            return this;
        }

    public:
        virtual bool in(int pixelX, int pixelY) const
        {
            return mathf::pointInRectangle<int>(pixelX, pixelY, x(), y(), w(), h());
        }

    public:
        virtual void setFocus(bool argFocus)
        {
            foreachChild([](Widget * widget, bool)
            {
                widget->setFocus(false);
            });

            m_focus = argFocus;
        }

        virtual bool focus() const
        {
            bool hasFocus = false;
            foreachChild([&hasFocus](const Widget * widget, bool) -> bool
            {
                return hasFocus = widget->focus();
            });

            if(hasFocus){
                return true;
            }

            return m_focus;
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

        bool consumeFocus(bool argFocus, Widget *child = nullptr)
        {
            if(argFocus){
                if(child){
                    if(hasChild(child->id())){
                        if(child->focus()){
                            // don't setup here
                            // when we setup focus in a deep call, this preserve focus of deep sub-widget
                        }
                        else{
                            setFocus(false);
                            child->setFocus(true);
                        }
                    }
                    else{
                        throw fflerror("widget has no child: %p", to_cvptr(child));
                    }
                }
                else{
                    setFocus(true);
                }
            }
            else{
                if(child){
                    throw fflerror("unexpected child: %p", to_cvptr(child));
                }
                else{
                    setFocus(false);
                }
            }
            return argFocus;
        }

    public:
        /**/  Widget *focusedChild()       { if(firstChild() && firstChild()->focus()){ return firstChild(); } return nullptr; }
        const Widget *focusedChild() const { if(firstChild() && firstChild()->focus()){ return firstChild(); } return nullptr; }

    public:
        void setShow(Widget::VarFlag argShow)
        {
            m_show = std::make_pair(std::move(argShow), false);
        }

        bool show() const
        {
            if(m_parent && !m_parent->show()){
                return false;
            }
            return Widget::evalFlag(m_show.first, this) != m_show.second;
        }

        void flipShow()
        {
            m_show.second = !m_show.second;
        }

    public:
        void setActive(Widget::VarFlag argActive)
        {
            m_active = std::make_pair(std::move(argActive), false);
        }

        bool active() const
        {
            if(m_parent && !m_parent->active()){
                return false;
            }
            return Widget::evalFlag(m_active.first, this) != m_active.second;
        }

        void flipActive()
        {
            m_active.second = !m_active.second;
        }

    public:
        void moveBy(Widget::VarOffset dx, Widget::VarOffset dy)
        {
            const auto fnOp = [](Widget::VarOffset u, Widget::VarOffset v) -> Widget::VarOffset
            {
                if(Widget::hasIntOffset(u) && Widget::hasIntOffset(v)){
                    return Widget::asIntOffset(u) + Widget::asIntOffset(v);
                }

                return [u = std::move(u), v = std::move(v)](const Widget *widgetPtr)
                {
                    return (Widget::hasFuncOffset(u) ? Widget::asFuncOffset(u)(widgetPtr) : Widget::asIntOffset(u))
                        +  (Widget::hasFuncOffset(v) ? Widget::asFuncOffset(v)(widgetPtr) : Widget::asIntOffset(v));
                };
            };

            m_x = fnOp(std::move(m_x), std::move(dx));
            m_y = fnOp(std::move(m_y), std::move(dy));
        }

        void moveAt(Widget::VarDir argDir, Widget::VarOffset argX, Widget::VarOffset argY)
        {
            m_dir = std::move(argDir);
            m_x   = std::move(argX  );
            m_y   = std::move(argY  );
        }

    public:
        void moveXTo(Widget::VarOffset arg) { m_x = std::move(arg); }
        void moveYTo(Widget::VarOffset arg) { m_y = std::move(arg); }

    public:
        void moveTo(Widget::VarOffset argX, Widget::VarOffset argY)
        {
            m_x = std::move(argX);
            m_y = std::move(argY);
        }

    public:
        void setW(Widget::VarSize argSize) { m_w = std::move(argSize); }
        void setH(Widget::VarSize argSize) { m_h = std::move(argSize); }

    public:
        void setSize(Widget::VarSize argW, Widget::VarSize argH)
        {
            m_w = std::move(argW);
            m_h = std::move(argH);
        }
};
