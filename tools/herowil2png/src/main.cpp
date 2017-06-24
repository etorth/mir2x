/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *  Last Modified: 06/22/2017 23:29:17
 *
 *    Description: convert hero graphics res to png files, usage:
 *
 *                      herowil2png gender path-to-package basename extension out-dir
 *
 *                 parameters:
 *                      gender   : 1 :    male
 *                                 0 :  female
 *
 *                      pathInfo : path
 *                               : basename
 *                               : extension
 *
 *                      out-dir  : output folder
 *                              
 *                  path-to-package/basename.extension should exist, i.e.
 *
 *                      wil2png 0 /home/you WM-Hero wil /home/you/out
 *
 *                  otherwise get error
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

#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cinttypes>

#include "shadow.hpp"
#include "savepng.hpp"
#include "filesys.hpp"
#include "wilimagepackage.hpp"

void printUsage()
{
    const char *szUsage = 
        "Usage: convert hero graphics res to png files, work as:\n"
        "            wil2png gender path-to-package basename extension out-dir\n"
        "       parameters:\n"
        "            gender   : 1 :    male\n"
        "                       0 :  female\n"
        "            pathInfo : path\n"
        "                     : basename\n"
        "                     : extension\n"
        "            out-dir  : output folder\n"
        "        path-to-package/basename.extension should exist, i.e.\n"
        "            wil2png 0 /home/you WM-Hero wil /home/you/out\n"
        "        otherwise get error\n";
    std::printf("%s", szUsage);
}

const char *createOffsetFileName(char *szFileName,
        const char *szOutDir,
        bool bShadow,
        bool bGender,
        int  nDress,
        int  nMotion,
        int  nDirection,
        int  nFrame,
        int  nDX,
        int  nDY)
{
    if(szFileName){
        // refer to client/src/hero.cpp to get encoding strategy
        uint32_t nEncodeShadow    = bShadow ? 1 : 0;
        uint32_t nEncodeGender    = bGender ? 1 : 0;
        uint32_t nEncodeDress     = nDress;
        uint32_t nEncodeMotion    = nMotion;
        uint32_t nEncodeDirection = nDirection;
        uint32_t nEncodeFrame     = nFrame;
        uint32_t nEncode = 0
            | (nEncodeShadow    << 23)
            | (nEncodeGender    << 22)
            | (nEncodeDress     << 14)
            | (nEncodeMotion    <<  8)
            | (nEncodeDirection <<  5)
            | (nEncodeFrame     <<  0);

        std::sprintf(szFileName, "%s/%08" PRIX32 "%s%s%04X%04X.PNG",
                szOutDir,
                nEncode, 
                ((nDX > 0) ? "1" : "0"),
                ((nDY > 0) ? "1" : "0"),
                std::abs(nDX),
                std::abs(nDY));
    }
    return szFileName;
}

bool heroWil2PNG(bool bGender,
        const char *szPath,
        const char *szBaseName,
        const char *szExt,
        const char *szOutDir)
{
    WilImagePackage stPackage;
    if(!stPackage.Load(szPath, szBaseName, szExt)){
        std::printf("Load wil file failed: %s/%s/%s", szPath, szBaseName, szExt);
        return false;
    }

    std::vector<uint32_t> stPNGBuf;
    std::vector<uint32_t> stPNGBufShadow;
    for(int nDress = 0; nDress < 8; ++nDress){
        for(int nMotion = 0; nMotion < 33; ++nMotion){
            for(int nDirection = 0; nDirection < 8; ++nDirection){
                for(int nFrame = 0; nFrame < 10; ++nFrame){
                    int nBaseIndex = nDress * 3000 + nMotion * 80 + nDirection * 10 + nFrame + 1;
                    if(true
                            && stPackage.SetIndex(nBaseIndex)
                            && stPackage.CurrentImageValid()){

                        auto stInfo = stPackage.CurrentImageInfo();
                        stPNGBuf.resize(stInfo.shWidth * stInfo.shHeight);
                        stPackage.Decode(&(stPNGBuf[0]), 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);

                        // export for HumanGfxDBN
                        char szSaveFileName[128];
                        createOffsetFileName(szSaveFileName,
                                szOutDir,
                                false,
                                bGender,
                                nDress,
                                nMotion,
                                nDirection,
                                nFrame,
                                stInfo.shPX,
                                stInfo.shPY);

                        if(!SaveRGBABufferToPNG((uint8_t *)(&(stPNGBuf[0])), stInfo.shWidth, stInfo.shHeight, szSaveFileName)){
                            std::printf("save PNG failed: %s", szSaveFileName);
                            return false;
                        }

                        // make a big buffer to hold the shadow as needed
                        // shadow buffer size depends on do project or not
                        //
                        //  project :  (nW + nH / 2) x (nH / 2 + 1)
                        //          :  (nW x nH)
                        //
                        int nMaxW = std::max<int>(stInfo.shWidth + stInfo.shHeight / 2, stInfo.shWidth ) + 20;
                        int nMaxH = std::max<int>(             1 + stInfo.shHeight / 2, stInfo.shHeight) + 20;
                        stPNGBufShadow.resize(nMaxW * nMaxH);

                        bool bProject = true;
                        if(true
                                && nMotion == 19
                                && nFrame  ==  9){ bProject = false; }

                        int nShadowW = 0;
                        int nShadowH = 0;
                        Shadow::MakeShadow(&(stPNGBufShadow[0]), bProject, &(stPNGBuf[0]), stInfo.shWidth, stInfo.shHeight, &nShadowW, &nShadowH, 0XFF000000);

                        if(true
                                && nShadowW > 0
                                && nShadowH > 0){
                            createOffsetFileName(szSaveFileName,
                                    szOutDir,
                                    true,
                                    bGender,
                                    nDress,
                                    nMotion,
                                    nDirection,
                                    nFrame,
                                    bProject ? stInfo.shShadowPX : (stInfo.shPX + 3),
                                    bProject ? stInfo.shShadowPY : (stInfo.shPY + 2));

                            if(!SaveRGBABufferToPNG((uint8_t *)(&(stPNGBufShadow[0])), nShadowW, nShadowH, szSaveFileName)){
                                std::printf("save shadow PNG failed: %s", szSaveFileName);
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    // check arguments
    if(argc != 6){
        printUsage();
        return 1;
    }

    if(true
            && std::strcmp(argv[1], "0")
            && std::strcmp(argv[1], "1")){
        std::printf("invlid argv[1] : %s\n", argv[1]);
        printUsage();
        return 1;
    }

    return heroWil2PNG((std::atoi(argv[1]) != 0), argv[2], argv[3], argv[4], argv[5]);
}
