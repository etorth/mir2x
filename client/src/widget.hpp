/*
 * =====================================================================================
 *
 *       Filename: widget.hpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 09/15/2017 10:41:13
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
#include <vector>
#include <cstdint>
#include <SDL2/SDL.h>

class Widget
{
    private:
        struct ChildRecord
        {
            Widget *Child;
            bool    AutoDelete;

            ChildRecord(Widget *pWidget, bool bAutoDelete)
                : Child(pWidget)
                , AutoDelete(bAutoDelete)
            {}
        };

    protected:
        Widget *m_Parent;

    protected:
        bool m_Show;
        bool m_Focus;

    protected:
        int m_X;
        int m_Y;
        int m_W;
        int m_H;

    protected:
        std::vector<ChildRecord> m_BindChildV;

    public:

        // when creating, every widget should specify its parent, and
        // the location w.r.t its parent's up-left point.
        //
        // if pWidget is null, then it's the base widget, this means
        // widget location are ``fixed" w.r.t its parents, if you want
        // to move independently, you should have its pointer to do so.
        //
        // you can specify the width and height, but for tokenboard, it
        // is undefined before parsing the XML.

        Widget(
                int nX,                     //
                int nY,                     //
                int nW = 0,                 //
                int nH = 0,                 //
                Widget *pWidget  = nullptr, // by default all widget are independent
                bool bAutoDelete = false)   // delete automatically when deleting its parent

            : m_Parent(pWidget)
            , m_Show(true)
            , m_Focus(false)
            , m_X(nX)
            , m_Y(nY)
            , m_W(nW)
            , m_H(nH)
        {
            if(m_Parent){
                m_Parent->m_BindChildV.emplace_back(this, bAutoDelete);
            }
        }
        
        virtual ~Widget()
        {
            for(auto &stRecord: m_BindChildV){
                if(stRecord.AutoDelete){
                    delete stRecord.Child;
                }
            }
        }

    public:
        virtual void Draw()
        {
            if(Show()){
                DrawEx(X(), Y(), 0, 0, W(), H());
            }
        }

    public:
        virtual void DrawEx(int,        // dst x on the screen coordinate
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

        // it's not pure virtual, since not every widget should
        // accept events.
        //
        // Add a ``bool *" here to indicate the passed event is
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

        virtual bool ProcessEvent(const SDL_Event &, bool *)
        {
            // since it's virtual
            // don't use default nullptr for bValid as default parameter
            return false;
        }

    public:

        // we don't have SetX/Y/W/H()
        // W/H() is not needed, for X/Y() most likely if we want to use
        // SetX/Y(), which means we need to draw the widget at different place.

        // If we have this requirement, we could just make it independent 
        // and use Draw(int, int) to draw itself instead.

        int X() const
        {
            if(m_Parent){
                return m_Parent->X() + m_X;
            }else{
                return m_X;
            }
        }

        int Y() const
        {
            if(m_Parent){
                return m_Parent->Y() + m_Y;
            }else{
                return m_Y;
            }
        }

        int W() const { return m_W; }
        int H() const { return m_H; }

    public:
        bool In(int nX, int nY) const
        {
            return true
                && nX >= X() && nX < X() + W()
                && nY >= Y() && nY < Y() + H();
        }

    public:
        void Focus(bool bFocus)
        {
            m_Focus = bFocus;
        }

        bool Focus() const
        {
            return m_Focus;
        }

    public:
        void Show(bool bShow)
        {
            m_Show = bShow;
        }

        bool Show() const
        {
            return m_Show;
        }

    public:
        void Move(int nDX, int nDY)
        {
            m_X += nDX;
            m_Y += nDY;
        }
};
