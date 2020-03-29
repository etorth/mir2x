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
        int m_X;
        int m_Y;

    protected:
        int m_W;
        int m_H;

    protected:
        std::vector<childNode> m_childList;

    public:
        widget(int nX, int nY, int nW = 0, int nH = 0, widget *pParent = nullptr, bool bAutoDelete = false)
            : m_parent(pParent)
            , m_show(true)
            , m_focus(false)
            , m_X(nX)
            , m_Y(nY)
            , m_W(nW)
            , m_H(nH)
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

        virtual void Update(double)
        {
            // widget supports update
            // but not every widget should update
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
                return m_parent->X() + m_X;
            }else{
                return m_X;
            }
        }

        int Y() const
        {
            if(m_parent){
                return m_parent->Y() + m_Y;
            }else{
                return m_Y;
            }
        }

        int W() const
        {
            return m_W;
        }

        int H() const
        {
            return m_H;
        }

    public:
        bool in(int nX, int nY) const
        {
            return (nX >= X() && nX < X() + W()) && (nY >= Y() && nY < Y() + H());
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
        void show(bool bShow)
        {
            m_show = bShow;
        }

        bool show() const
        {
            return m_show;
        }

    public:
        void moveBy(int nDX, int nDY)
        {
            m_X += nDX;
            m_Y += nDY;
        }

        void moveTo(int nX, int nY)
        {
            m_X = nX;
            m_Y = nY;
        }
};
