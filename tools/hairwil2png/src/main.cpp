/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *    Description: convert hair graphics res to png files, usage:
 *
 *                     hairwil2png gender hair-wil-path hair-wil-basename hair-wil-extension hair-output-dir
 *
 *                 parameters:
 *                      gender   : 1 :   male
 *                                 0 : female
 *
 *                      pathInfo : path
 *                               : basename
 *                               : extension
 *
 *                      out-dir  : output folder
 *                              
 *                  path-to-package/basename.extension should exist, i.e.
 *
 *                      hairwil2png 0 /home/you WM-Hair wil /home/you/out
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

#include "imgf.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"
#include "wilimagepackage.hpp"

void printUsage()
{
    std::printf("Please check repo mir2x/tools/hairwil2png/src/main.cpp for usage.\n");
}

const char *createOffsetFileName(char *szFileName,
        const char *szOutDir,
        bool bGender,
        int  nHair,
        int  nMotion,
        int  nDirection,
        int  nFrame,
        int  nDX,
        int  nDY)
{
    if(szFileName){
        // refer to client/src/hero.cpp to get encoding strategy
        const uint32_t nEncodeGender    = bGender ? 1 : 0;
        const uint32_t nEncodeHair      = nHair;
        const uint32_t nEncodeMotion    = nMotion;
        const uint32_t nEncodeDirection = nDirection;
        const uint32_t nEncodeFrame     = nFrame;
        const uint32_t nEncode = 0
            | (nEncodeGender    << 22)
            | (nEncodeHair      << 14)
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

void hairWil2PNG(bool bGender,
        const char *szHairWilPath,
        const char *szHairWilBaseName,
        const char *,
        const char *szOutDir)
{
    WilImagePackage stHairWilPackage(szHairWilPath, szHairWilBaseName);
    for(int nHair = 0; nHair < 10; ++nHair){ // acturally only 0 ~ 4 valid
        for(int nMotion = 0; nMotion < 33; ++nMotion){
            for(int nDirection = 0; nDirection < 8; ++nDirection){
                for(int nFrame = 0; nFrame < 10; ++nFrame){

                    const int nHairIndex = nHair * 3000 + nMotion * 80 + nDirection * 10 + nFrame + 0;
                    if(stHairWilPackage.setIndex(nHairIndex)){

                        const auto hairImgInfo = stHairWilPackage.currImageInfo();
                        const auto layer = stHairWilPackage.decode(true, false, false);

                        char szSaveFileName[128];
                        createOffsetFileName(szSaveFileName,
                                szOutDir,
                                bGender,
                                nHair,
                                nMotion,
                                nDirection,
                                nFrame,
                                hairImgInfo->px,
                                hairImgInfo->py);

                        if(!imgf::saveImageBuffer((uint8_t *)(layer[0]), hairImgInfo->width, hairImgInfo->height, szSaveFileName)){
                            throw fflerror("save hair PNG failed: %s", szSaveFileName);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc != 6){
        throw fflerror("Usage: hairwil2png gender hair-wil-path hair-wil-basename hair-wil-extension hair-output-dir");
    }

    if(true
            && std::strcmp(argv[1], "0")
            && std::strcmp(argv[1], "1")){
        throw fflerror("invlid argv[1] : %s", argv[1]);
    }

    hairWil2PNG((std::atoi(argv[1]) != 0), argv[2], argv[3], argv[4], argv[5]);
    return 0;
}
