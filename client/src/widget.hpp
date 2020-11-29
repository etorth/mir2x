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
#include <cstdint>
#include <SDL2/SDL.h>
#include "mathf.hpp"
#include "lalign.hpp"
#include "bevent.hpp"
#include "fflerror.hpp"

class Widget
{
    private:
        struct WidgetChildNode
        {
            Widget *child;
            bool    autoDelete;

            WidgetChildNode(Widget *pwidget, bool autoDeleteFlag)
                : child(pwidget)
                , autoDelete(autoDeleteFlag)
            {}
        };

    protected:
        Widget * const m_parent;

    protected:
        bool m_show  = true;
        bool m_focus = false;

    private:
        int m_x;
        int m_y;

    protected:
        int m_w;
        int m_h;

    protected:
        std::list<WidgetChildNode> m_childList;

    public:
        Widget(int x, int y, int w = 0, int h = 0, Widget *parent = nullptr, bool autoDelete = false)
            : m_parent(parent)
            , m_x(x)
            , m_y(y)
            , m_w(w)
            , m_h(h)
        {
            if(m_parent){
                m_parent->m_childList.emplace_back(this, autoDelete);
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
        }

    public:
        virtual void draw()
        {
            if(show()){
                drawEx(x(), y(), 0, 0, w(), h());
            }
        }

    public:
        virtual void drawEx(int,        // dst x on the screen coordinate
                            int,        // dst y on the screen coordinate
                            int,        // src x on the widget, take top-left as origin
                            int,        // src y on the widget, take top-left as origin
                            int,        // size to draw
                            int) = 0;   // size to draw

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
            if(m_parent){
                return m_parent->x() + m_x;
            }else{
                return m_x;
            }
        }

        int y() const
        {
            if(m_parent){
                return m_parent->y() + m_y;
            }else{
                return m_y;
            }
        }

        int dx() const
        {
            return m_x;
        }

        int dy() const
        {
            return m_y;
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
};

// simple *tiling* widget group
// this class won't handle widget overlapping

class WidgetGroup: public Widget
{
    public:
        WidgetGroup(int x, int y, int w, int h, Widget *parent = nullptr, bool autoDelete = false)
            : Widget(x, y, w, h, parent, autoDelete)
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
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) override
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
//     return focusConsumer(this, ...)
//
inline bool focusConsumer(Widget *widgetPtr, bool setFocus)
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
