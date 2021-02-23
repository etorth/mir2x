/*
 * =====================================================================================
 *
 *       Filename: widget.hpp
 *        Created: 08/12/2015 09:59:15
 *    Description:
 *                 class Widget has no resize()
 *                 widget has no box concept like gtk, it can't calculate size in parent
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <list>
#include <array>
#include <cstdint>
#include <SDL2/SDL.h>
#include "mathf.hpp"
#include "lalign.hpp"
#include "bevent.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"

class Widget
{
    private:
        struct WidgetChildNode
        {
            Widget   *child = nullptr;
            bool autoDelete = false;
        };

    protected:
        Widget * const m_parent;

    protected:
        bool m_show  = true;
        bool m_focus = false;

    private:
        dir8_t m_dir;

    private:
        int m_x;
        int m_y;

    protected:
        int m_w;
        int m_h;

    protected:
        std::list<WidgetChildNode> m_childList;

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
                m_parent->m_childList.push_back(WidgetChildNode
                {
                    .child = this,
                    .autoDelete = autoDelete,
                });
            }
        }

    public:
        virtual ~Widget()
        {
            for(auto node: m_childList){
                if(node.autoDelete){
                    delete node.child;
                }
            }
            m_childList.clear();
        }

    public:
        virtual void draw() const
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
            for(auto &node: m_childList){
                node.child->update(fUpdateTime);
            }
        }

    public:
        //  valid: this event has been consumed by other widget
        // return: does current widget take this event?
        //         always return false if given event has been take by previous widget
        virtual bool processEvent(const SDL_Event &event, bool valid)
        {
            bool took = false;
            for(auto &node: m_childList){
                took |= node.child->processEvent(event, valid && !took);
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

        int dx() const
        {
            return x() - (m_parent ? m_parent->x() : 0);
        }

        int dy() const
        {
            return y() - (m_parent ? m_parent->y() : 0);
        }

        int w() const
        {
            return m_w;
        }

        int h() const
        {
            return m_h;
        }

    public:
        bool in(int pixelX, int pixelY) const
        {
            return (pixelX >= x() && pixelX < x() + w()) && (pixelY >= y() && pixelY < y() + h());
        }

    public:
        void focus(bool focusFlag)
        {
            m_focus = focusFlag;
        }

        bool focus() const
        {
            return m_focus;
        }

    public:
        void show(bool showFlag)
        {
            m_show = showFlag;
        }

        bool show() const
        {
            return m_show;
        }

    public:
        void moveBy(int dx, int dy)
        {
            m_x += dx;
            m_y += dy;
        }

        void moveTo(int x, int y)
        {
            m_x = x;
            m_y = y;
        }

        void moveAt(dir8_t dir, int x, int y)
        {
            m_dir = dir;
            m_x   = x;
            m_y   = y;
        }
};

// simple *tiling* widget group
// this class won't handle widget overlapping

class WidgetGroup: public Widget
{
    public:
        WidgetGroup(dir8_t dir, int x, int y, int w, int h, Widget *parent = nullptr, bool autoDelete = false)
            : Widget(dir, x, y, w, h, parent, autoDelete)
        {}

    public:
        bool processEvent(const SDL_Event &event, bool valid) override
        {
            // this function alters the draw order
            // if a WidgetGroup has children having overlapping then be careful

            bool took = false;
            auto focusedNode = m_childList.end();

            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                if(!show()){
                    continue;
                }

                took |= p->child->processEvent(event, valid && !took);
                if(focusedNode == m_childList.end() && p->child->focus()){
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
                if(!p->child->show()){
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

                            p->child->dx(), p->child->dy(), p->child->w(), p->child->h())){
                    continue;
                }
                p->child->drawEx(dstXCrop, dstYCrop, srcXCrop - p->child->dx(), srcYCrop - p->child->dy(), srcWCrop, srcHCrop);
            }
        }
};

// focus helper
// we have tons of code like:
//
//     if(...){
//         focus(true);     // get focus
//         return false;    // since the event changes focus, then event is consumed
//     }
//     else{
//         focus(false);    // event doesn't help to move focus to the widget
//         return false;    // not consumed, try next widget
//     }
//
// this function helps to simplify the code to:
//
//     return focusConsume(this, ...)
//
inline bool focusConsume(Widget *widgetPtr, bool setFocus)
{
    if(!widgetPtr){
        throw fflerror("invalid widget pointer: (null)");
    }

    widgetPtr->focus(setFocus);
    return setFocus;
}

inline void flipShow(Widget *widgetPtr)
{
    if(!widgetPtr){
        throw fflerror("invalid widget pointer: (null)");
    }
    widgetPtr->show(!widgetPtr->show());
}
