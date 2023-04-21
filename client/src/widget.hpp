// class Widget has no resize()
// widget has no box concept like gtk, it can't calculate size in parent

#pragma once
#include <list>
#include <array>
#include <cstdint>
#include <optional>
#include <SDL2/SDL.h>
#include "mathf.hpp"
#include "lalign.hpp"
#include "bevent.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"

class Widget
{
    protected:
        Widget * const m_parent;

    protected:
        bool m_show   = true;
        bool m_focus  = false;
        bool m_active = true;

    private:
        dir8_t m_dir;

    private:
        int m_x;
        int m_y;

    protected:
        int m_w;
        int m_h;

    protected:
        int m_dz = 0;

    protected:
        std::list<std::pair<Widget *, bool>> m_childList;

    public:
        Widget(dir8_t dir, int x, int y, int w = 0, int h = 0, Widget *parent = nullptr, bool autoDelete = false)
            : m_parent(parent)
            , m_dir(dir)
            , m_x(x)
            , m_y(y)
            , m_w(w)
            , m_h(h)
        {
            if(m_parent){
                m_parent->m_childList.emplace_back(this, autoDelete);
            }

            fflassert(m_w >= 0, m_w, m_h);
            fflassert(m_h >= 0, m_w, m_h);
        }

    public:
        virtual ~Widget()
        {
            for(auto &[child, autoDelete]: m_childList){
                if(autoDelete){
                    delete child;
                }
            }
            m_childList.clear();
        }

    public:
        void draw() const
        {
            if(show()){
                drawEx(x(), y(), 0, 0, w(), h());
            }
        }

    public:
        virtual void drawEx(int,            // dst x on the screen coordinate
                            int,            // dst y on the screen coordinate
                            int,            // src x on the widget, take top-left as origin
                            int,            // src y on the widget, take top-left as origin
                            int,            // size to draw
                            int) const = 0; // size to draw

    public:
        void drawAt(
                int dir,
                int dstX,
                int dstY,

                int dstRgnX = 0,
                int dstRgnY = 0,

                int dstRgnW = -1,
                int dstRgnH = -1) const
        {
            const auto [dstUpLeftX, dstUpLeftY] = [dir, dstX, dstY, this]() -> std::array<int, 2>
            {
                switch(dir){
                    case DIR_UPLEFT   : return {dstX          , dstY          };
                    case DIR_UP       : return {dstX - w() / 2, dstY          };
                    case DIR_UPRIGHT  : return {dstX - w()    , dstY          };
                    case DIR_RIGHT    : return {dstX - w()    , dstY - h() / 2};
                    case DIR_DOWNRIGHT: return {dstX - w()    , dstY - h()    };
                    case DIR_DOWN     : return {dstX - w() / 2, dstY - h()    };
                    case DIR_DOWNLEFT : return {dstX          , dstY - h()    };
                    case DIR_LEFT     : return {dstX          , dstY - h() / 2};
                    default           : return {dstX - w() / 2, dstY - h() / 2};
                }
            }();

            int srcXCrop = 0;
            int srcYCrop = 0;
            int dstXCrop = dstUpLeftX;
            int dstYCrop = dstUpLeftY;
            int srcWCrop = w();
            int srcHCrop = h();

            if(mathf::ROICrop(
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
            for(auto &[child, autoDelete]: m_childList){
                child->update(fUpdateTime);
            }
        }

    public:
        //  valid: this event has been consumed by other widget
        // return: does current widget take this event?
        //         always return false if given event has been take by previous widget
        virtual bool processEvent(const SDL_Event &event, bool valid)
        {
            if(!show()){
                return false;
            }

            bool took = false;
            for(auto &[child, autoDelete]: m_childList){
                if(!child->show()){
                    continue;
                }
                took |= child->processEvent(event, valid && !took);
            }
            return took;
        }

    public:
        int x() const
        {
            const auto anchorX = [this]() -> int
            {
                if(m_parent){
                    return m_parent->x() + m_x;
                }
                else{
                    return m_x;
                }
            }();

            switch(m_dir){
                case DIR_UPLEFT   : return anchorX;
                case DIR_UP       : return anchorX - w() / 2;
                case DIR_UPRIGHT  : return anchorX - w();
                case DIR_RIGHT    : return anchorX - w();
                case DIR_DOWNRIGHT: return anchorX - w();
                case DIR_DOWN     : return anchorX - w() / 2;
                case DIR_DOWNLEFT : return anchorX;
                case DIR_LEFT     : return anchorX;
                default           : return anchorX - w() / 2;
            }
        }

        int y() const
        {
            const auto anchorY = [this]() -> int
            {
                if(m_parent){
                    return m_parent->y() + m_y;
                }
                else{
                    return m_y;
                }
            }();

            switch(m_dir){
                case DIR_UPLEFT   : return anchorY;
                case DIR_UP       : return anchorY;
                case DIR_UPRIGHT  : return anchorY;
                case DIR_RIGHT    : return anchorY - h() / 2;
                case DIR_DOWNRIGHT: return anchorY - h();
                case DIR_DOWN     : return anchorY - h();
                case DIR_DOWNLEFT : return anchorY - h();
                case DIR_LEFT     : return anchorY - h() / 2;
                default           : return anchorY - h() / 2;
            }
        }

        int w() const
        {
            return m_w;
        }

        int h() const
        {
            return m_h;
        }

        Widget * parent()
        {
            return m_parent;
        }

        const Widget * parent() const
        {
            return m_parent;
        }

    public:
        int dx() const
        {
            return x() - (m_parent ? m_parent->x() : 0);
        }

        int dy() const
        {
            return y() - (m_parent ? m_parent->y() : 0);
        }

        int dz() const
        {
            return m_dz;
        }

    public:
        bool in(int pixelX, int pixelY) const
        {
            return (pixelX >= x() && pixelX < x() + w()) && (pixelY >= y() && pixelY < y() + h());
        }

    public:
        void setFocus(bool argFocus)
        {
            m_focus = argFocus;
        }

        bool focus() const
        {
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

        bool consumeFocus(bool argFocus)
        {
            setFocus(argFocus);
            return argFocus;
        }

    public:
        void setShow(bool argShow)
        {
            m_show = argShow;
        }

        bool show() const
        {
            return m_show;
        }

        void flipShow()
        {
            setShow(!m_show);
        }

    public:
        void setActive(bool argActive)
        {
            m_active = argActive;
        }

        bool active() const
        {
            return m_active;
        }

        void flipActive()
        {
            setActive(!m_active);
        }

    public:
        void moveBy(std::optional<int> dx, std::optional<int> dy)
        {
            m_x += dx.value_or(0);
            m_y += dy.value_or(0);
        }

        void moveTo(std::optional<int> x, std::optional<int> y)
        {
            m_x = x.value_or(m_x);
            m_y = y.value_or(m_y);
        }

        void moveAt(std::optional<dir8_t> dir, std::optional<int> x, std::optional<int> y)
        {
            m_dir = dir.value_or(m_dir);
            m_x = x.value_or(m_x);
            m_y = y.value_or(m_y);
        }

    public:
        void setDz(int dzArg)
        {
            m_dz = dzArg;
        }

        void setSize(int argW, int argH)
        {
            fflassert(argW >= 0, argW, argH);
            fflassert(argH >= 0, argW, argH);

            m_w = argW;
            m_h = argH;
        }

        template<typename T> void setSize(const T &t)
        {
            const auto [argW, argH] = t;
            setSize(argW, argH);
        }
};

// simple *tiling* widget group
// this class won't handle widget overlapping

// when to use Widget, when to use WidgetContainer:
// when you are implement a widget from scratch, define how to draw every element, then use Widget
// if your widget is a composition of other widgets, has many child widgets inside, then you should use WidgetContainer

class WidgetContainer: public Widget
{
    private:
        mutable std::vector<const Widget *> m_childWidgetPtrList;

    public:
        WidgetContainer(dir8_t dir, int x, int y, int w, int h, Widget *parent = nullptr, bool autoDelete = false)
            : Widget(dir, x, y, w, h, parent, autoDelete)
        {}

    public:
        virtual bool processUnhandledEvent(const SDL_Event &)
        {
            // valid event but not consumed by any child widget
            return false;
        }

    public:
        bool processEvent(const SDL_Event &event, bool valid) override
        {
            // this function alters the draw order
            // if a WidgetContainer has children having overlapping then be careful

            if(!show()){
                return false;
            }

            bool took = false;
            auto focusedNode = m_childList.end();

            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                if(!p->first->show()){
                    continue;
                }

                took |= p->first->processEvent(event, valid && !took);
                if(focusedNode == m_childList.end() && p->first->focus()){
                    focusedNode = p;
                }
            }

            if(focusedNode != m_childList.end()){
                m_childList.push_front(*focusedNode);
                m_childList.erase(focusedNode);
            }
            return took;
        }

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            for(auto p = m_childList.rbegin(); p != m_childList.rend(); ++p){
                if(!p->first->show()){
                    continue;
                }

                int srcXCrop = srcX;
                int srcYCrop = srcY;
                int dstXCrop = dstX;
                int dstYCrop = dstY;
                int srcWCrop = srcW;
                int srcHCrop = srcH;

                if(!mathf::ROICrop(
                            &srcXCrop, &srcYCrop,
                            &srcWCrop, &srcHCrop,
                            &dstXCrop, &dstYCrop,

                            w(),
                            h(),

                            p->first->dx(), p->first->dy(), p->first->w(), p->first->h())){
                    continue;
                }
                p->first->drawEx(dstXCrop, dstYCrop, srcXCrop - p->first->dx(), srcYCrop - p->first->dy(), srcWCrop, srcHCrop);
            }
        }

    private:
        template<typename F> std::optional<std::pair<int, int>> getVarRange(const F &f) const
        {
            if(m_childList.empty()){
                return {};
            }

            const auto pr = std::minmax_element(m_childList.begin(), m_childList.end(), [&f](const auto &x, const auto &y)
            {
                return f(x) < f(y);
            });

            return std::make_pair(f(*pr.first), f(*pr.second));
        }

    public:
        std::optional<std::pair<int, int>> dxRange() const { return getVarRange([](const auto &node) { return node.first->dx(); }); }
        std::optional<std::pair<int, int>> dyRange() const { return getVarRange([](const auto &node) { return node.first->dy(); }); }
        std::optional<std::pair<int, int>> dzRange() const { return getVarRange([](const auto &node) { return node.first->dz(); }); }
};
