/*
 * =====================================================================================
 *
 *       Filename: actionset.hpp
 *        Created: 08/05/2015 11:22:52 PM
 *  Last Modified: 06/04/2016 19:00:46
 *
 *    Description: actionset only take care of actionset align, body frame align, and
 *                 shadow frame align
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
#include <functional>

#include <FL/Fl.H>
#include <tinyxml2.h>
#include <FL/Fl_Shared_Image.H>

class ActionSet
{
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
        ActionSet();
        ~ActionSet();

    public:
        bool Valid();
        void Draw(int, int);
        void UpdateFrame();
        bool ImportMir2Action(int, int, int, int);

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
