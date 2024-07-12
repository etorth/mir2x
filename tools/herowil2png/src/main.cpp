// convert hero graphics res to png files, usage:
//
//      herowil2png gender path-to-package basename extension out-dir
//
// parameters:
//      gender   : 1 :    male
//                 0 :  female
//
//      pathInfo : path
//               : basename
//               : extension
//
//      out-dir  : output folder
//
// path-to-package/basename.extension should exist, i.e.
//
//      wil2png 0 /home/you WM-Hero wil /home/you/out
//
// otherwise get error

#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cinttypes>
#include <algorithm>

#include "imgf.hpp"
#include "alphaf.hpp"
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
        bool layerIndex,
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
        uint32_t nEncodeLayerIdx  = layerIndex ? 1 : 0;
        uint32_t nEncodeShadow    = bShadow ? 1 : 0;
        uint32_t nEncodeGender    = bGender ? 1 : 0;
        uint32_t nEncodeDress     = nDress;
        uint32_t nEncodeMotion    = nMotion;
        uint32_t nEncodeDirection = nDirection;
        uint32_t nEncodeFrame     = nFrame;
        uint32_t nEncode = 0
            | (nEncodeLayerIdx  << 24)
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
        const char *,
        const char *szOutDir)
{
    std::vector<uint32_t> pngShadowBuf;
    WilImagePackage stPackage(szPath, szBaseName);

    for(int nDress = 0; nDress < 9; ++nDress){
        for(int nMotion = 0; nMotion < 33; ++nMotion){
            for(int nDirection = 0; nDirection < 8; ++nDirection){
                for(int nFrame = 0; nFrame < 10; ++nFrame){
                    const int nBaseIndex = nDress * 3000 + nMotion * 80 + nDirection * 10 + nFrame;
                    if(stPackage.setIndex(nBaseIndex)){

                        const auto imgInfo = stPackage.currImageInfo();
                        const auto layer = stPackage.decode(false, false, false);

                        // export for HumanGfxDBN
                        for(const int layerIndex: {0, 1}){
                            if(layer[layerIndex]){
                                char szSaveFileName[128];
                                createOffsetFileName(szSaveFileName,
                                        szOutDir,
                                        layerIndex,
                                        false,
                                        bGender,
                                        nDress,
                                        nMotion,
                                        nDirection,
                                        nFrame,
                                        imgInfo->px,
                                        imgInfo->py);

                                if(!imgf::saveImageBuffer((uint8_t *)(layer[layerIndex]), imgInfo->width, imgInfo->height, szSaveFileName)){
                                    std::printf("save PNG failed: %s", szSaveFileName);
                                    return false;
                                }
                            }
                        }

                        // make a big buffer to hold the shadow as needed
                        // shadow buffer size depends on do project or not
                        //
                        //  project :  (nW + nH / 2) x (nH / 2 + 1)
                        //          :  (nW x nH)
                        //

                        bool bProject = true;
                        if(true
                                && nMotion == 19
                                && nFrame  ==  9){ bProject = false; }

                        const auto mergedLayer = stPackage.decode(true, false, false);
                        const auto [bufShadow, nShadowW, nShadowH] = alphaf::createShadow(pngShadowBuf, bProject, mergedLayer[0], imgInfo->width, imgInfo->height, colorf::A_SHF(0XFF));
                        if(true
                                && nShadowW > 0
                                && nShadowH > 0){
                            char szSaveFileName[128];
                            createOffsetFileName(szSaveFileName,
                                    szOutDir,
                                    0,
                                    true,
                                    bGender,
                                    nDress,
                                    nMotion,
                                    nDirection,
                                    nFrame,
                                    bProject ? imgInfo->shadowPX : (imgInfo->px + 3),
                                    bProject ? imgInfo->shadowPY : (imgInfo->py + 2));

                            if(!imgf::saveImageBuffer((uint8_t *)(bufShadow), nShadowW, nShadowH, szSaveFileName)){
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
