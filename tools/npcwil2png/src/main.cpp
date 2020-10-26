/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *    Description: convert npc graphics res to png files, usage:
 *
 *                      npcwil2png path-to-package basename extension out-dir
 *
 *                 parameters:
 *                      pathInfo : path
 *                               : basename
 *                               : extension
 *
 *                      out-dir  : output folder
 *                              
 *                  path-to-package/basename.extension should exist, i.e.
 *
 *                      wil2png /home/you WM-Hero wil /home/you/out
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

#include <map>
#include <array>
#include <vector>
#include <string>
#include <cstdio>
#include <utility>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include "pngf.hpp"
#include "strf.hpp"
#include "shadow.hpp"
#include "motion.hpp"
#include "filesys.hpp"
#include "protocoldef.hpp"
#include "wilimagepackage.hpp"

void printUsage()
{
    const char *usage = 
        "Usage: convert npc graphics res to png files, work as:\n"
        "            wil2png path-to-package basename extension out-dir\n"
        "       parameters:\n"
        "            pathInfo : path\n"
        "                     : basename\n"
        "                     : extension\n"
        "            out-dir  : output folder\n"
        "        path-to-package/basename.extension should exist, i.e.\n"
        "            wil2png /home/you WM-Hero wil /home/you/out\n"
        "        otherwise get error\n";
    std::printf("%s", usage);
}

std::string createOffsetFileName(const char *outDir, bool shadow, int look, int motion, int direction, int frame, int dx, int dy)
{
    const uint32_t encodeShadow    = shadow ? 1 : 0;
    const uint32_t encodeLookID    = look;
    const uint32_t encodeMotion    = motion;
    const uint32_t encodeDirection = direction;
    const uint32_t encodeFrame     = frame;
    const uint32_t encode = 0
        | (encodeShadow    << 23)
        | (encodeLookID    << 12)
        | (encodeMotion    <<  8)
        | (encodeDirection <<  5)
        | (encodeFrame     <<  0);

    return str_printf("%s/%08llX%s%s%04X%04X.PNG",
            outDir,
            to_llu(encode),
            ((dx > 0) ? "1" : "0"),
            ((dy > 0) ? "1" : "0"),
            std::abs(dx),
            std::abs(dy));
}

void npcWil2PNG(const char *path, const char *baseName, const char *fileExt, const char *outDir)
{
    WilImagePackage package;
    if(!package.Load(path, baseName, fileExt)){
        throw fflerror("Load wil file failed: %s/%s/%s", path, baseName, fileExt);
    }

    std::vector<uint32_t> pngBuf;
    std::vector<uint32_t> pngBufShadow;

    struct frameSeq
    {
        int start = 0;
        int frameCount = 0;
    };

    const std::map<int, std::map<std::array<int, 2>, frameSeq>> npcGfxType
    {
        {0, // default type
            {
                {{DIR_DOWNRIGHT, MOTION_NPC_STAND}, { 0,  4}},
                {{DIR_DOWN     , MOTION_NPC_STAND}, {10,  4}},
                {{DIR_DOWNLEFT , MOTION_NPC_STAND}, {20,  4}},
                {{DIR_DOWNRIGHT, MOTION_NPC_ACT  }, {30, 10}},
                {{DIR_DOWN     , MOTION_NPC_ACT  }, {40, 10}},
                {{DIR_DOWNLEFT , MOTION_NPC_ACT  }, {50, 10}},
            }
        },

        {1,
            {
                {{DIR_DOWNRIGHT, MOTION_NPC_STAND}, { 0,  4}},
                {{DIR_DOWN     , MOTION_NPC_STAND}, {10,  4}},
                {{DIR_DOWNLEFT , MOTION_NPC_STAND}, {20,  4}},
            }
        },

        {2,
            {
                {{DIR_DOWNRIGHT, MOTION_NPC_STAND }, { 0,  4}},
                {{DIR_DOWN     , MOTION_NPC_STAND }, {10,  4}},
                {{DIR_DOWNLEFT , MOTION_NPC_STAND }, {20,  4}},
                {{DIR_DOWNRIGHT, MOTION_NPC_ACT   }, {30,  6}},
                {{DIR_DOWN     , MOTION_NPC_ACT   }, {40,  6}},
                {{DIR_DOWNLEFT , MOTION_NPC_ACT   }, {50,  6}},
                {{DIR_DOWNRIGHT, MOTION_NPC_ACTEXT}, {60,  9}},
            }
        },

        {3,
            {
                {{DIR_DOWNRIGHT, MOTION_NPC_STAND }, { 0,  4}},
                {{DIR_DOWN     , MOTION_NPC_STAND }, {10,  4}},
                {{DIR_DOWNLEFT , MOTION_NPC_STAND }, {20,  4}},
                {{DIR_DOWNRIGHT, MOTION_NPC_ACT   }, {30,  9}},
                {{DIR_DOWN     , MOTION_NPC_ACT   }, {40,  9}},
                {{DIR_DOWNLEFT , MOTION_NPC_ACT   }, {50,  9}},
                {{DIR_DOWNRIGHT, MOTION_NPC_ACTEXT}, {60,  6}},
                {{DIR_DOWN     , MOTION_NPC_ACTEXT}, {70,  6}},
                {{DIR_DOWNLEFT , MOTION_NPC_ACTEXT}, {80,  6}},
            }
        },

        {4,
            {
                {{DIR_DOWNRIGHT, MOTION_NPC_STAND }, { 0,  4}},
                {{DIR_DOWNRIGHT, MOTION_NPC_ACT   }, {30, 10}},
            }
        },

        {5,
            {
                {{DIR_UPLEFT ,   MOTION_NPC_STAND}, { 0,  4}},
                {{DIR_UP     ,   MOTION_NPC_STAND}, {10,  4}},
                {{DIR_UPRIGHT,   MOTION_NPC_STAND}, {20,  4}},
            }
        },

        {6,
            {
                {{DIR_UP     ,   MOTION_NPC_STAND}, { 0,  4}},
                {{DIR_UPRIGHT,   MOTION_NPC_STAND}, {10,  4}},
                {{DIR_UPLEFT ,   MOTION_NPC_STAND}, {20,  4}},
            }
        },

        {7,
            {
                {{DIR_DOWNLEFT,  MOTION_NPC_STAND}, { 0,  1}},
            }
        },

        {8,
            {
                {{DIR_DOWNRIGHT, MOTION_NPC_STAND}, { 0,  1}},
            }
        },

        {9,
            {
                {{DIR_DOWNRIGHT, MOTION_NPC_STAND}, { 0,  12}},
            }
        },

        {10,
            {
                {{DIR_DOWNRIGHT, MOTION_NPC_STAND }, { 0,  4}},
                {{DIR_DOWN     , MOTION_NPC_STAND }, {10,  4}},
                {{DIR_DOWNLEFT , MOTION_NPC_STAND }, {20,  4}},
                {{DIR_DOWNRIGHT, MOTION_NPC_ACT   }, {30,  6}},
                {{DIR_DOWN     , MOTION_NPC_ACT   }, {40,  6}},
                {{DIR_DOWNLEFT , MOTION_NPC_ACT   }, {50,  6}},
            }
        },

        {11,
            {
                {{DIR_DOWN,      MOTION_NPC_STAND }, { 0,  1}},
            }
        },

        {12,
            {
                {{DIR_DOWN,      MOTION_NPC_STAND }, { 0, 10}},
            }
        },

        {13,
            {
                {{DIR_DOWN,      MOTION_NPC_STAND }, { 0,  4}},
            }
        },

        {14,
            {
                {{DIR_DOWNLEFT,  MOTION_NPC_STAND }, { 0,  4}},
            }
        },
    };

    auto fnNPCGfxType = [](int lookId) -> int
    {
        if(lookId >=  0 && lookId <= 18) return  0;
        if(lookId == 19)                 return 14;
        if(lookId >= 20 && lookId <= 23) return  0;
        if(lookId >= 24 && lookId <= 25) return  1;
        if(lookId == 26)                 return  0;
        if(lookId == 27)                 return  2;
        if(lookId == 28)                 return  3;
        if(lookId >= 29 && lookId <= 33) return  0;
        if(lookId >= 34 && lookId <= 35) return  1;
        if(lookId >= 36 && lookId <= 39) return  0;
        if(lookId == 40)                 return  4; 
        if(lookId == 41)                 return -1; // no this lookId
        if(lookId == 42)                 return  0;
        if(lookId == 43)                 return  1;
        if(lookId == 44)                 return  5;
        if(lookId >= 45 && lookId <= 49) return  1;
        if(lookId == 50)                 return  6;
        if(lookId == 51)                 return  7;
        if(lookId >= 52 && lookId <= 53) return  8;
        if(lookId == 54)                 return  7;
        if(lookId == 55)                 return  8;
        if(lookId >= 56 && lookId <= 57) return  9;
        if(lookId == 58)                 return  1;
        if(lookId == 59)                 return  7;
        if(lookId >= 60 && lookId <= 63) return  0;
        if(lookId >= 64 && lookId <= 65) return  8;
        if(lookId >= 66 && lookId <= 68) return  0;
        if(lookId == 69)                 return 10;
        if(lookId == 70)                 return 11;
        if(lookId == 71)                 return  7;
        if(lookId == 72)                 return  8;
        if(lookId == 73)                 return  7;
        if(lookId == 74)                 return  7;
        if(lookId == 75)                 return  8;
        if(lookId == 76)                 return  1;
        if(lookId >= 77 && lookId <= 79) return  0;
        if(lookId == 80)                 return  1;
        if(lookId == 81)                 return  0;
        if(lookId == 82)                 return  1;
        if(lookId >= 83 && lookId <= 86) return  1;
        if(lookId == 87)                 return 12;
        if(lookId >= 88 && lookId <= 89) return 13;
        if(lookId == 90)                 return  1;
        return -1;
    };

    for(int lookId = 0; lookId <= 90; ++lookId){
        const int gfxType = fnNPCGfxType(lookId);
        if(gfxType < 0){
            continue;
        }

        if(npcGfxType.count(gfxType) == 0){
            throw fflerror("can't find gfx for gfxType %d", gfxType);
        }

        // NPC don't respect DIR_XXX
        // we use dir_0, dir_1, dir2 ... to arrange gfx
        // becase many NPC don't have direction concept, and NPC won't turn direction

        const auto &frameSeqMap = npcGfxType.at(gfxType);
        const auto dirMap = [&frameSeqMap]()
        {
            std::map<int, int> dirMap;
            for(const auto &p: frameSeqMap){
                const int dir = p.first.at(0);
                if(!dirMap.count(dir)){
                    dirMap.insert(std::make_pair(dir, (int)(dirMap.size())));
                }
            }

            if(dirMap.empty()){
                throw fflerror("empty frame seq record");
            }
            return dirMap;
        }();

        // but NPC respect MOTION_NPC_XXX

        for(const auto &p: frameSeqMap){
            const int encodeMotion = p.first.at(1) - MOTION_NPC_STAND;
            if(encodeMotion < 0){
                throw fflerror("invalid NPC motion: %d", encodeMotion);
            }

            const int frameStart = p.second.start;
            const int frameCount = p.second.frameCount;

            for(int frame = 0; frame < frameCount; ++frame){
                const int gfxId = lookId * 100 + frameStart + frame;
                if(!(package.SetIndex(gfxId) && package.CurrentImageValid())){
                    throw fflerror("gfx table is wrong");
                }

                const auto imgInfo = package.CurrentImageInfo();
                pngBuf.resize(imgInfo.shWidth * imgInfo.shHeight);
                package.Decode(&(pngBuf[0]), 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);

                const int dir = dirMap.at(p.first.at(0));
                const auto fileName = createOffsetFileName(outDir, false, lookId, encodeMotion, dir, frame, imgInfo.shPX, imgInfo.shPY);
                pngf::saveRGBABuffer(reinterpret_cast<const uint8_t *>(pngBuf.data()), imgInfo.shWidth, imgInfo.shHeight, fileName.c_str());

                const auto [needShadow, projectShadow] = [lookId]() -> std::tuple<bool, bool>
                {
                    switch(lookId){
                        case 51:
                        case 52:
                        case 55:
                        case 56:
                        case 59: return {false, false};
                        case 71:
                        case 72:
                        case 73: return {true , false};
                        default: return {true , true };
                    }
                }();

                if(!needShadow){
                    continue;
                }

                // make a big buffer to hold the shadow as needed
                // shadow buffer size depends on do project or not
                //
                //  project :  (nW + nH / 2) x (nH / 2 + 1)
                //          :  (nW x nH)

                const int maxShadowW = (std::max<int>)(imgInfo.shWidth + imgInfo.shHeight / 2, imgInfo.shWidth ) + 20;
                const int maxShadowH = (std::max<int>)(             1 + imgInfo.shHeight / 2, imgInfo.shHeight) + 20;
                pngBufShadow.resize(maxShadowW * maxShadowH);

                int nShadowW = 0;
                int nShadowH = 0;
                Shadow::MakeShadow(&(pngBufShadow[0]), projectShadow, &(pngBuf[0]), imgInfo.shWidth, imgInfo.shHeight, &nShadowW, &nShadowH, 0XFF000000);

                if(nShadowW <= 0 || nShadowH <= 0){
                    throw fflerror("create shadow image failed");
                }

                const auto shadowFileName = createOffsetFileName(outDir, true, lookId, encodeMotion, dir, frame, 
                        projectShadow ? imgInfo.shShadowPX : (imgInfo.shPX + 3),
                        projectShadow ? imgInfo.shShadowPY : (imgInfo.shPY + 2));
                pngf::saveRGBABuffer(reinterpret_cast<const uint8_t *>(pngBufShadow.data()), nShadowW, nShadowH, shadowFileName.c_str());
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc != 5){
        printUsage();
        return 1;
    }

    npcWil2PNG(argv[1], argv[2], argv[3], argv[4]);
    return 0;
}
