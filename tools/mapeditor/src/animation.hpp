/*
 * =====================================================================================
 *
 *       Filename: animation.hpp
 *        Created: 06/20/2016 19:41:08
 *  Last Modified: 06/20/2016 20:11:29
 *
 *    Description: animation for test
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
#include <FL/Fl_Shared_Image.H>

class Animation
{
    private:
        typedef struct _AnimationFrame{
            int DX;
            int DY;
            std::string ImageName;
            Fl_Shared_Image *Image;

            _AnimationFrame(int nDX = 0, int nDY = 0,
                    std::string szImageName = "", Fl_Shared_Image *pImage = nullptr)
                : DX(nDX)
                , DY(nDY)
                , ImageName(std::move(szImageName))
                , Image(pImage)
            {}
        }AnimationFrame;

    protected:
        uint32_t m_ID;
        uint32_t m_SubID;
        std::vector<std::vector<AnimationFrame>> m_AnimationFrameV2D;

    public:
        void Add(int, int, int, int, Fl_Shared_Image *);
        void Update();
};
