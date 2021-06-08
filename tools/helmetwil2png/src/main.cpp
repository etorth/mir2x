/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *    Description: convert helmet graphics res to png files, usage:
 *
 *                      helmetwil2png gender index \
 *                          helmet-wil-path        \
 *                          helmet-wil-basename    \
 *                          helmet-wil-extension   \
 *                          helmet-output-dir
 *
 *                 parameters:
 *                      gender   : 1 : male
 *                                 0 : female
 *                      index    : 1 : [W]M-Helmet1.wil
 *                                 2 : [W]M-Helmet2.wil
 *
 *                      pathInfo : path
 *                               : basename
 *                               : extension
 *
 *                      out-dir  : output folder
 *                              
 *                  path-to-package/basename.extension should exist, i.e.
 *
 *                      helmetwil2png 0 1 /home/you WM-Helmet wil /home/you/out
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
#include <algorithm>

#include "pngf.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"
#include "wilimagepackage.hpp"

void printUsage()
{
    std::printf("Please check repo mir2x/tools/helmetwil2png/src/main.cpp for usage.\n");
}

const char *createOffsetFileName(char *szFileName,
        const char *szOutDir,
        bool bGender,
        int  nHelmet,
        int  nMotion,
        int  nDirection,
        int  nFrame,
        int  nDX,
        int  nDY)
{
    if(szFileName){
        // refer to client/src/hero.cpp to get encoding strategy
        const uint32_t nEncodeGender    = bGender ? 1 : 0;
        const uint32_t nEncodeDress     = nHelmet;
        const uint32_t nEncodeMotion    = nMotion;
        const uint32_t nEncodeDirection = nDirection;
        const uint32_t nEncodeFrame     = nFrame;
        const uint32_t nEncode = 0
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

void helmetWil2PNG(bool bGender, int nIndex,
        const char *szHelmetWilPath,
        const char *szHelmetWilBaseName,
        const char *szHelmetWilExt,
        const char *szOutDir)
{
    WilImagePackage stHelmetWilPackage;
    if(!stHelmetWilPackage.Load(szHelmetWilPath, szHelmetWilBaseName, szHelmetWilExt)){
        throw fflerror("Load helmet wil file failed: %s/%s.%s", szHelmetWilPath, szHelmetWilBaseName, szHelmetWilExt);
    }

    std::vector<uint32_t> stHelmetPNGBuf;
    for(int nHelmet = 0; nHelmet < 10; ++nHelmet){
        for(int nMotion = 0; nMotion < 33; ++nMotion){
            for(int nDirection = 0; nDirection < 8; ++nDirection){
                for(int nFrame = 0; nFrame < 10; ++nFrame){

                    const int nHelmetIndex = nHelmet * 3000 + nMotion * 80 + nDirection * 10 + nFrame;
                    if(true
                            && stHelmetWilPackage.SetIndex(nHelmetIndex)
                            && stHelmetWilPackage.CurrentImageValid()){

                        const auto stHelmetInfo = stHelmetWilPackage.CurrentImageInfo();
                        stHelmetPNGBuf.resize(stHelmetInfo.width * stHelmetInfo.height);
                        stHelmetWilPackage.Decode(&(stHelmetPNGBuf[0]), 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);

                        char szSaveFileName[128];
                        createOffsetFileName(szSaveFileName,
                                szOutDir,
                                bGender,
                                (nIndex - 1) * 10 + nHelmet,
                                nMotion,
                                nDirection,
                                nFrame,
                                stHelmetInfo.px,
                                stHelmetInfo.py);

                        if(!pngf::saveRGBABuffer((uint8_t *)(&(stHelmetPNGBuf[0])), stHelmetInfo.width, stHelmetInfo.height, szSaveFileName)){
                            throw fflerror("save helmet PNG failed: %s", szSaveFileName);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc != 7){
        throw fflerror("Usage: hairwil2png gender hair-wil-path hair-wil-basename hair-wil-extension hair-output-dir");
    }

    if(true
            && std::strcmp(argv[1], "0")
            && std::strcmp(argv[1], "1")){
        throw fflerror("invlid argv[1] : %s", argv[1]);
    }

    if(true
            && std::strcmp(argv[2], "1")
            && std::strcmp(argv[2], "2")){
        throw fflerror("invlid argv[2] : %s", argv[2]);
    }

    helmetWil2PNG((std::atoi(argv[1]) != 0), std::atoi(argv[2]), argv[3], argv[4], argv[5], argv[6]);
    return 0;
}
