/*
 * =====================================================================================
 *
 *       Filename: widget.hpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 03/22/2016 22:35:39
 *
 *    Description: public API for class game only
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
#include <cstdint>
#include <SDL2/SDL.h>

class Widget
{
    public:
        // TBD
        //
        // when creating, every widget should specify its parent, and
        // the location w.r.t its parent's up-left point.
        //
        // if pWidget is null, then it's the base widget, this means
        // widget location are ``fixed" w.r.t its parents, if you want
        // to move independently, you should have its pointer to do so.
        //
        // you can specify the width and height, but for tokenboard, it
        // is undefined before parsing the XML.
        //
        Widget(
                int nX,                     //
                int nY,                     //
                int nW = 0,                 //
                int nH = 0,                 //
                Widget * pWidget = nullptr, // by default all widget are independent
                bool bFreeWidget = true)    // delete automatically when deleting its parent

            : m_Parent(pWidget)
            , m_Focus(false)
            , m_X(nX)
            , m_Y(nY)
            , m_W(nW)
            , m_H(nH)
        {
            if(m_Parent){
                m_Parent->ClildV.emplace_back({this, bFreeWidget});
            }
        }
        
        virtual ~Widget()
        {
            for(auto &stPair: m_ChildV){
                if(stPair.second){
                    delete stPair.first;
                }
            }
        }

    public:
        virtual void Draw()
        {
            Draw(X(), Y());
        }

    public:
        // TBD
        // Draw is something that every widget should have
        // so make it pure virtual
        virtual void Draw(int, int) = 0;

        virtual void Update(double)
        {
            // not every widget should update
        }

        // TBD
        // it's not pure virtual, since not every widget should
        // accept events.
        //
        // Add a ``boo *" here to indicate the passed event is
        // valid or not:
        //
        //
        // +----------+
        // |          |
        // |    +---+ |
        // |    | 1 | |    +----------+
        // |    +---+ |    | +---+    |
        // +----------+    | | 2 |    |
        //                 | +---+    |
        //                 +----------+
        //
        // currently events will be dispatched to *every* widget, 
        // when button-1 and button-2 are overlapped. then both
        // event handler will be triggered if there is a click o-
        // ver button-1!
        //
        // but if we use ProcessEvent(const SDL_Event *) to pass a
        // null pointer to indicate event has already be grabbed, 
        // then can we update all the widgets properly?
        //
        // bValid to be null means this event is broadcast, we must
        // handle it?
        //
        virtual bool ProcessEvent(const SDL_Event &, bool *bValid = nullptr)
        {
        }

    public:

        // TBD
        // we don't have SetX/Y/W/H()
        // W/H() is not needed, X/Y(): most likely if we want to use
        // SetX/Y(), which just means we need to draw the widget at
        // different place.
        //
        // If we have this requirement, we could just make it as in-
        // dependent widget(parent is null) and use Draw(int, int) to
        // draw itself instead.
        //
        int X()
        {
            if(m_Parent){
                return m_Parent->X() + m_X;
            }else{
                return m_X;
            }
        }

        int Y()
        {
            if(m_Parent){
                return m_Parent->Y() + m_Y;
            }else{
                return m_Y;
            }
        }

        int W() { return m_W; }
        int H() { return m_H; }

    public:
        bool In(int nX, int nY)
        {
            return true
                && nX >= X()
                && nX <  X() + W()
                && nY >= Y()
                && nY <  Y() + H();
        }

    protected:
        Widget *m_Parent;
        bool    m_Focus;
        int     m_X;
        int     m_Y;
        int     m_W;
        int     m_H;

    protected:
        std::vector<std::pair<Widget *, bool>> m_ChildV;
};
