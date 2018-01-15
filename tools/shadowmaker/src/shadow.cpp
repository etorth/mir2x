/*
 * =====================================================================================
 *
 *       Filename: shadow.cpp
 *        Created: 07/24/2015 07:20:18 PM
 *  Last Modified: 01/14/2018 19:04:49
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

#include <cmath>
#include <utility>
#include <cstdint>
#include <cstring>
#include <algorithm>

uint32_t *ShadowDecode(bool bProject,
        uint32_t *pData,
        int nW, int nH,
        int &nSW, int &nSH,
        uint32_t nColor)
{
	if(false
            || pData == nullptr
            || nW <= 0
            || nH <= 0
      ){
		return nullptr;
	}
    // if project, pShadowData should be of (nW + nH / 2) * (nH / 2 + 1)
    // otherwise of nW * nH
    if(bProject){
        int nNewW = (nW + nH / 2);
        int nNewH = (nH / 2 + 1);
        auto pShadowData = new uint32_t[nNewW * nNewH];
        std::memset(pShadowData, 0, nNewW * nNewH * sizeof(uint32_t));
        for(int nY = 0; nY < nH; ++nY){
            int nYCnt = nY - nY / 2;
            for(int nX = 0; nX < nW; ++nX){
                int nXCnt = nX + (nH - nY) / 2;
                if(pData[nY * nW + nX] & 0XFF000000){
                    pShadowData[nYCnt * nNewW + nXCnt] = nColor;
                }
            }
        }
        nSW = nNewW;
        nSH = nNewH;
        return pShadowData;
    }else{
        auto pShadowData = new uint32_t[nW * nH];
        std::memset(pShadowData, 0, nW * nH * sizeof(uint32_t));
        for(int nY = 0; nY < nH; ++nY){
            for(int nX = 0; nX < nW; ++nX){
                if(pData[nY * nW + nX] & 0XFF000000){
                    pShadowData[nY * nW + nX] = nColor;
                }
            }
        }
        nSW = nW;
        nSH = nH;
        return pShadowData;
    }
    return nullptr;
}

uint32_t *TwoPointShadowDecode(uint32_t *pData,
        int nW, int nH, int &nSW, int &nSH,
        int nX1, int nY1, int nX2, int nY2,
        uint32_t nShadowColor)
{
    if(nX1 > nX2){
        std::swap(nX1, nX2);
        std::swap(nY1, nY2);
    }

    if(nY1 <= nY2){
        // +----+--+-------+ (V1)
        // |    |  |       |
        // |(A1)|A2| (A3)  |
        // |    |  |       |
        // |(P1)|  |       |
        // +----+  |       |
        // |     \ |       |
        // |      \|       |
        // |   (P2)+-------+
        // |               |
        // |(A4)           |
        // +---------------+
        // pixel in A123 will create shadow
        // we drop pixel in A4

        int nNewW = nY2 / 2 + nW;
        int nNewH = nY1 / 2 + nY2 - nY1;

        nSW = nNewW;
        nSH = nNewH;

        auto pSData = new uint32_t[nNewW * nNewH];
        std::memset(pSData, 0, nNewW * nNewH * sizeof(uint32_t));
        for(int nYCnt = 0; nYCnt < nNewH; ++nYCnt){
            for(int nXCnt = 0; nXCnt < nNewW; ++nXCnt){
                if(true
                        && (nYCnt + nY1 / 2 - nY1) > 0
                        && (nYCnt + nY1 / 2 - nY1) * (nX2 - nX1) > (nY2 - nY1) * (nXCnt - nX1)
                  ){
                    continue;
                }

                int  nMapX, nMapY;
                int  nCrossX = nXCnt - (nNewH - nYCnt);
                bool bExpand = false;
                bool bMapped = false;

                if(nX2 <= nCrossX && nCrossX < nW){
                    nMapX   = nCrossX;
                    nMapY   = nY2 - (nNewH - nYCnt) * 2;
                    bExpand = true;
                    bMapped = true;
                }else if(nX1 - (nY2 - nY1) <= nCrossX && nCrossX < nX2){
                    double dRatio = 1.0 *
                        (nCrossX - (nX1 - (nY2 - nY1))) / (std::max)(1, nX2 - (nX1 - (nY2 - nY1)));
                    dRatio = (std::max)(0.0, (std::min)(dRatio, 1.0));
                    int nRealCrossX = nX1 + std::lround((nX2 - nX1) * dRatio);
                    int nRealCrossY = nY1 + std::lround((nY2 - nY1) * dRatio);
                    nMapX   = nRealCrossX;
                    nMapY   = nRealCrossY - 2 * (nRealCrossY - (nYCnt + nY1 / 2));
                    bExpand = false;
                    bMapped = true;
                }else if(-(nY2 - nY1) <= nCrossX && nCrossX < nX1 - (nY2 - nY1)){
                    nMapX   = nCrossX + (nY2 - nY1);
                    nMapY   = nY1 - 2 * (nY1 / 2 - nYCnt);
                    bExpand = true;
                    bMapped = true;
                }else{
                    nMapX   = -50000;
                    nMapY   = -50000;
                    bExpand =  false;
                    bMapped =  false; // not a valid mapping point
                }

                if(bMapped){
                    if(bExpand){
                        {
                            for(int nVX = -1; nVX <= 1; ++nVX){
                                for(int nVY = -1; nVY <= 1; ++nVY){
                                    int nMapXX = nMapX + nVX;
                                    int nMapYY = nMapY + nVY;
                                    if(nMapXX >= 0 && nMapXX < nW && nMapYY >= 0 && nMapYY < nH){
                                        if(pData[nMapXX + nMapYY * nW] & 0XFF000000){
                                            pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                                            goto __TwoPointShadowDecode_Pixel_Set;
                                        }
                                    }
                                }
                            }
__TwoPointShadowDecode_Pixel_Set: ;
                        }
                    }else{
                        if(nMapX >= 0 && nMapX < nW && nMapY >= 0 && nMapY < nH){
                            if(pData[nMapX + nMapY * nW] & 0XFF000000){
                                pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                            }
                        }
                    }
                }
            }
        }
        return pSData;
    }else{
        // this logic is much complicated, because of slope of (P1---P2)
        // which makes something prohibit us from applying previous logic
        //
        // nY1 > nY2
        //
        // +----+--+-------+ (V1)
        // |    |  |       |
        // |(A1)|A2| (A3)  |
        // |    |  |       |
        // |    |  +-------+
        // |    | /(P2)    |
        // |    |/         |
        // +----+          |
        // |   (P1)        |
        // |          (A4) |
        // +---------------+
        // pixel in A123 will create shadow
        // we drop pixel in A4

        int nNewW = nY2 / 2 + nW;
        int nNewH = nY2 / 2 + nY1 - nY2;

        nSW = nNewW;
        nSH = nNewH;

        auto pSData = new uint32_t[nNewW * nNewH];
        std::memset(pSData, 0, nNewW * nNewH * sizeof(uint32_t));
        for(int nYCnt = 0; nYCnt < nNewH; ++nYCnt){
            for(int nXCnt = 0; nXCnt < nNewW; ++nXCnt){
                if(true
                        && (nYCnt + nY2 / 2 - nY2) > 0
                        && (nYCnt + nY2 / 2 - nY1) * (nX2 - nX1) > (nY2 - nY1) * (nXCnt - nX1)){
                    continue;
                }
                if(nX1 <= nX2 - (nY1 - nY2)){
                    // normal, easy to use previous logic
                    int  nMapX, nMapY;
                    int  nCrossX = nXCnt - (nNewH - nYCnt);
                    bool bMapped = false;
                    bool bMiddle = false;

                    if(nX2 - (nY1 - nY2) <= nCrossX && nCrossX < nW - (nY1 - nY2)){
                        nMapX   = nCrossX + (nY1 - nY2);
                        nMapY   = nY2 - 2 * (nY2 / 2 - nYCnt);
                        bMapped = true;
                        bMiddle = false;
                    }else if(nX1 <= nCrossX && nCrossX <  nX2 - (nY1 - nY2)){
                        double dRatio = 1.0 *
                            (nCrossX - nX1) / (std::max)(1, (nX2 - (nY1 - nY2)) - nX1);
                        dRatio = (std::max)(0.0, (std::min)(dRatio, 1.0));
                        int nRealCrossX = nX1 + std::lround((nX2 - nX1) * dRatio);
                        int nRealCrossY = nY2 + std::lround((nY1 - nY2) * (1.0 - dRatio));
                        nMapX   = nRealCrossX;
                        nMapY   = nRealCrossY - 2 * (nRealCrossY - (nYCnt + nY2 / 2));
                        bMapped = true;
                        bMiddle = true;
                    }else if(0 <= nCrossX && nCrossX < nX1){
                        nMapX   = nCrossX;
                        nMapY   = nY1 - 2 * (nNewH - nYCnt);
                        bMapped = true;
                        bMiddle = false;
                    }else{
                        bMapped = false;
                        bMiddle = false;
                    }

                    if(bMapped){
                        int nExpandCount = 0;
                        if(bMiddle){
                            double dSlope = 1.0 * (nY1 - nY2) / (std::max)(1, nX2 - nX1);
                            nExpandCount  = std::lround(1.4 * (std::max)(0.0, dSlope) + 1.2);
                        }else{
                            nExpandCount = 1;
                        }

                        { // expand with proper expand count
                            for(int nVX = -nExpandCount; nVX <= nExpandCount; ++nVX){
                                for(int nVY = -nExpandCount; nVY <= nExpandCount; ++nVY){
                                    int nMapXX = nMapX + nVX;
                                    int nMapYY = nMapY + nVY;
                                    if(nMapXX >= 0 && nMapXX < nW && nMapYY >= 0 && nMapYY < nH){
                                        if(pData[nMapXX + nMapYY * nW] & 0XFF000000){
                                            pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                                            goto __TwoPointShadowDecode_Pixel_Set2;
                                        }
                                    }
                                }
                            }
__TwoPointShadowDecode_Pixel_Set2: ;
                        }
                    }
                }else{
                    // nX1 > nX2 - (nY1 - nY2)
                    // the 45 line will cross with three point in the middle part !!!
                    //
                    // still we need nCrossX
                    int nCrossX = nXCnt - (nNewH - nYCnt);
                    if(nX2 - (nY1 - nY2) <= nCrossX && nCrossX < nX1){
                        // three cross point
                        int nMapX[3], nMapY[3];
                        nMapX[0] = nCrossX;
                        nMapY[0] = nY1 - 2 * (nNewH - nYCnt);
                        nMapX[2] = nCrossX + (nY1 - nY2);
                        nMapY[2] = nY2 - 2 * (nY2 / 2 - nYCnt);
                        { // middle point, since nX1 > (nX2 - (nY1 - nY2)) here, skip max(1, x)
                            double dRatio = 1.0 *
                                (nCrossX - (nX2 - (nY1 - nY2))) / (nX1 - (nX2 - (nY1 - nY2)));
                            int nRealCrossX = nX1 + std::lround((nX2 - nX1) * (1.0 - dRatio));
                            int nRealCrossY = nY2 + std::lround((nY1 - nY2) * dRatio);
                            nMapX[1] = nRealCrossX;
                            nMapY[1] = nRealCrossY - 2 * (nRealCrossY - (nYCnt + nY2 / 2));
                        }
                        { // check three mapping points
                            for(int nIndex = 0; nIndex < 3; ++nIndex){
                                for(int nVX = -1; nVX <= 1; ++nVX){
                                    for(int nVY = -1; nVY <= 1; ++nVY){
                                        int nMapXX = nMapX[nIndex] + nVX;
                                        int nMapYY = nMapY[nIndex] + nVY;
                                        if(nMapXX >= 0 && nMapXX < nW && nMapYY >= 0 && nMapYY < nH){
                                            if(pData[nMapXX + nMapYY * nW] & 0XFF000000){
                                                pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                                                goto __TwoPointShadowDecode_Pixel_Set3;
                                            }
                                        }
                                    }
                                }
                            }
__TwoPointShadowDecode_Pixel_Set3: ;
                        }
                    }else{
                        int  nMapX   = -100;
                        int  nMapY   = -100;
                        bool bMapped =  false;
                        if(nX1 <= nCrossX && nCrossX < nW - (nY1 - nY2)){
                            nMapX   = nCrossX + (nY1 - nY2);
                            nMapY   = nY2 - 2 * (nY2 / 2 - nYCnt);
                            bMapped = true;
                        }else if(0 <= nCrossX && nCrossX < nX2 - (nY1 - nY2)){
                            // TODO maybe 0 > nX2 - (nY1 - nY2)
                            nMapX   = nCrossX;
                            nMapY   = nY1 - 2 * (nNewH - nYCnt);
                            bMapped = true;
                        }
                        if(bMapped){
                            { // check only one mapping point, fixedly expand to 3
                                for(int nVX = -1; nVX <= 1; ++nVX){
                                    for(int nVY = -1; nVY <= 1; ++nVY){
                                        int nMapXX = nMapX + nVX;
                                        int nMapYY = nMapY + nVY;
                                        if(nMapXX >= 0 && nMapXX < nW && nMapYY >= 0 && nMapYY < nH){
                                            if(pData[nMapXX + nMapYY * nW] & 0XFF000000){
                                                pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                                                goto __TwoPointShadowDecode_Pixel_Set4;
                                            }
                                        }
                                    }
                                }
__TwoPointShadowDecode_Pixel_Set4: ;
                            }
                        }
                    }
                }
            }
        }
        return pSData;
    }
    return nullptr;
}
