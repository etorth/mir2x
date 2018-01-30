/*
 * =====================================================================================
 *
 *       Filename: operationarea.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *    Description: 
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
#include <FL/Fl.H>
#include <FL/Fl_Box.H>

class OperationArea: public Fl_Box
{
    private:
        // location on current box
        int m_CenterPositionX;
        int m_CenterPositionY;

    public:
        OperationArea(int, int, int, int);
        OperationArea(int, int, int, int, const char *);

        ~OperationArea();

    public:
        void draw();
        int  handle(int);

    private:
        void DrawAnimation();
};
