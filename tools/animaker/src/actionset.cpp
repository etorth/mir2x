/*
 * =====================================================================================
 *
 *       Filename: actionset.cpp
 *        Created: 8/5/2015 11:22:52 PM
 *  Last Modified: 06/03/2016 14:54:48
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
#include "filesys.hpp"
#include "savepng.hpp"
#include "actionset.hpp"
#include <algorithm>
#include "wilimagepackage.hpp"
#include "mainwindow.hpp"
#include "hexstring.hpp"
#include "wilanimationinfo.hpp"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

ActionSet::ActionSet()
    : m_CurrentFrameIndex(0)
    , m_FrameCount(0)
    , m_MaxW(0)
    , m_MaxH(0)
    , m_ImageMaxW(0)
    , m_ImageMaxH(0)
    , m_ActionSetAlignX(0)
    , m_ActionSetAlignY(0)
    , m_Valid(false)
{
    for(int i = 0; i < 100; ++i){
        m_PNG[0][i] = nullptr;
        m_PNG[1][i] = nullptr;
    }
}

ActionSet::~ActionSet()
{}

bool ActionSet::ImportMir2Action(int nFileIndex, int nAnimationIndex, int nStatus, int nDirection)
{
    // all parameters are assumed to be valid
    m_FileIndex      = nFileIndex;
    m_AnimationIndex = nAnimationIndex;
    m_Status         = nStatus;
    m_Direction      = nDirection;

    // when call this funcition
    // we assume that g_WilImagePackage has been set to nFileIndex
    // extern WilImagePackage g_WilImagePackage[2];

    std::memset(m_DSX, 0, 10 * sizeof(int));
    std::memset(m_DSY, 0, 10 * sizeof(int));
    std::memset(m_PX,  0, 10 * sizeof(int));
    std::memset(m_PY,  0, 10 * sizeof(int));

    // parameter nFileIndex, nAnimationIndex is for caching
    // otherwise we can just use nAnimationIndex

    // only init this class by click on AnimationPreviewWindow
    // so we can assume nFileIndex and nAnimationIndex are always valid
    m_FileIndex      = nFileIndex;
    m_AnimationIndex = nAnimationIndex;

    int       nMaxW    = 0;
    int       nMaxH    = 0;
    int       nDataLen = 0;
    uint32_t *pData    = nullptr;

    extern int g_MonsterWilFileStartIndex[];
    extern int g_MonsterShadowWilFileStartIndex[];

    int nBase0      = g_MonsterWilFileStartIndex[m_FileIndex - 1]       % 2;
    int nBase1      = g_MonsterShadowWilFileStartIndex[m_FileIndex - 1] % 2;
    int nBaseIndex  = WilAnimationStartBaseIndex(m_FileIndex, m_AnimationIndex, m_Status, m_Direction);
    int nFrameCount = WilAnimationFrameCount(m_FileIndex, m_AnimationIndex, m_Status, m_Direction);

    // assume largest framecount is 100: 0~99
    // and they are continuously located in the .wil file
    // otherwise for each nFileIndex/nAnimationIndex/nStatus/nDirection we need to set it
    for(int nFrame = 0; nFrame < nFrameCount; ++nFrame){

        extern WilImagePackage g_WilImagePackage[2];
        if(true
                && g_WilImagePackage[0].SetIndex(nFrame + nBaseIndex + nBase0)
                && g_WilImagePackage[0].CurrentImageValid()
                && g_WilImagePackage[1].SetIndex(nFrame + nBaseIndex + nBase1)
                && g_WilImagePackage[1].CurrentImageValid()
          ){
            auto stInfo0 = g_WilImagePackage[0].CurrentImageInfo();
            auto stInfo1 = g_WilImagePackage[1].CurrentImageInfo();

            {// non-shadow layer
                char szSaveFileName[128];
                //0 12 2 3 3 5
                //1 2  3 4 5 6
                //1: shadow or not
                //2: file index 1 ~ 20: Mon-1 ~ Mon-20
                //3: monster index in file
                //4: status
                //5: direction
                //6: frame index
                std::sprintf(szSaveFileName, "./IMG/0%02d%02d%02d%d%02d.PNG",
                        m_FileIndex, m_AnimationIndex, m_Status, m_Direction, nFrame);
                if(!FileExist(szSaveFileName)){
                    if(nDataLen < stInfo0.shWidth * stInfo0.shHeight){
                        delete pData;
                        pData    = new uint32_t[stInfo0.shWidth * stInfo0.shHeight];
                        nDataLen = stInfo0.shWidth * stInfo0.shHeight;
                    }
                    g_WilImagePackage[0].Decode(pData, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
                    SaveRGBABufferToPNG((uint8_t *)pData,
                            stInfo0.shWidth, stInfo0.shHeight,
                            szSaveFileName);
                }
                m_PNG[0][nFrame] = Fl_Shared_Image::get(szSaveFileName);
            }

            {// shadow layer
                char szSaveFileName[128];
                //0 12 2 3 3 5
                //1 2  3 4 5 6
                //1: shadow or not
                //2: file index 1 ~ 20: Mon-1 ~ Mon-20
                //3: monster index in file
                //4: status
                //5: direction
                //6: frame index
                std::sprintf(szSaveFileName, "./IMG/1%02d%02d%02d%d%02d.PNG",
                        m_FileIndex, m_AnimationIndex, m_Status, m_Direction, nFrame);
                if(!FileExist(szSaveFileName)){
                    if(nDataLen < stInfo1.shWidth * stInfo1.shHeight){
                        delete pData;
                        pData    = new uint32_t[stInfo1.shWidth * stInfo1.shHeight];
                        nDataLen = stInfo1.shWidth * stInfo1.shHeight;
                    }
                    g_WilImagePackage[1].Decode(pData, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
                    SaveRGBABufferToPNG((uint8_t *)pData,
                            stInfo1.shWidth, stInfo1.shHeight,
                            szSaveFileName);
                }
                m_PNG[1][nFrame] = Fl_Shared_Image::get(szSaveFileName);
            }

            m_DSX[nFrame] = stInfo1.shPX - stInfo0.shPX;
            m_DSY[nFrame] = stInfo1.shPY - stInfo0.shPY;
            m_PX [nFrame] = stInfo0.shPX;
            m_PY [nFrame] = stInfo0.shPY;

            // m_PNG[0]: start: (x - dx, y - dy)
            // m_PNG[1]: start: (x     , y     )

            if(m_PNG[0][nFrame] == nullptr || m_PNG[1][nFrame] == nullptr){
                continue;
            }
            int nMinX = (std::min)(0 - m_DSX[nFrame], 0);
            int nMinY = (std::min)(0 - m_DSY[nFrame], 0);
            int nMaxX = (std::max)(0 - m_DSX[nFrame] + m_PNG[0][nFrame]->w(), 0 + m_PNG[1][nFrame]->w());
            int nMaxY = (std::max)(0 - m_DSY[nFrame] + m_PNG[0][nFrame]->h(), 0 + m_PNG[1][nFrame]->h());

            nMaxW = (std::max)(nMaxW, nMaxX - nMinX);
            nMaxH = (std::max)(nMaxH, nMaxY - nMinY);
            m_FrameCount++;
        }else{
            m_PNG[0][nFrame] = nullptr;
            m_PNG[1][nFrame] = nullptr;
        }
    }

    delete pData;


    m_Valid = (m_FrameCount > 0);

    // EstimateRectCover();

    return m_Valid;
}

void ActionSet::Draw(int nVStartPX, int nVStartPY)
{
    // nVStartPX: x position on the virtual canvas, (0, 0) is on top-left
    // nVStartPY: y position on the virtual canvas
    // 
    extern MainWindow *g_MainWindow;
    if(g_MainWindow->TestMode() && !g_MainWindow->TestAnimation()){
        //selecting action set to show
        return;
    }

    if(true
            && m_PNG[0][m_CurrentFrameIndex] != nullptr
            && m_PNG[1][m_CurrentFrameIndex] != nullptr
      ){
        extern MainWindow *g_MainWindow;
        if(g_MainWindow->ShowShadowLayer()){
            m_PNG[1][m_CurrentFrameIndex]->draw(
                    m_ActionSetAlignX + m_PX[m_CurrentFrameIndex] + m_DSX[m_CurrentFrameIndex] + nVStartPX, 
                    m_ActionSetAlignY + m_PY[m_CurrentFrameIndex] + m_DSY[m_CurrentFrameIndex] + nVStartPY);
        }
        if(g_MainWindow->ShowShadowFrame()){
            int nX1, nX2, nY1, nY2;
            nX1 = m_ActionSetAlignX + m_PX[m_CurrentFrameIndex] + m_DSX[m_CurrentFrameIndex] + nVStartPX;
            nX2 = m_ActionSetAlignX + m_PX[m_CurrentFrameIndex] + m_DSX[m_CurrentFrameIndex] + nVStartPX + m_PNG[1][m_CurrentFrameIndex]->w();
            nY1 = m_ActionSetAlignY + m_PY[m_CurrentFrameIndex] + m_DSY[m_CurrentFrameIndex] + nVStartPY;
            nY2 = m_ActionSetAlignY + m_PY[m_CurrentFrameIndex] + m_DSY[m_CurrentFrameIndex] + nVStartPY + m_PNG[1][m_CurrentFrameIndex]->h();

            auto nOldColor = fl_color();
            fl_color(FL_MAGENTA);
            fl_line(nX1, nY1, nX2, nY1);
            fl_line(nX2, nY1, nX2, nY2);
            fl_line(nX2, nY2, nX1, nY2);
            fl_line(nX1, nY2, nX1, nY1);
            fl_color(nOldColor);
        }

        extern MainWindow *g_MainWindow;
        if(g_MainWindow->ShowBodyLayer()){
        m_PNG[0][m_CurrentFrameIndex]->draw(
                m_ActionSetAlignX + m_PX[m_CurrentFrameIndex] + nVStartPX, 
                m_ActionSetAlignY + m_PY[m_CurrentFrameIndex] + nVStartPY);
        }
        if(g_MainWindow->ShowBodyFrame()){
            int nX1, nX2, nY1, nY2;
            nX1 = m_ActionSetAlignX + m_PX[m_CurrentFrameIndex] + nVStartPX;
            nX2 = m_ActionSetAlignX + m_PX[m_CurrentFrameIndex] + nVStartPX + m_PNG[0][m_CurrentFrameIndex]->w();
            nY1 = m_ActionSetAlignY + m_PY[m_CurrentFrameIndex] + nVStartPY;
            nY2 = m_ActionSetAlignY + m_PY[m_CurrentFrameIndex] + nVStartPY + m_PNG[0][m_CurrentFrameIndex]->h();

            auto nOldColor = fl_color();
            fl_color(FL_CYAN);
            fl_line(nX1, nY1, nX2, nY1);
            fl_line(nX2, nY1, nX2, nY2);
            fl_line(nX2, nY2, nX1, nY2);
            fl_line(nX1, nY2, nX1, nY1);
            fl_color(nOldColor);
        }
    }else{
        // printf("oooooooops\n");
    }
}

void ActionSet::EstimateRectCover(double fX, double fY)
{
    if(m_PNG[0][0]){
        switch(m_Direction){
            case 0:
            case 4:
                {
                    SetCover(fX, fY, m_PNG[0][0]->w() / 3, m_PNG[0][0]->h() / 4);
                    break;
                }
            case 1:
            case 5:
            case 3:
            case 7:
                SetCover(fX, fY, m_PNG[0][0]->w() / 3, m_PNG[0][0]->h() / 4);
                break;
            case 2:
            case 6:
                SetCover(fX, fY, m_PNG[0][0]->w() / 3, m_PNG[0][0]->h() / 4);
                break;
            default:
                break;
        }
    }
}

void ActionSet::SetCover(double fMidX, double fMidY, double fW, double fH)
{
    fW = (std::max)(fW, 0.0);
    fH = (std::max)(fH, 0.0);

    double fX1 = 0.0;
    double fX2 = 0.0;
    double fY1 = 0.0;
    double fY2 = 0.0;

    switch(m_Direction){
        case 0:
            {
                fX1 = fMidX - fW / 2.0;
                fY1 = fMidY - fH / 2.0;
                fX2 = fMidX + fW / 2.0;
                fY2 = fMidY - fH / 2.0;
                break;
            }
        case 2:
            {
                fX1 = fMidX + fH / 2.0;
                fY1 = fMidY - fW / 2.0;
                fX2 = fMidX + fH / 2.0;
                fY2 = fMidY + fW / 2.0;
                break;
            }
        case 4:
            {
                fX1 = fMidX + fW / 2.0;
                fY1 = fMidY + fH / 2.0;
                fX2 = fMidX - fW / 2.0;
                fY2 = fMidY + fH / 2.0;
                break;
            }
        case 6:
            {
                fX1 = fMidX - fH / 2.0;
                fY1 = fMidY + fW / 2.0;
                fX2 = fMidX - fH / 2.0;
                fY2 = fMidY - fW / 2.0;
                break;
            }
        case 1:
            {
                double fL    = std::sqrt((fW * fW + fH * fH) / 4.0 - fW * fH * 0.38461538461 * 0.50);
                double fCosb = (fH * fH + 4.0 * fL * fL - fW * fW) / (4.0 * fH * fL);
                double fSinb = std::sqrt(1.0 - fCosb * fCosb); // always positive
                double fDX1  = 0 + fL * (0.83205029433 * fCosb - 0.55470019622 * fSinb);
                double fDY1  = 0 - std::sqrt(fL * fL - fDX1 * fDX1);
                double fTX   = fMidX + fH * 0.50 * 0.83205029433;
                double fTY   = fMidY - fH * 0.50 * 0.55470019622;

                double fTX1  = fMidX + fDX1;
                double fTY1  = fMidY + fDY1;
                double fTX2  = 2.0 * fTX - fTX1;
                double fTY2  = 2.0 * fTY - fTY1;

                fX1 = fTX1;
                fY1 = fTY1;
                fX2 = fTX2;
                fY2 = fTY2;

                break;
            }
        case 3:
            {
                std::swap(fW, fH);
                double fL    = std::sqrt((fW * fW + fH * fH) / 4.0 - fW * fH * 0.38461538461 * 0.50);
                double fCosb = (fH * fH + 4.0 * fL * fL - fW * fW) / (4.0 * fH * fL);
                double fSinb = std::sqrt(1.0 - fCosb * fCosb); // always positive
                double fDX1  = 0 + fL * (0.83205029433 * fCosb - 0.55470019622 * fSinb);
                double fDY1  = 0 - std::sqrt(fL * fL - fDX1 * fDX1);
                double fTX   = fMidX + fH * 0.50 * 0.83205029433;
                double fTY   = fMidY - fH * 0.50 * 0.55470019622;

                double fTX1  = fMidX + fDX1;
                double fTY1  = fMidY + fDY1;
                double fTX2  = 2.0 * fTX - fTX1;
                double fTY2  = 2.0 * fTY - fTY1;

                fX1   = fTX2;
                fY1   = fTY2;
                fX2   = 2.0 * fMidX - fTX1;
                fY2   = 2.0 * fMidY - fTY1;

                break;
            }
        case 5:
            {
                double fL    = std::sqrt((fW * fW + fH * fH) / 4.0 - fW * fH * 0.38461538461 * 0.50);
                double fCosb = (fH * fH + 4.0 * fL * fL - fW * fW) / (4.0 * fH * fL);
                double fSinb = std::sqrt(1.0 - fCosb * fCosb); // always positive
                double fDX1  = 0 + fL * (0.83205029433 * fCosb - 0.55470019622 * fSinb);
                double fDY1  = 0 - std::sqrt(fL * fL - fDX1 * fDX1);
                double fTX   = fMidX + fH * 0.50 * 0.83205029433;
                double fTY   = fMidY - fH * 0.50 * 0.55470019622;
                double fTX1  = fMidX + fDX1;
                double fTY1  = fMidY + fDY1;
                double fTX2  = 2.0 * fTX - fTX1;
                double fTY2  = 2.0 * fTY - fTY1;

                fX1   = 2.0 * fMidX - fTX1;
                fY1   = 2.0 * fMidY - fTY1;
                fX2   = 2.0 * fMidX - fTX2;
                fY2   = 2.0 * fMidY - fTY2;

                break;
            }
        case 7:
            {
                std::swap(fW, fH);
                double fL    = std::sqrt((fW * fW + fH * fH) / 4.0 - fW * fH * 0.38461538461 * 0.50);
                double fCosb = (fH * fH + 4.0 * fL * fL - fW * fW) / (4.0 * fH * fL);
                double fSinb = std::sqrt(1.0 - fCosb * fCosb); // always positive
                double fDX1  = 0 + fL * (0.83205029433 * fCosb - 0.55470019622 * fSinb);
                double fDY1  = 0 - std::sqrt(fL * fL - fDX1 * fDX1);
                double fTX   = fMidX + fH * 0.50 * 0.83205029433;
                double fTY   = fMidY - fH * 0.50 * 0.55470019622;
                double fTX1  = fMidX + fDX1;
                double fTY1  = fMidY + fDY1;
                double fTX2  = 2.0 * fTX - fTX1;
                double fTY2  = 2.0 * fTY - fTY1;

                fX1   = 2.0 * fMidX - fTX2;
                fY1   = 2.0 * fMidY - fTY2;
                fX2   = fTX1;
                fY2   = fTY1;

                break;
            }
        default:
            break;
    }

    m_RectCover.Set(fX1, fY1, fX2, fY2, fMidX, fMidY);

}

void ActionSet::UpdateFrame()
{
    if(!Valid()){
        return;
    }

    // playing animation is disabled
    extern MainWindow *g_MainWindow;
    if(!g_MainWindow->NeedUpdateFrame()){
        return;
    }
    
    if(g_MainWindow->TestMode() && !g_MainWindow->TestAnimation()){
        // selecting action set, don't need to update frame
        return;
    }
    
    // m_CurrentFrameIndex++;
    // m_CurrentFrameIndex %= m_FrameCount;

    // update one frame
    int nOldFrameIndex = m_CurrentFrameIndex;
    do{
        m_CurrentFrameIndex++;
        m_CurrentFrameIndex %= 100;
    }while(!m_PNG[0][m_CurrentFrameIndex] || !m_PNG[1][m_CurrentFrameIndex]);

    if(nOldFrameIndex >= m_CurrentFrameIndex){
        extern MainWindow *g_MainWindow;
        if(g_MainWindow->TestAnimation()){
            g_MainWindow->TestAnimationUpdate();
        }
    }
}

void ActionSet::MoveRectCover(double fDX, double fDY)
{
    m_RectCover.Move(fDX, fDY);
}

void ActionSet::DSetW(double fDW)
{
    auto stMPoint = m_RectCover.Point(0);
    SetCover(stMPoint.first, stMPoint.second, m_RectCover.W() + fDW, m_RectCover.H());
}

void ActionSet::DSetH(double fDH)
{
    auto stMPoint = m_RectCover.Point(0);
    SetCover(stMPoint.first, stMPoint.second, m_RectCover.W(), m_RectCover.H() + fDH);
}

bool ActionSet::InCover(double fX, double fY)
{
    return m_RectCover.In(fX, fY);
}

RectCover &ActionSet::GetRectCover()
{
    return m_RectCover;
}

void ActionSet::SetRectCover(const RectCover &cRect)
{
    m_RectCover = cRect;
}

int ActionSet::FrameCount()
{
    return m_FrameCount;
}

void ActionSet::FirstFrame()
{
    m_CurrentFrameIndex = 0;
}

void ActionSet::PreviousFrame()
{
    if(m_CurrentFrameIndex > 0){
        m_CurrentFrameIndex--;
    }
}

void ActionSet::NextFrame()
{
    if(m_CurrentFrameIndex < m_FrameCount - 1){
        m_CurrentFrameIndex++;
    }
}

void ActionSet::LastFrame()
{
    m_CurrentFrameIndex = m_FrameCount - 1;
}

void ActionSet::DSetShadowOffset(int dX, int dY)
{
    m_DSX[m_CurrentFrameIndex] += dX;
    m_DSY[m_CurrentFrameIndex] += dY;
}

void ActionSet::DSetFrameAlign(int dX, int dY)
{
    m_PX[m_CurrentFrameIndex] += dX;
    m_PY[m_CurrentFrameIndex] += dY;
}

void ActionSet::DSetActionSetAlign(int dX, int dY)
{
    m_ActionSetAlignX += dX;
    m_ActionSetAlignY += dY;
}

bool ActionSet::Valid()
{
    return m_Valid;
}

bool ActionSet::Export(
        const char *szIMGFolderName,
		int /* nSID */,
        int nState,
        int nDirection,
        int nCenterX,
        int nCenterY,
        tinyxml2::XMLDocument *pDoc,
        tinyxml2::XMLElement *pActionSet)
{
    if(false
            || !szIMGFolderName
            ||  std::strlen(szIMGFolderName) <= 0
            || !pDoc
            || !pActionSet
            ||  FrameCount() <= 0
      ){
        return false;
    }

    // seems m_FileIndex starts from 1
    uint32_t nLookID = (uint32_t)((m_FileIndex - 1) * 10 + m_AnimationIndex) & 0X00000FFF;
    char szLookID[16];
    HexString<uint32_t, 2>(nLookID, szLookID);
    szLookID[4] = '\0';

    for(int nFrame = 0; nFrame < FrameCount(); ++nFrame){
        auto pFrame = pDoc->NewElement("Frame");
        {
            { // shadow layer
                // offset info
                int nDX = nCenterX - (m_ActionSetAlignX + m_PX[nFrame] + m_DSX[nFrame]);
                int nDY = nCenterY - (m_ActionSetAlignY + m_PY[nFrame] + m_DSY[nFrame]);

                auto pShadow = pDoc->NewElement("Shadow");
                {
                    auto pDX = pDoc->NewElement("DX");
                    auto pDY = pDoc->NewElement("DY");
                    pDX->LinkEndChild(pDoc->NewText(std::to_string(-nDX).c_str()));
                    pDY->LinkEndChild(pDoc->NewText(std::to_string(-nDY).c_str()));

                    pShadow->LinkEndChild(pDX);
                    pShadow->LinkEndChild(pDY);
                }
                pFrame->LinkEndChild(pShadow);

                // graphics
                // move from ./IMG/XXXX.PNG to ./workingfolder/IMG/YYYY.PNG
                //
                // format of ./IMG/XXXX.PNG
                // std::sprintf(szSaveFileName, "./IMG/0%02d%02d%02d%d%02d.PNG",
                //         m_FileIndex, m_AnimationIndex, m_Status, m_Direction, nFrame);
                //
                // format of ./workingfolder/IMG/YYYY>PNG
                // precode | monster SID | Image Index
                //  0 ~ 63 |    0 ~ 1023 |  0 ~ 65535
                // 2 bytes      4 bytes      5 bytes  of char
                // 
                // shadow:
                //       2 |    SID      |  ImageIndex
                // body:
                //       1 |    SID      |  ImageIndex
                //
                // ImageIndex: 100 frames, 8 directions, 50 states
                //
                // |   State   |  Direction  |  FrameIndex  |
                //     2 bytes      1 byte         2 bytes
                //    00 ~ 99      0 ~ 9          0 ~ 99 : so max is 99 9 99 > 65535
                //                                         but we limit state to be 0 ~49
                //                                                   direction to be 0 ~7
                //                                         then is:
                //    00 ~ 49       0~7           0 ~ 99 : max is 49799 < 65535: good enough

                // old dup file name format
                // char szTmpFileName[64];
                // std::sprintf(szTmpFileName,
                //         "%s/02%04d%02d%d%02d.PNG",
                //         // 02 for shadow(6 bits), %04d for SID(10 bits), %02d%d%02d for ImageIndex (16 bits)
                //         // then to binary code: 0X FC000000 : precode
                //         //                      0X 03FF0000 : sid
                //         //                      0X 0000FFFF : image index
                //         szIMGFolderName,
				// 		nSID,
                //         nState,
                //         nDirection,
                //         nFrame);
                // DupFile(szTmpFileName, m_PNG[1][nFrame]->name());

                // new dup file name format
                char szTmpHexStringFileName[128];
                // uint32_t as Key, then:
                // FF FF FF FF
                // 01 ZZ Z- --
                //
                //  01  : for monster shadow
                // ZZZ  : for LookID
                // ---  : 12 bits: 12 ~ 09: 4 bits for states,     in total 16 states
                //                 08 ~ 06: 3 bits for directions, in total 08 directions
                //                 05 ~ 01: 5 bits for frames,     in total 32 frames
                //
                uint32_t nStateDirectionFrame = 0
                    + ((((uint32_t)nState)     & 0X0000000F) << 8)
                    + ((((uint32_t)nDirection) & 0X00000007) << 5)
                    + ((((uint32_t)nFrame)     & 0X0000001F) << 0);

                char szTmpHexStringStateDirectionFrame[16];
                HexString<uint32_t, 2>(nStateDirectionFrame, szTmpHexStringStateDirectionFrame);
                szTmpHexStringStateDirectionFrame[4] = '\0';

                std::sprintf(szTmpHexStringFileName,
                        "%s/01%s%s%s%s%04X%04X.PNG",
                        szIMGFolderName,
                        szLookID + 1,
                        szTmpHexStringStateDirectionFrame + 1, // skip one zero for first 4 bits
                        ((-nDX > 0) ? "1" : "0"),              // sign
                        ((-nDY > 0) ? "1" : "0"),              // sign
                        std::abs(-nDX),
                        std::abs(-nDY));
                DupFile(szTmpHexStringFileName, m_PNG[1][nFrame]->name());
            }

            {// body layer
                int nDX = nCenterX - (m_ActionSetAlignX + m_PX[nFrame]);
                int nDY = nCenterY - (m_ActionSetAlignY + m_PY[nFrame]);

                auto pBody = pDoc->NewElement("Body");
                {
                    auto pDX = pDoc->NewElement("DX");
                    auto pDY = pDoc->NewElement("DY");
                    pDX->LinkEndChild(pDoc->NewText(std::to_string(-nDX).c_str()));
                    pDY->LinkEndChild(pDoc->NewText(std::to_string(-nDY).c_str()));

                    pBody->LinkEndChild(pDX);
                    pBody->LinkEndChild(pDY);
                }
                pFrame->LinkEndChild(pBody);

                // old dup file name format
                // char szTmpFileName[64];
                // std::sprintf(szTmpFileName,
                //         "%s/01%04d%02d%d%02d.PNG",
                //         // 02 for shadow(6 bits), %04d for SID(10 bits), %02d%d%02d for ImageIndex (16 bits)
                //         // then to binary code: 0X FC000000 : precode
                //         //                      0X 03FF0000 : sid
                //         //                      0X 0000FFFF : image index
                //         szIMGFolderName,
				// 		nSID,
                //         nState,
                //         nDirection,
                //         nFrame);
                // DupFile(szTmpFileName, m_PNG[0][nFrame]->name());

                // new dup file name format
                char szTmpHexStringFileName[128];
                // uint32_t as Key, then:
                // FF FF FF FF
                // 01 ZZ Z- --
                //
                //  01  : for monster shadow
                // ZZZ  : for LookID
                // ---  : 12 bits: 12 ~ 09: 4 bits for states,     in total 16 states
                //                 08 ~ 06: 3 bits for directions, in total 08 directions
                //                 05 ~ 01: 5 bits for frames,     in total 32 frames
                //
                uint32_t nStateDirectionFrame = 0
                    + ((((uint32_t)nState)     & 0X0000000F) << 8)
                    + ((((uint32_t)nDirection) & 0X00000007) << 5)
                    + ((((uint32_t)nFrame)     & 0X0000001F) << 0);

                char szTmpHexStringStateDirectionFrame[16];
                HexString<uint32_t, 2>(nStateDirectionFrame, szTmpHexStringStateDirectionFrame);
                szTmpHexStringStateDirectionFrame[4] = '\0';

                std::sprintf(szTmpHexStringFileName,
                        "%s/00%s%s%s%s%04X%04X.PNG",
                        szIMGFolderName,
                        szLookID + 1,
                        szTmpHexStringStateDirectionFrame + 1, // skip one zero for first 4 bits
                        ((-nDX > 0) ? "1" : "0"),              // sign
                        ((-nDY > 0) ? "1" : "0"),              // sign
                        std::abs(-nDX),
                        std::abs(-nDY));
                DupFile(szTmpHexStringFileName, m_PNG[0][nFrame]->name());
            }
        }
        pActionSet->LinkEndChild(pFrame);
    }
    return true;
}
