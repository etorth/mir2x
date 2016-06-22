/*
 * =====================================================================================
 *
 *       Filename: animation.hpp
 *        Created: 06/20/2016 19:41:08
 *  Last Modified: 06/21/2016 12:04:48
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

            void ResetFrame(int nDX, int nDY, std::string szImageName)
            {
                DX = nDX;
                DY = nDY;
                ImageName = szImageName;
            }
        }AnimationFrame;

    protected:
        uint32_t m_ID;
        uint32_t m_SubID;
        std::vector<std::vector<AnimationFrame>> m_AnimationFrameV2D;

    public:
        void Add(int, int, int, int, Fl_Shared_Image *);
        void Update();

    public:
        template<typename... T> void Add(size_t nAction, size_t nDirection, size_t nFrame, bool bFShadow, T... stT)
        {
            if(nAction >= m_AnimationFrameV2D.size()){
                m_AnimationFrameV2D.resize((size_t)nAction);
            }

            if(nDirection >= m_AnimationFrameV2D[nAction].size()){
                m_AnimationFrameV2D[nAction].resize((size_t)nDirection);
            }

            if(nFrame >= m_AnimationFrameV2D[nAction][nDirection]){
                m_AnimationFrameV2D[nAction][nDirection].resize((size_t)nFrame);
            }

            m_AnimationFrameV2D[nAction][nDirection][bShadow ? 1 : 0].ResetFrame(std::forward<T>(stT)...);
        }

    public:
        void ResetLocation(int nX, bool bXRelative, int nY, bool bYRelative)
        {
            if(bXRelative){
                m_X += nX;
            }else{
                m_X = nX;
            }

            if(bYRelative){
                m_Y += nY;
            }else{
                m_Y = nY;
            }
        }
};
