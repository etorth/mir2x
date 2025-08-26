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
    protected:
        using VarDir   = std::variant<             dir8_t, std::function<  dir8_t(const Widget *)>>;
        using VarOff   = std::variant<                int, std::function<     int(const Widget *)>>;
        using VarSize  = std::variant<std::monostate, int, std::function<     int(const Widget *)>>;
        using VarFlag  = std::variant<               bool, std::function<    bool(const Widget *)>>;
        using VarColor = std::variant<           uint32_t, std::function<uint32_t(const Widget *)>>;

    private:
        friend class Widget;

    protected:
        struct ChildElement final
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
        std::list<WidgetTreeNode::ChildElement> m_childList; // widget shall NOT access this list directly

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

        const char *name() const
        {
            return typeid(*this).name();
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
                        removeChildElement(child, true);
                    }
                }
            }
        }

        void clearChild()
        {
            clearChild([](const Widget *, bool){ return true; });
        }

    protected:
        virtual void removeChildElement(WidgetTreeNode::ChildElement &, bool);

    public:
        virtual void purge();
        virtual void removeChild(Widget *, bool);

    private:
        virtual void doAddChild(Widget *, bool) final;

    public:
        virtual void addChild  (Widget *, bool);
        virtual void addChildAt(Widget *, WidgetTreeNode::VarDir, WidgetTreeNode::VarOff, WidgetTreeNode::VarOff, bool);

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

    public:
        /**/  Widget *hasDescendant(uint64_t);
        const Widget *hasDescendant(uint64_t) const;

    public:
        /**/  Widget *hasDescendant(std::invocable<const Widget *, bool> auto);
        const Widget *hasDescendant(std::invocable<const Widget *, bool> auto) const;

    public:
        template<std::derived_from<Widget> T> /**/  T *hasParent();
        template<std::derived_from<Widget> T> const T *hasParent() const;
};

class Widget: public WidgetTreeNode
{
    public:
        using WidgetTreeNode::VarDir;
        using WidgetTreeNode::VarOff;
        using WidgetTreeNode::VarSize;
        using WidgetTreeNode::VarFlag;
        using WidgetTreeNode::VarColor;

    public:
        using WidgetTreeNode::ChildElement;

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
        static bool hasIntOff(const Widget::VarOff &varOff)
        {
            return varOff.index() == 0;
        }

        static bool hasFuncOff(const Widget::VarOff &varOff)
        {
            return varOff.index() == 1;
        }

        static int  asIntOff(const Widget::VarOff &varOff) { return std::get<int>(varOff); }
        static int &asIntOff(      Widget::VarOff &varOff) { return std::get<int>(varOff); }

        static const std::function<int(const Widget *)> &asFuncOff(const Widget::VarOff &varOff) { return std::get<std::function<int(const Widget *)>>(varOff); }
        static       std::function<int(const Widget *)> &asFuncOff(      Widget::VarOff &varOff) { return std::get<std::function<int(const Widget *)>>(varOff); }

        static int evalOff(const Widget::VarOff &varOffset, const Widget *widgetPtr)
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

    public:
        static bool hasU32Color(const Widget::VarColor &varColor)
        {
            return varColor.index() == 0;
        }

        static bool hasFuncColor(const Widget::VarColor &varColor)
        {
            return varColor.index() == 1;
        }

        static uint32_t  asU32Color(const Widget::VarColor &varColor) { return std::get<uint32_t>(varColor); }
        static uint32_t &asU32Color(      Widget::VarColor &varColor) { return std::get<uint32_t>(varColor); }

        static const std::function<uint32_t(const Widget *)> &asFuncColor(const Widget::VarColor &varColor) { return std::get<std::function<uint32_t(const Widget *)>>(varColor); }
        static       std::function<uint32_t(const Widget *)> &asFuncColor(      Widget::VarColor &varColor) { return std::get<std::function<uint32_t(const Widget *)>>(varColor); }

        static uint32_t evalColor(const Widget::VarColor &varColor, const Widget *widgetPtr)
        {
            return std::visit(VarDispatcher
            {
                [](const uint32_t &arg)
                {
                    return arg;
                },

                [widgetPtr](const auto &arg)
                {
                    return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
                },
            }, varColor);
        }

    private:
        class RecursionDetector final
        {
            private:
                bool &m_flag;

            public:
                RecursionDetector(bool &flag, const char *type, const char *func)
                    : m_flag(flag)
                {
                    if(m_flag){
                        throw fflerror("recursion detected in %s::%s", type, func);
                    }
                    else{
                        m_flag = true;
                    }
                }

                ~RecursionDetector()
                {
                    m_flag = false;
                }
        };

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
        std::pair<Widget::VarOff, int> m_x;
        std::pair<Widget::VarOff, int> m_y;

    private:
        bool m_disableSetSize = false;

    private:
        Widget::VarSize m_w;
        Widget::VarSize m_h;

    private:
        mutable bool m_hCalc = false;
        mutable bool m_wCalc = false;

    private:
        std::function<void(Widget *)> m_afterResizeHandler;
        std::function<bool(Widget *, const SDL_Event &, bool, int, int)> m_processEventHandler;

    public:
        Widget(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget::VarSize argW = {},
                Widget::VarSize argH = {},

                std::vector<std::tuple<Widget *, Widget::VarDir, Widget::VarOff, Widget::VarOff, bool>> argChildList = {},

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : WidgetTreeNode(argParent, argAutoDelete)
            , m_dir(std::move(argDir))
            , m_x  (std::make_pair(std::move(argX), 0))
            , m_y  (std::make_pair(std::move(argY), 0))
            , m_w  (std::move(argW))
            , m_h  (std::move(argH))
        {
            // don't check if w/h is a function
            // because it may refers to sub-widget which has not be initialized yet

            if(Widget::hasFuncDir (m_dir      )){ fflassert(Widget::asFuncDir (m_dir      ), m_dir      ); }
            if(Widget::hasFuncOff (m_x.first  )){ fflassert(Widget::asFuncOff (m_x.first  ), m_x.first  ); }
            if(Widget::hasFuncOff (m_y.first  )){ fflassert(Widget::asFuncOff (m_y.first  ), m_y.first  ); }
            if(Widget::hasFuncSize(m_w        )){ fflassert(Widget::asFuncSize(m_w        ), m_w        ); }
            if(Widget::hasFuncSize(m_h        )){ fflassert(Widget::asFuncSize(m_h        ), m_h        ); }

            if(Widget::hasIntSize(m_w)){ fflassert(Widget::asIntSize(m_w) >= 0, m_w); }
            if(Widget::hasIntSize(m_h)){ fflassert(Widget::asIntSize(m_h) >= 0, m_h); }

            for(auto &[childPtr, offDir, offX, offY, autoDelete]: argChildList){
                if(childPtr){
                    addChildAt(childPtr, std::move(offDir), std::move(offX), std::move(offY), autoDelete);
                }
            }
        }

    public:
        virtual void drawChildEx(const Widget *child, int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const final
        {
            fflassert(child);
            fflassert(hasChild(child->id()));

            if(!child->show()){
                return;
            }

            int drawSrcX = srcX;
            int drawSrcY = srcY;
            int drawSrcW = srcW;
            int drawSrcH = srcH;
            int drawDstX = dstX;
            int drawDstY = dstY;

            if(mathf::cropChildROI(
                        &drawSrcX, &drawSrcY,
                        &drawSrcW, &drawSrcH,
                        &drawDstX, &drawDstY,

                        w(),
                        h(),

                        child->dx(),
                        child->dy(),
                        child-> w(),
                        child-> h())){
                child->drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
            }
        }

        virtual void drawAt(
                dir8_t dstDir,
                int    dstX,
                int    dstY,

                int dstRgnX = 0,
                int dstRgnY = 0,

                int dstRgnW = -1,
                int dstRgnH = -1) const final
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

        virtual void drawRoot() const final
        {
            fflassert(!parent());
            drawEx(dx(), dy(), 0, 0, w(), h());
        }

    public:
        virtual void drawEx(int, int, int, int, int, int) const;

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
        virtual void update(double fUpdateTime)
        {
            foreachChild(false, [fUpdateTime, this](Widget *widget, bool)
            {
                widget->update(fUpdateTime);
            });
        }

    public:
        Widget *setProcessEvent(std::function<bool(Widget *, const SDL_Event &, bool, int, int)> argHandler)
        {
            m_processEventHandler = std::move(argHandler);
            return this;
        }

        virtual bool processEvent(const SDL_Event &event, bool valid, int dstX, int dstY) final
        {
            if(m_processEventHandler){ return m_processEventHandler(this, event, valid, dstX, dstY); }
            else                     { return   processEventDefault(      event, valid, dstX, dstY); }
        }

        virtual bool processParentEvent(const SDL_Event &event, bool valid, int dstX, int dstY) final
        {
            return processEvent(event, valid, dstX + dx(), dstY + dy());
        }

        virtual bool applyRootEvent(const SDL_Event &event) final
        {
            fflassert(!parent());
            return processEvent(event, true, dx(), dy());
        }

    protected:
        //  valid: this event has been consumed by other widget
        // return: does current widget take this event?
        //         always return false if given event has been take by previous widget
        //
        // this function alters the draw order
        // if a widget has children having overlapping then be careful
        virtual bool processEventDefault(const SDL_Event &, bool);

    public:
        Widget *setAfterResize(std::function<void(Widget *)> argHandler)
        {
            m_afterResizeHandler = std::move(argHandler);
            return this;
        }

        // this function is used for top-down widgets
        // means parent->afterResize() is the place to setup all its children's size
        //
        // if a widget's size is determined by its child
        // this function should keep no effect, like LayoutBoard, whose size is decided by all typset inside
        virtual void afterResize() final
        {
            if(m_afterResizeHandler){
                m_afterResizeHandler(this);
            }
            else{
                afterResizeDefault();
            }
        }

    protected:
        virtual void afterResizeDefault();

    public:
        virtual dir8_t dir() const
        {
            return Widget::evalDir(m_dir, this);
        }

        virtual int x() const
        {
            return Widget::evalOff(m_x.first, this) + m_x.second + (m_parent ? m_parent->x() : 0) - xSizeOff(dir(), w());
        }

        virtual int y() const
        {
            return Widget::evalOff(m_y.first, this) + m_y.second + (m_parent ? m_parent->y() : 0) - ySizeOff(dir(), h());
        }

        virtual int w() const
        {
            const RecursionDetector hDetect(m_wCalc, name(), "w()");
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

                [this](const auto &) // auto-scaling mode
                {
                    int maxW = 0;
                    foreachChild([&maxW](const Widget *widget, bool)
                    {
                        if(widget->localShow()){
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
            const RecursionDetector hDetect(m_hCalc, name(), "h()");
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

                [this](const auto &) // auto-scaling mode
                {
                    int maxH = 0;
                    foreachChild([&maxH](const Widget *widget, bool)
                    {
                        if(widget->localShow()){
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
        const auto &varw() const { return m_w; }
        const auto &varh() const { return m_h; }

    public:
        virtual int dx() const { return Widget::evalOff(m_x.first, this) + m_x.second - xSizeOff(dir(), w()); }
        virtual int dy() const { return Widget::evalOff(m_y.first, this) + m_y.second - ySizeOff(dir(), h()); }

    public:
        std::any &data()
        {
            return m_data;
        }

        const std::any &data() const
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
        virtual Widget *setFocus(bool argFocus)
        {
            foreachChild([](Widget * widget, bool)
            {
                widget->setFocus(false);
            });

            m_focus = argFocus;
            return this;
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
        Widget *setShow(Widget::VarFlag argShow)
        {
            m_show = std::make_pair(std::move(argShow), false);
            return this;
        }

        bool localShow() const
        {
            return Widget::evalFlag(m_show.first, this) != m_show.second;
        }

        bool show() const
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

        void flipShow()
        {
            m_show.second = !m_show.second;
        }

    public:
        Widget *setActive(Widget::VarFlag argActive)
        {
            m_active = std::make_pair(std::move(argActive), false);
            return this;
        }

        bool localActive() const
        {
            return Widget::evalFlag(m_active.first, this) != m_active.second;
        }

        bool active() const
        {
            if(m_parent && !m_parent->active()){
                return false;
            }
            return localActive();
        }

        void flipActive()
        {
            m_active.second = !m_active.second;
        }

    public:
        void moveBy(Widget::VarOff dx, Widget::VarOff dy)
        {
            const auto fnOp = [](std::pair<Widget::VarOff, int> &offset, Widget::VarOff update)
            {
                if(Widget::hasIntOff(update)){
                    offset.second += Widget::asIntOff(update);
                }
                else if(Widget::hasIntOff(offset.first)){
                    offset.second += Widget::asIntOff(offset.first);
                    offset.first   = std::move(update);
                }
                else{
                    offset.first = [u = std::move(offset.first), v = std::move(update)](const Widget *widgetPtr)
                    {
                        return Widget::asFuncOff(u)(widgetPtr) + Widget::asFuncOff(v)(widgetPtr);
                    };
                }
            };

            fnOp(m_x, std::move(dx));
            fnOp(m_y, std::move(dy));
        }

        void moveAt(Widget::VarDir argDir, Widget::VarOff argX, Widget::VarOff argY)
        {
            m_dir = std::move(argDir);
            m_x   = std::make_pair(std::move(argX), 0);
            m_y   = std::make_pair(std::move(argY), 0);
        }

    public:
        void moveXTo(Widget::VarOff arg) { m_x   = std::make_pair(std::move(arg), 0); }
        void moveYTo(Widget::VarOff arg) { m_y   = std::make_pair(std::move(arg), 0); }

    public:
        void moveTo(Widget::VarOff argX, Widget::VarOff argY)
        {
            moveXTo(std::move(argX));
            moveYTo(std::move(argY));
        }

    public:
        Widget *disableSetSize()
        {
            m_disableSetSize = true; // can not flip back
            return this;
        }

    public:
        virtual Widget *setW(Widget::VarSize argSize) final
        {
            if(m_disableSetSize){
                throw fflerror("can not resize %s", name());
            }

            m_w = std::move(argSize);
            return this;
        }

        virtual Widget *setH(Widget::VarSize argSize) final
        {
            if(m_disableSetSize){
                throw fflerror("can not resize %s", name());
            }

            m_h = std::move(argSize);
            return this;
        }

    public:
        virtual Widget *setSize(Widget::VarSize argW, Widget::VarSize argH) final
        {
            return setW(std::move(argW))->setH(std::move(argH));
        }
};

Widget *WidgetTreeNode::hasDescendant(std::invocable<const Widget *, bool> auto f)
{
    for(auto &child: m_childList){
        if(child.widget){
            if(f(child.widget, child.autoDelete)){
                return child.widget;
            }
            else if(auto descendant = child.widget->hasDescendant(f)){
                return descendant;
            }
        }
    }
    return nullptr;
}

const Widget *WidgetTreeNode::hasDescendant(std::invocable<const Widget *, bool> auto f) const
{
    for(auto &child: m_childList){
        if(child.widget){
            if(f(child.widget, child.autoDelete)){
                return child.widget;
            }
            else if(auto descendant = child.widget->hasDescendant(f)){
                return descendant;
            }
        }
    }
    return nullptr;
}

template<std::derived_from<Widget> T> T *WidgetTreeNode::hasParent()
{
    for(auto p = parent(); p; p = p->parent()){
        if(dynamic_cast<T *>(p)){
            return static_cast<T *>(p);
        }
    }
    return nullptr;
}

template<std::derived_from<Widget> T> const T *WidgetTreeNode::hasParent() const
{
    for(auto p = parent(); p; p = p->parent()){
        if(dynamic_cast<const T *>(p)){
            return static_cast<const T *>(p);
        }
    }
    return nullptr;
}
