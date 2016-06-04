/*
 * =====================================================================================
 *
 *       Filename: animationset.cpp
 *        Created: 8/6/2015 5:43:46 AM
 *  Last Modified: 06/04/2016 02:58:12
 *
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

#include <string>

#include <FL/Fl.H>
#include <tinyxml2.h>
#include <FL/fl_draw.H>

#include "sysconst.hpp"
#include "mainwindow.hpp"
#include "animationset.hpp"

AnimationSet::AnimationSet()
    : m_R(SYS_MAXR)
    , m_DX(0)
    , m_DY(0)
{}

AnimationSet::~AnimationSet()
{}

bool AnimationSet::ImportMir2Animation(int nFileIndex, int nAnimationIndex)
{

    for(int nDirection = 0; nDirection < 8; ++nDirection){
        m_DirectionAlignX[nDirection] = 0;
        m_DirectionAlignY[nDirection] = 0;
    }

    // TODO I have no idea why I need to split it into 0 and 1 ~ 100
    for(int nDirection = 0; nDirection < 8; ++nDirection){
        m_ActionSet[0][nDirection].ImportMir2Action(nFileIndex, nAnimationIndex, 0, nDirection);
    }

    for(int nDirection = 0; nDirection < 8; ++nDirection){
        for(int nStatus = 1; nStatus < 100; ++nStatus){
            m_ActionSet[nStatus][nDirection].ImportMir2Action(nFileIndex, nAnimationIndex, nStatus, nDirection);
        }
    }
    return true;
}

void AnimationSet::SetStatus(int nStatus)
{
    m_Status = nStatus;
    m_ActionSet[m_Status][m_Direction].FirstFrame();
}

void AnimationSet::SetDirection(int nDirection)
{
    m_Direction = nDirection;
    m_ActionSet[m_Status][m_Direction].FirstFrame();
}

// (nPosX, nPosY) are center position of animation on current *window*
// means we already add the Fl_Box::x() and Fl_Box::y()
void AnimationSet::Draw(int nPosX, int nPosY)
{
    if(!m_ActionSet[m_Status][m_Direction].Valid()){ return; }

    // (nPosX - m_DX, nPosY - m_DY) is the start point of virtual image start point
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ShowCover()){
        if(g_MainWindow->TestMode() && !g_MainWindow->TestAnimation()){
            return;
        }else{
            DrawCover(nPosX, nPosY);
        }
    }

    int nX = nPosX + m_DX + m_DirectionAlignX[m_Direction];
    int nY = nPosY + m_DY + m_DirectionAlignY[m_Direction];
    m_ActionSet[m_Status][m_Direction].Draw(nX, nY);
}

void AnimationSet::DrawCover(int nPosX, int nPosY)
{
    auto nOldColor = fl_color();

    fl_color(FL_YELLOW);

    fl_begin_polygon();
    fl_arc(nPosX * 1.0, nPosY * 1.0, m_R * 1.0, 0.0, 360.0);
    fl_end_polygon();

    fl_color(FL_BLUE);

    fl_begin_line();
    fl_arc(nPosX * 1.0, nPosY * 1.0, m_R * 1.0, 0.0, 360.0);
    fl_end_line();

    fl_color(nOldColor);
}

void AnimationSet::UpdateFrame()
{
    m_ActionSet[m_Status][m_Direction].UpdateFrame();
}

void AnimationSet::TimeoutCallback(void *pArg)
{
    if(pArg){
        ((AnimationSet*)pArg)->UpdateFrame();
        extern MainWindow *g_MainWindow;
        g_MainWindow->RedrawAll();
        Fl::repeat_timeout(0.2, TimeoutCallback, pArg);
    }else{
        Fl::remove_timeout(TimeoutCallback);
    }
}

void AnimationSet::Clear()
{
}

void AnimationSet::DSetOffset(int dX, int dY)
{
    m_DX += dX;
    m_DY += dY;
}

void AnimationSet::FirstFrame()
{
    m_ActionSet[m_Status][m_Direction].FirstFrame();
}

void AnimationSet::PreviousFrame()
{
    m_ActionSet[m_Status][m_Direction].PreviousFrame();
}

void AnimationSet::NextFrame()
{
    m_ActionSet[m_Status][m_Direction].NextFrame();
}

void AnimationSet::LastFrame()
{
    m_ActionSet[m_Status][m_Direction].LastFrame();
}

void AnimationSet::DSetShadowOffset(int dX, int dY)
{
    m_ActionSet[m_Status][m_Direction].DSetShadowOffset(dX, dY);
}

void AnimationSet::DSetFrameAlign(int dX, int dY)
{
    m_ActionSet[m_Status][m_Direction].DSetFrameAlign(dX, dY);
}

void AnimationSet::DSetDirectionAlign(int dX, int dY)
{
    m_DirectionAlignX[m_Direction] += dX;
    m_DirectionAlignY[m_Direction] += dY;
}

void AnimationSet::DSetActionSetAlign(int dX, int dY)
{
    m_ActionSet[m_Status][m_Direction].DSetActionSetAlign(dX, dY);
}

bool AnimationSet::Valid(int nStatus, int nDirection)
{
    nStatus    %= 100;
    nDirection %= 10;
    return m_ActionSet[nStatus][nDirection].Valid();
}

bool AnimationSet::Valid()
{
    return Valid(m_Status, m_Direction);
}

bool AnimationSet::Export()
{
	bool bValid = false;
    for(int nStatus = 0; nStatus < 100; ++nStatus){
        for(int nDir = 0; nDir < 10; ++nDir){
            if(m_ActionSet[nStatus][nDir].Valid() && m_ActionSet[nStatus][nDir].FrameCount() > 0){
				bValid = true;
				goto _AnimationSet_Export_Continue;
            }
        }
    }

    if(!bValid){
        return false;
    }

_AnimationSet_Export_Continue:

    extern std::string g_WorkingPathName;
    std::string szXMLFileFullName;
    std::string szIMGPathFolderName;
    std::string szRCFileFullName;

    // if(g_WorkingPathName == ""){
	//	extern MainWindow *g_MainWindow;
    //     g_MainWindow->MakeWorkingFolder();
    // }
    //
    extern MainWindow *g_MainWindow;
    g_MainWindow->MakeWorkingFolder();

    if(g_WorkingPathName.back() == '/'){
        szXMLFileFullName   = g_WorkingPathName + "desc.xml";
        szIMGPathFolderName = g_WorkingPathName + "IMG";
		szRCFileFullName    = g_WorkingPathName + "drc.txt";
    }else{
        szXMLFileFullName   = g_WorkingPathName + "/desc.xml";
        szIMGPathFolderName = g_WorkingPathName + "/IMG";
		szRCFileFullName    = g_WorkingPathName + "/drc.txt";
    }

    tinyxml2::XMLDocument stXMLDoc;
    tinyxml2::XMLDocument *pDoc = &stXMLDoc;

    tinyxml2::XMLElement *pRoot = pDoc->NewElement("Root");
    pDoc->LinkEndChild(pRoot);

    for(int nStatus = 0; nStatus < 100; ++nStatus){
        for(int nDir = 0; nDir < 10; ++nDir){
            if(m_ActionSet[nStatus][nDir].Valid() && m_ActionSet[nStatus][nDir].FrameCount() > 0){
                { // for offset
                    tinyxml2::XMLElement *pActionSet = pDoc->NewElement("ActionSet");
                    pActionSet->SetAttribute("State", (std::to_string(nStatus).c_str()));
                    pActionSet->SetAttribute("Direction", (std::to_string(nDir).c_str()));
                    m_ActionSet[nStatus][nDir].Export(
                            szIMGPathFolderName.c_str(),
                            nStatus,
                            nDir,
                            m_DX,
                            m_DY,
                            pDoc,
                            pActionSet);
                    pRoot->LinkEndChild(pActionSet);
                }

            }
        }
    }
    pDoc->SaveFile(szXMLFileFullName.c_str());
    return true;
}
