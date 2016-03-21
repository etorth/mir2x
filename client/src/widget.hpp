/*
 * =====================================================================================
 *
 *       Filename: widget.hpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 03/20/2016 18:28:08
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
        Widget()
            : m_X(0)
            , m_Y(0)
            , m_W(0)
            , m_H(0)
            , m_Focus(false)
            , m_Parent(nullptr)
        {}
        
        virtual ~Widget()
        {}

    // public:
    //     virtual void Draw(std::function<void(uint32_t, int, int)>)  = 0;
    //     virtual void Update(uint32_t)                               = 0;
    //     virtual bool ProcessEvent(SDL_Event &)                      = 0;

    public:
        int X()
        {
            if(m_Parent){
                return m_Parent->X() + m_X;
            }else{
                return m_X;
            }
        }

        void SetX(int nX)
        {
            m_X = nX;
        }

        int Y()
        {
            if(m_Parent){
                return m_Parent->Y() + m_Y;
            }else{
                return m_Y;
            }
        }

        void SetY(int nY)
        {
            m_Y = nY;
        }


        int W()
        {
            return m_W;
        }

        void SetW(int) = delete;

        int H()
        {
            return m_H;
        }

        void SetH(int) = delete;

    public:
        void SetParent(Widget *pWidget)
        {
            m_Parent = pWidget;
        }

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
        int m_X;
        int m_Y;
        int m_W;
        int m_H;

    protected:
        bool m_Focus;

    protected:
        Widget *m_Parent;
};
