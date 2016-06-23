/*
 * =====================================================================================
 *
 *       Filename: animation.hpp
 *        Created: 06/20/2016 19:41:08
 *  Last Modified: 06/23/2016 01:00:21
 *
 *    Description: animation for test, we only support monster animation currently
 *                 how about for human with weapon? do I need to support it?
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
#include <array>
#include <vector>
#include <string>
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
        uint32_t m_Frame;
        uint32_t m_Action;
        uint32_t m_Direction;
        uint32_t m_MonsterID;
        std::vector<std::vector<std::vector<std::array<AnimationFrame, 2>>>> m_AnimationFrameV2D;

    public:
        Animation(uint32_t nMonsterID = 0)
            : m_MonsterID(nMonsterID)
        {}
        ~Animation() = default;

    public:
        void Add(int, int, int, int, Fl_Shared_Image *);
        void Update();

    public:
        uint32_t MonsterID()
        {
            return m_MonsterID;
        }

    public:
        template<typename... T> bool Add(uint32_t nAction, uint32_t nDirection, uint32_t nFrame, bool bShadow, T... stT)
        {
            if(nAction >= 16 || nDirection >= 8 || nFrame >= 32){ return false; }
            if(nAction >= (uint32_t)m_AnimationFrameV2D.size()){
                m_AnimationFrameV2D.resize((size_t)nAction + 1);
            }

            if(nDirection >= (uint32_t)m_AnimationFrameV2D[nAction].size()){
                m_AnimationFrameV2D[nAction].resize((size_t)nDirection + 1);
            }

            if(nFrame >= (uint32_t)m_AnimationFrameV2D[nAction][nDirection].size()){
                m_AnimationFrameV2D[nAction][nDirection].resize((size_t)nFrame + 1);
            }

            m_AnimationFrameV2D[nAction][nDirection][nFrame][bShadow ? 1 : 0].ResetFrame(std::forward<T>(stT)...);
            return true;
        }

    public:
        bool Valid()
        {
            return m_MonsterID != 0;
        }

    public:
        void Draw(int, int);

        bool ActionValid();
        bool DirectionValid();
        bool FrameValid();

    public:
        bool ResetAction(uint32_t);
        bool ResetDirection(uint32_t);

    public:
        int AnimationW(uint32_t, uint32_t);
        int AnimationH(uint32_t, uint32_t);
};
