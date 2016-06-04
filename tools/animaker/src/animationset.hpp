#pragma once
#include "actionset.hpp"
#include <functional>

class AnimationSet
{
    public:
        AnimationSet();
        ~AnimationSet();

    public:
        bool ImportMir2Animation(int, int);

    public:
        void Draw(int nPosX, int nPosY);
        void DrawCover(int nPosX, int nPosY);
        void Clear();

    public:
        void SetDirection(int);
        void SetStatus(int);
        void UpdateFrame();

    public:
        static void TimeoutCallback(void *);

    private:
        ActionSet   m_ActionSet[100][10];
        int         m_DirectionAlignX[10];
        int         m_DirectionAlignY[10];
        int         m_Direction;
        int         m_Status;
        int         m_R;

    private:
        int         m_DX;
        int         m_DY;

    public:
        void DSetOffset(int, int);
        void DSetShadowOffset(int, int);
        void DSetDirectionAlign(int, int);
        void DSetFrameAlign(int, int);
        void DSetActionSetAlign(int, int);
        bool Valid(int, int);
        bool Valid();

    public:
        int R()
        {
            return m_R;
        }

        void ResetR(int nR)
        {
            m_R = nR;
        }

    public:
        void FirstFrame();
        void PreviousFrame();
        void NextFrame();
        void LastFrame();

    public:
        bool Export();

    // (virtual start point, (0, 0))
    //     +-------------------------------
    //     |
    //     |     (image start point, (ActionSet::m_DX[m_CurrentFrameIndex], ActionSet::m_DY[m_CurrentFrameIndex]))
    //     |      +------------------------
    //     |      |
    //     |      |
    //     |      |       (center point, (AnimationSet::m_DX, AnimationSet::m_DY))
    //     |      |          +
};
