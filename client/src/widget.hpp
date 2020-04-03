/*
 * =====================================================================================
 *
 *       Filename: widget.hpp
 *        Created: 08/12/2015 09:59:15
 *    Description: public API for class client only
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
#include <vector>
#include <cstdint>
#include <SDL2/SDL.h>
#include "lalign.hpp"

class widget
{
    private:
        struct childNode
        {
            widget *child;
            bool    autoDelete;

            childNode(widget *pwidget, bool bAutoDelete)
                : child(pwidget)
                , autoDelete(bAutoDelete)
            {}
        };

    protected:
        widget *m_parent;

    protected:
        bool m_show;
        bool m_focus;

    private:
        int m_x;
        int m_y;

    protected:
        int m_w;
        int m_h;

    protected:
        std::vector<childNode> m_childList;

    public:
        widget(int x, int y, int nW = 0, int nH = 0, widget *pParent = nullptr, bool bAutoDelete = false)
            : m_parent(pParent)
            , m_show(true)
            , m_focus(false)
            , m_x(x)
            , m_y(y)
            , m_w(nW)
            , m_h(nH)
        {
            if(m_parent){
                m_parent->m_childList.emplace_back(this, bAutoDelete);
            }
        }
        
    public:
        virtual ~widget()
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
                drawEx(X(), Y(), 0, 0, W(), H());
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
        virtual void Update(double ms)
        {
            for(auto &node: m_childList){
                node.child->Update(ms);
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
        int X() const
        {
            if(m_parent){
                return m_parent->X() + m_x;
            }else{
                return m_x;
            }
        }

        int Y() const
        {
            if(m_parent){
                return m_parent->Y() + m_y;
            }else{
                return m_y;
            }
        }

        int W() const
        {
            return m_w;
        }

        int H() const
        {
            return m_h;
        }

    public:
        bool in(int x, int y) const
        {
            return (x >= X() && x < X() + W()) && (y >= Y() && y < Y() + H());
        }

    public:
        void focus(bool bFocus)
        {
            m_focus = bFocus;
        }

        bool focus() const
        {
            return m_focus;
        }

    public:
        void show(bool bshow)
        {
            m_show = bshow;
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

class widgetGroup: public widget
{
    public:
        widgetGroup(int x, int y, int w, int h, widget *parent = nullptr, bool autoDelete = false)
            : widget(x, y, w, h, parent, autoDelete)
        {}

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) override
        {
            // we don't have box concept for widget class
            // this drawEx is a simple forward of parameters to all children

            for(auto &node: m_childList){
                node.child->drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
            }
        }
};
