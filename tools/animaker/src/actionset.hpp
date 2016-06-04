#pragma once
#include "rectcover.hpp"
#include <functional>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl.H>
#include <tinyxml2.h>

class ActionSet
{
    public:
        ActionSet();
        ~ActionSet();
    public:
        void Draw(int, int);
        bool ImportMir2Action(int, int, int, int);
        bool Valid();
        void UpdateFrame();
    private:
        int       m_Direction;
        int       m_Status;
        int       m_AnimationIndex;
        int       m_FileIndex;
        int       m_CurrentFrameIndex;
        int       m_FrameCount;
        int       m_DSX[100];
        int       m_DSY[100];
        int       m_PX[100];
        int       m_PY[100];
        int       m_MaxW;
        int       m_MaxH;
        int       m_ImageMaxW;
        int       m_ImageMaxH;
        int       m_ActionSetAlignX;
        int       m_ActionSetAlignY;
        bool      m_Valid;
    private:
        Fl_Shared_Image *m_PNG[2][100];
    public:
        int  FrameCount();
        void FirstFrame();
        void PreviousFrame();
        void NextFrame();
        void LastFrame();
        void DSetShadowOffset(int, int);
        void DSetFrameAlign(int, int);
        void DSetActionSetAlign(int, int);

    public:
        bool Export(const char *, int, int, int, int, tinyxml2::XMLDocument *, tinyxml2::XMLElement *);
};
