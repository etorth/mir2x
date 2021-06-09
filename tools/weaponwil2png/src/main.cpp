/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *    Description: convert hero graphics res to png files, usage:
 *
 *                      weaponwil2png gender index                                  \
 *                          hero-wil-path   hero-wil-basename   hero-wil-extension  \
 *                        weapon-wil-path weapon-wil-basename weapon-wil-extension  weapon-output-dir
 *
 *                 this command has a really long parameter list
 *
 *                      1. it needs hero   gfx resource
 *                      2. it needs weapon gfx resource as expected
 *
 *                 parameters:
 *                      gender   : 1 :   male
 *                                 0 : female
 *                      index    : 1 : [w]m-weapon1.wil
 *                                 2 : [w]m-weapon2.wil
 *                                 3 : [w]m-weapon3.wil
 *                                 4 : [w]m-weapon4.wil
 *                                 5 : [w]m-weapon5.wil
 *
 *                      pathInfo : path
 *                               : basename
 *                               : extension
 *
 *                      out-dir  : output folder
 *                              
 *                  path-to-package/basename.extension should exist, i.e.
 *
 *                      weaponwil2png 0 1 /home/you WM-Hum wil /home/you wm-weapon1 wil /home/you/out
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
#include "shadow.hpp"
#include "wilimagepackage.hpp"

void printUsage()
{
    std::printf("%s",
            "Usage: convert hero graphics res to png files, usage:\n"
            "       weaponwil2png gender index                                  \\ \n"
            "           hero-wil-path   hero-wil-basename   hero-wil-extension  \\ \n"
            "         weapon-wil-path weapon-wil-basename weapon-wil-extension  weapon-output-dir\n"
            "  this command has a really long parameter list\n"
            "       1. it needs hero   gfx resource\n"
            "       2. it needs weapon gfx resource as expected\n"
            "  parameters:\n"
            "       gender   : 1 :   male\n"
            "                  0 : female\n"
            "       index    : 1 : [w]m-weapon1.wil\n"
            "                  2 : [w]m-weapon2.wil\n"
            "                  3 : [w]m-weapon3.wil\n"
            "                  4 : [w]m-weapon4.wil\n"
            "                  5 : [w]m-weapon5.wil\n"
            "       pathInfo : path\n"
            "                : basename\n"
            "                : extension\n"
            "       out-dir  : output folder\n"
            "               \n"
            "   path-to-package/basename.extension should exist, i.e.\n"
            "       weaponwil2png 0 1 /home/you WM-Hum wil /home/you wm-weapon1 wil /home/you/out\n"
            "   otherwise get error\n");
}

const char *createOffsetFileName(char *szFileName,
        const char *szOutDir,
        bool bShadow,
        bool bGender,
        int  nWeapon,
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
        uint32_t nEncodeDress     = nWeapon;
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

bool weaponWil2PNG(bool bGender, int nIndex,
        const char *szHeroWilPath,
        const char *szHeroWilBaseName,
        const char *,
        const char *szWeaponWilPath,
        const char *szWeaponWilBaseName,
        const char *,
        const char *szOutDir)
{
    WilImagePackage stHeroWilPackage(szHeroWilPath, szHeroWilBaseName);
    WilImagePackage stWeaponWilPackage(szWeaponWilPath, szWeaponWilBaseName);

    std::vector<uint32_t> stWeaponPNGBuf;
    std::vector<uint32_t> stWeaponPNGBufShadow;

    for(int nWeapon = 0; nWeapon < 10; ++nWeapon){
        for(int nMotion = 0; nMotion < 33; ++nMotion){
            for(int nDirection = 0; nDirection < 8; ++nDirection){
                for(int nFrame = 0; nFrame < 10; ++nFrame){

                    bool bProject = true;
                    if(true
                            && nMotion == 19
                            && nFrame  ==  9){ bProject = false; }

                    int   nHeroIndex =       0 * 3000 + nMotion * 80 + nDirection * 10 + nFrame + 1;
                    int nWeaponIndex = nWeapon * 3000 + nMotion * 80 + nDirection * 10 + nFrame + 1;

                    if(stHeroWilPackage.setIndex(nHeroIndex) && stWeaponWilPackage.setIndex(nWeaponIndex)){
                        const auto heroImgInfo   =   stHeroWilPackage.currImageInfo();
                        const auto weaponImgInfo = stWeaponWilPackage.currImageInfo();

                        stWeaponPNGBuf.resize(weaponImgInfo->width * weaponImgInfo->height);
                        stWeaponWilPackage.decode(stWeaponPNGBuf.data(), 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);

                        // make a buffer to hold the shadow as needed
                        // shadow buffer size depends on do project or not
                        //
                        //  project :  (nW + nH / 2) x (nH / 2 + 1)
                        //          :  (nW x nH)
                        //

                        int nMaxShadowW = (std::max<int>)(weaponImgInfo->width + weaponImgInfo->height / 2, weaponImgInfo->width ) + 20;
                        int nMaxShadowH = (std::max<int>)(                 1 + weaponImgInfo->height / 2, weaponImgInfo->height) + 20;
                        stWeaponPNGBufShadow.resize(nMaxShadowW * nMaxShadowH);

                        int nShadowW = 0;
                        int nShadowH = 0;
                        Shadow::MakeShadow(
                                &(stWeaponPNGBufShadow[0]),
                                bProject,
                                stWeaponPNGBuf.data(),
                                weaponImgInfo->width,
                                weaponImgInfo->height,
                                &nShadowW,
                                &nShadowH,
                                0XFF000000);

                        char szSaveFileName[128];
                        createOffsetFileName(szSaveFileName,
                                szOutDir,
                                false,
                                bGender,
                                (nIndex - 1) * 10 + nWeapon,
                                nMotion,
                                nDirection,
                                nFrame,
                                weaponImgInfo->px,
                                weaponImgInfo->py);

                        if(!pngf::saveRGBABuffer(stWeaponPNGBuf.data(), weaponImgInfo->width, weaponImgInfo->height, szSaveFileName)){
                            std::printf("save weapon PNG failed: %s", szSaveFileName);
                            return false;
                        }

                        // understand how I get it:
                        // two coords: origin at left-top    : coord-1
                        //             origin at left-bottom : coord-2
                        //
                        // 1. every frame consists of body image and weapon image, in coord-1: (X0, Y0) and (X1, Y1)
                        // 2. these two images construct a bigger image, call it combined image, take (W, H) as its size
                        // 3. then take origin of coord-2 at the left-bottom of the combined image
                        // 4. (X0, Y0) -> (X0, H - Y0)
                        //    (X1, Y1) -> (X1, H - Y1)
                        // 5. any point (x, y) in coord-2 will project to (x + y / 2, 1 + y / 2)
                        //    then calculate the projectioin of start point of body image and weapon image in coord-2
                        // 6. real start point of the projected images has a (y / 2) shift to the left
                        //
                        //     p   q
                        //     +---+----+      p : real start point of projected image
                        //     |  /    /       q :      start point of projected image
                        //     | /    /
                        //     +-----+
                        // 7. convert back to coord-1 of ``real start point" of shadow images
                        // 8. offset of ``real start point" of body shadow image is given, then we calculate for weapon shadow image
                        // 9. this method works pretty good!

                        int nWeaponDX = heroImgInfo->shadowPX + (weaponImgInfo->px - heroImgInfo->px) - (weaponImgInfo->py - heroImgInfo->py) / 2 - (weaponImgInfo->height - heroImgInfo->height) / 2;
                        int nWeaponDY = heroImgInfo->shadowPY + (weaponImgInfo->py - heroImgInfo->py) / 2;

                        if(true
                                && nShadowW > 0
                                && nShadowH > 0){
                            createOffsetFileName(szSaveFileName,
                                    szOutDir,
                                    true,
                                    bGender,
                                    (nIndex - 1) * 10 + nWeapon,
                                    nMotion,
                                    nDirection,
                                    nFrame,
                                    bProject ? (nWeaponDX) : (weaponImgInfo->px + 3),
                                    bProject ? (nWeaponDY) : (weaponImgInfo->py + 2));

                            if(!pngf::saveRGBABuffer((uint8_t *)(&(stWeaponPNGBufShadow[0])), nShadowW, nShadowH, szSaveFileName)){
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
    if(argc != 10){
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

    if(true
            && std::strcmp(argv[2], "1")
            && std::strcmp(argv[2], "2")
            && std::strcmp(argv[2], "3")
            && std::strcmp(argv[2], "4")
            && std::strcmp(argv[2], "5")){
        std::printf("invlid argv[2] : %s\n", argv[2]);
        printUsage();
        return 1;
    }

    return weaponWil2PNG((std::atoi(argv[1]) != 0), std::atoi(argv[2]), argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]);
}
