/*
 * =====================================================================================
 *
 *       Filename: animationset.cpp
 *        Created: 8/6/2015 5:43:46 AM
 *  Last Modified: 06/01/2016 17:25:22
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

#include "sidwindow.hpp"
#include "animationset.hpp"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "mainwindow.hpp"
#include <string>
#include <tinyxml2.h>

AnimationSet::AnimationSet()
    : m_DX(0)
    , m_DY(0)
{}

AnimationSet::~AnimationSet()
{}

bool AnimationSet::ImportMir2Animation(int nFileIndex, int nAnimationIndex)
{

    for(int nDirection = 0; nDirection < 8; ++nDirection){
        m_ActionSet[0][nDirection].ImportMir2Action( nFileIndex, nAnimationIndex, 0, nDirection);
        m_ActionSet[0][nDirection].EstimateRectCover(0.0, 0.0);
    }

    for(int nDirection = 0; nDirection < 8; ++nDirection){
        auto stRectCover = m_ActionSet[0][nDirection].GetRectCover();
        for(int nStatus = 1; nStatus < 100; ++nStatus){
            m_ActionSet[nStatus][nDirection].ImportMir2Action(
                    nFileIndex, nAnimationIndex, nStatus, nDirection);
            // m_ActionSet[nStatus][nDirection].EstimateRectCover(0.0, 0.0);
            m_ActionSet[nStatus][nDirection].SetRectCover(stRectCover);
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

void AnimationSet::Draw(int nPosX, int nPosY)
{
    // (nPosX - m_DX, nPosY - m_DY) is the start point of virtual image start point
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->ShowRectCover()){
        if(g_MainWindow->TestMode() && !g_MainWindow->TestAnimation()){
            return;
        }else{
            DrawRectCover(nPosX, nPosY);
        }
    }
    m_ActionSet[m_Status][m_Direction].Draw(nPosX - m_DX, nPosY - m_DY);
}

void AnimationSet::DrawRectCover(int nPosX, int nPosY)
{
    // auto stMPoint = m_ActionSet[m_Status][m_Direction].GetRectCover().Point(0);
    auto stPoint1 = m_ActionSet[m_Status][m_Direction].GetRectCover().Point(1);
    auto stPoint2 = m_ActionSet[m_Status][m_Direction].GetRectCover().Point(2);
    auto stPoint3 = m_ActionSet[m_Status][m_Direction].GetRectCover().Point(3);
    auto stPoint4 = m_ActionSet[m_Status][m_Direction].GetRectCover().Point(4);

    auto nOldColor = fl_color();
    fl_color(FL_YELLOW);
    fl_polygon(
            (int)(nPosX + stPoint1.first),
            (int)(nPosY + stPoint1.second),
            (int)(nPosX + stPoint2.first),
            (int)(nPosY + stPoint2.second),
            (int)(nPosX + stPoint3.first),
            (int)(nPosY + stPoint3.second),
            (int)(nPosX + stPoint4.first),
            (int)(nPosY + stPoint4.second));
    fl_color(FL_BLUE);
    // line right
    fl_line((int)(nPosX + stPoint2.first),
            (int)(nPosY + stPoint2.second),
            (int)(nPosX + stPoint3.first),
            (int)(nPosY + stPoint3.second));
    // line left
    fl_line((int)(nPosX + stPoint1.first),
            (int)(nPosY + stPoint1.second),
            (int)(nPosX + stPoint4.first),
            (int)(nPosY + stPoint4.second));
    // line bottom
    fl_line((int)(nPosX + stPoint3.first),
            (int)(nPosY + stPoint3.second),
            (int)(nPosX + stPoint4.first),
            (int)(nPosY + stPoint4.second));
    // for middle point: line 1
    fl_line((int)(nPosX + stPoint1.first),
            (int)(nPosY + stPoint1.second),
            (int)(nPosX + stPoint3.first),
            (int)(nPosY + stPoint3.second));
    // for middle point: line 2
    fl_line((int)(nPosX + stPoint2.first),
            (int)(nPosY + stPoint2.second),
            (int)(nPosX + stPoint4.first),
            (int)(nPosY + stPoint4.second));

    fl_color(FL_RED);
    // line top
    fl_line((int)(nPosX + stPoint1.first),
            (int)(nPosY + stPoint1.second),
            (int)(nPosX + stPoint2.first),
            (int)(nPosY + stPoint2.second));
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

void AnimationSet::MoveRectCover(double nDX, double nDY)
{
    m_ActionSet[m_Status][m_Direction].MoveRectCover(nDX, nDY);
}

void AnimationSet::DSetW(double nDW)
{
    extern MainWindow *g_MainWindow;
    switch(g_MainWindow->RectCoverCopyMethod()){
        case 2:
            {
                for(int nStatus = 0; nStatus < 100; ++nStatus){
                    if(m_ActionSet[nStatus][m_Direction].Valid()){
                        m_ActionSet[nStatus][m_Direction].DSetW(nDW);
                        m_ActionSet[nStatus][(m_Direction + 4) % 8].DSetW(nDW);
                    }
                }
                break;
            }
        case 1:
            {
                for(int nStatus = 0; nStatus < 100; ++nStatus){
                    if(m_ActionSet[nStatus][m_Direction].Valid()){
                        m_ActionSet[nStatus][m_Direction].DSetW(nDW);
                    }
                }
                break;
            }
        case 0:
            {
                m_ActionSet[m_Status][m_Direction].DSetW(nDW);
                break;
            }
        default:
            break;
    }
}

void AnimationSet::DSetH(double nDH)
{
    extern MainWindow *g_MainWindow;
    switch(g_MainWindow->RectCoverCopyMethod()){
        case 2:
            {
                for(int nStatus = 0; nStatus < 100; ++nStatus){
                    if(m_ActionSet[nStatus][m_Direction].Valid()){
                        m_ActionSet[nStatus][m_Direction].DSetH(nDH);
                        m_ActionSet[nStatus][(m_Direction + 4) % 8].DSetH(nDH);
                    }
                }
                break;
            }
        case 1:
            {
                for(int nStatus = 0; nStatus < 100; ++nStatus){
                    if(m_ActionSet[nStatus][m_Direction].Valid()){
                        m_ActionSet[nStatus][m_Direction].DSetH(nDH);
                    }
                }
                break;
            }
        case 0:
            {
                m_ActionSet[m_Status][m_Direction].DSetH(nDH);
                break;
            }
        default:
            break;
    }
}

bool AnimationSet::InCover(double fX, double fY)
{
    return m_ActionSet[m_Status][m_Direction].InCover(fX, fY);
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

bool AnimationSet::Export()
{
    extern SIDWindow *g_SIDWindow;
    if(g_SIDWindow->SID() < 0 || g_SIDWindow->SID() >= 1024){
        g_SIDWindow->ShowAll();
        return false;
    }

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
    FILE *pFile = fopen(szRCFileFullName.c_str(), "w+");
    if(!pFile){
        return false;
    }

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
                            g_SIDWindow->SID(),
                            nStatus,
                            nDir,
                            m_DX,
                            m_DY,
                            pDoc,
                            pActionSet);
                    pRoot->LinkEndChild(pActionSet);
                }

                { // for directive rectangle cover
                    int nW = m_ActionSet[nStatus][nDir].GetRectCover().W();
                    int nH = m_ActionSet[nStatus][nDir].GetRectCover().H();
                    fprintf(pFile, 
                            "%d,%d,%d,%d,%d\n",
                            g_SIDWindow->SID(), nStatus, nDir, nW, nH);
                }
            }
        }
    }
    fclose(pFile);
    pDoc->SaveFile(szXMLFileFullName.c_str());
    return true;
}
