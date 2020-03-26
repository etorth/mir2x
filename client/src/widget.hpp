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

class Widget
{
    private:
        struct ChildNode
        {
            Widget *Child;
            bool    AutoDelete;

            ChildNode(Widget *pWidget, bool bAutoDelete)
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
        std::vector<ChildNode> m_ChildNodeList;

    public:
        Widget(int nX, int nY, int nW = 0, int nH = 0, Widget *pParent = nullptr, bool bAutoDelete = false)
            : m_Parent(pParent)
            , m_Show(true)
            , m_Focus(false)
            , m_X(nX)
            , m_Y(nY)
            , m_W(nW)
            , m_H(nH)
        {
            if(m_Parent){
                m_Parent->m_ChildNodeList.emplace_back(this, bAutoDelete);
            }
        }
        
    public:
        virtual ~Widget()
        {
            for(auto rstChildNode: m_ChildNodeList){
                if(rstChildNode.AutoDelete){
                    delete rstChildNode.Child;
                }
            }
        }

    public:
        virtual void Draw()
        {
            if(Show()){
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
        virtual bool ProcessEvent(const SDL_Event &, bool)
        {
            return false;
        }

    public:
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

        int W() const
        {
            return m_W;
        }

        int H() const
        {
            return m_H;
        }

    public:
        bool In(int nX, int nY) const
        {
            return (nX >= X() && nX < X() + W()) && (nY >= Y() && nY < Y() + H());
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

        void MoveTo(int nX, int nY)
        {
            m_X = nX;
            m_Y = nY;
        }

    public:
};
