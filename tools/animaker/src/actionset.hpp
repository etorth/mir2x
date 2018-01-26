/*
 * =====================================================================================
 *
 *       Filename: actionset.hpp
 *        Created: 08/05/2015 11:22:52 PM
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

        // (virtual start point, (0, 0))
        //     +-------------------------------
        //     |
        //     |     (image start point, (ActionSet::m_DX[m_CurrentFrameIndex], ActionSet::m_DY[m_CurrentFrameIndex]))
        //     |      +------------------------
        //     |      |
        //     |      |
        //     |      |       (center point, (AnimationSet::m_DX, AnimationSet::m_DY))
        //     |      |          +

        // sometimes I think the shadow position is not good
        // so I add m_DSX and m_DSY to give an offset to make it better
        // bad idea?
        int       m_DSX[100];
        int       m_DSY[100];

        // shadow <-> body offset
        // this is read from the mil and mix files
        int       m_PX[100];
        int       m_PY[100];

        int       m_MaxW;
        int       m_MaxH;
        int       m_ImageMaxW;
        int       m_ImageMaxH;

        // actionset is one set in animation set
        // so I make this set can be moved by offset m_ActionSetAlignX and Y
        // then all frames have this offset relative to other action sets
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
