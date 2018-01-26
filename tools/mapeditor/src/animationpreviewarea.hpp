/*
 * =====================================================================================
 *
 *       Filename: animationpreviewarea.hpp
 *        Created: 06/28/2016 23:27:58
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

#include <FL/Fl_Box.H>
#include <FL/Fl_Shared_Image.H>

class AnimationPreviewArea: public Fl_Box
{
    public:
        AnimationPreviewArea(int, int, int, int);
        ~AnimationPreviewArea();

    private:
        int handle(int);

    protected:
        virtual void draw();
};
