/*
 * =====================================================================================
 *
 *       Filename: animation.hpp
 *        Created: 06/20/2016 19:41:08
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
#include <functional>
#include <FL/Fl_Shared_Image.H>

class Animation
{
    private:
        typedef struct _AnimationFrame
        {
            int DX;
            int DY;
            std::string ImageName;

            bool BadImageName;
            Fl_Shared_Image *Image;

            _AnimationFrame(int nDX = 0, int nDY = 0, std::string szImageName = "")
                : DX(nDX)
                , DY(nDY)
                , ImageName(std::move(szImageName))
                , BadImageName(false)
                , Image(nullptr)
            {}

            void ResetFrame(int nDX, int nDY, std::string szImageName)
            {
                DX = nDX;
                DY = nDY;
                ImageName = std::move(szImageName);

                BadImageName = false;
                Image = nullptr;
            }

            bool Valid()
            {
                if(Image){ return true; }
                if(BadImageName){ return false; }

                if(ImageName.empty()){
                    BadImageName = true;
                    return false;
                }

                Image = Fl_Shared_Image::get(ImageName.c_str());
                if(Image){ return true; }

                BadImageName = true;
                return false;
            }
        }AnimationFrame;

    protected:
        uint32_t m_monsterID;
        uint32_t m_action;
        uint32_t m_direction;
        uint32_t m_frame;
        std::vector<std::vector<std::vector<std::array<AnimationFrame, 2>>>> m_animationFrameV2D;

    public:
        Animation(uint32_t nMonsterID = 0)
            : m_monsterID(nMonsterID)
            , m_action(0)
            , m_direction(0)
            , m_frame(0)
        {}
        ~Animation() = default;

    public:
        void Add(int, int, int, int, Fl_Shared_Image *);
        void Update();

    public:
        uint32_t MonsterID()
        {
            return m_monsterID;
        }

        uint32_t Action()
        {
            return m_action;
        }

        uint32_t Direction()
        {
            return m_direction;
        }

    public:
        template<typename... T> bool Add(uint32_t nAction, uint32_t nDirection, uint32_t nFrame, bool bShadow, T... stT)
        {
            if(nAction >= 16 || nDirection >= 8 || nFrame >= 32){ return false; }
            if(nAction >= (uint32_t)m_animationFrameV2D.size()){
                m_animationFrameV2D.resize((size_t)nAction + 1);
            }

            if(nDirection >= (uint32_t)m_animationFrameV2D[nAction].size()){
                m_animationFrameV2D[nAction].resize((size_t)nDirection + 1);
            }

            if(nFrame >= (uint32_t)m_animationFrameV2D[nAction][nDirection].size()){
                m_animationFrameV2D[nAction][nDirection].resize((size_t)nFrame + 1);
            }

            m_animationFrameV2D[nAction][nDirection][nFrame][bShadow ? 1 : 0].ResetFrame(std::forward<T>(stT)...);
            return true;
        }

    public:
        bool Valid()
        {
            return m_monsterID != 0;
        }

    public:
        void Draw(int, int);
        // to get rid of the bug of FLTK under linux
        void Draw(int, int, std::function<void(Fl_Shared_Image *, int, int)>);

    public:
        bool ActionValid(uint32_t);
        bool ActionValid()
        {
            return ActionValid(m_action);
        }

        bool DirectionValid(uint32_t, uint32_t);
        bool DirectionValid()
        {
            return DirectionValid(m_action, m_direction);
        }

        bool FrameValid(uint32_t, uint32_t, uint32_t, bool);
        bool FrameValid(bool bShadow)
        {
            return FrameValid(m_action, m_direction, m_frame, bShadow);
        }
        bool FrameValid()
        {
            return FrameValid(false) && FrameValid(true);
        }

    public:
        bool ResetAction(uint32_t);
        bool ResetDirection(uint32_t);
        bool ResetFrame(uint32_t, uint32_t, uint32_t);

    public:
        int AnimationW(uint32_t, uint32_t);
        int AnimationH(uint32_t, uint32_t);
};
