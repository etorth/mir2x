/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2017 16:12:32
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

#include <memory>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "strf.hpp"
#include "imgf.hpp"
#include "imagedb.hpp"
#include "sysconst.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"
#include "mir2map.hpp"
#include "threadpool.hpp"
#include "mir2xmapdata.hpp"

static void exportOverview(const Mir2xMapData *p, const std::string &outName, ImageDB &imgDB)
{
    const size_t imgW = p->w() * SYS_MAPGRIDXP;
    const size_t imgH = p->h() * SYS_MAPGRIDYP;

    std::vector<uint32_t> imgBuf(imgW * imgH, 0);
    for(size_t y = 0; y < p->h(); ++y){
        for(size_t x = 0; x < p->w(); ++x){
            if((x % 2) == 0 && (y % 2) == 0){
                if(const auto &tile = p->tile(x, y); tile.valid){
                    if(const auto [img, w, h] = imgDB.decode(tile.texID, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF); img){
                        imgf::blendImageBuffer(imgBuf.data(), imgW, imgH, img, w, h, x * SYS_MAPGRIDXP, y * SYS_MAPGRIDYP);
                    }
                }
            }
        }
    }

    for(const int depth: {OBJD_GROUND, OBJD_OVERGROUND0, OBJD_OVERGROUND1, OBJD_SKY}){
        for(size_t y = 0; y < p->h(); ++y){
            for(size_t x = 0; x < p->w(); ++x){
                for(const int objIndex: {0, 1}){
                    if(const auto &obj = p->cell(x, y).obj[objIndex]; obj.valid && obj.depth == depth){
                        if(const auto [img, w, h] = imgDB.decode(obj.texID, 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF); img){
                            imgf::blendImageBuffer(imgBuf.data(), imgW, imgH, img, w, h, x * SYS_MAPGRIDXP, y * SYS_MAPGRIDYP + SYS_MAPGRIDYP - h);
                        }
                    }
                }
            }
        }
    }
    imgf::saveImageBuffer(imgBuf.data(), imgW, imgH, outName.c_str());
}

static void convertMap(std::string fromName, std::string outName, ImageDB &imgDB)
{
    std::cout << str_printf("converting %s", fromName.c_str()) << std::endl;
    auto mapPtr = std::make_unique<Mir2Map>(fromName.c_str());

    fflassert((mapPtr->w() % 2) == 0);
    fflassert((mapPtr->h() % 2) == 0);

    auto outPtr = std::make_unique<Mir2xMapData>();
    outPtr->allocate(mapPtr->w(), mapPtr->h());

    for(int y = 0; y < to_d(mapPtr->h()); ++y){
        for(int x = 0; x < to_d(mapPtr->w()); ++x){
            if((x % 2) == 0 && (y % 2) == 0){
                mapPtr->convBlock(x, y, outPtr->block(x, y), imgDB);
            }
        }
    }

    outPtr->save(outName);
    exportOverview(outPtr.get(), outName + ".png", imgDB);
}

int main(int argc, char *argv[])
{
    if(argc != 4){
        throw fflerror("Usage: %s mir2-map-wil-path mir2-map-path mir2x-map-output", argv[0]);
    }

    threadPool pool(0);
    threadPool::abortedTag hasError;
    std::vector<std::unique_ptr<ImageDB>> dbList(pool.poolSize);

    for(const auto &mapName: filesys::getFileList(argv[2], false, R"#(.*\.[mM][aA][pP]$)#")){
        const auto srcPath = str_printf("%s/%s",     argv[2], mapName.c_str());
        const auto dstPath = str_printf("%s/%s.bin", argv[3], mapName.c_str());
        pool.addTask(hasError, [argv, srcPath, dstPath, &dbList](int threadId)
        {
            // ImageDB is not thread safe
            // need to allocate for each thread

            if(!dbList[threadId]){
                dbList[threadId] = std::make_unique<ImageDB>(argv[1]);
            }
            convertMap(srcPath, dstPath, *dbList[threadId]);
        });
    }

    pool.finish();
    hasError.checkError();
    return 0;
}
