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
#include "imagedb.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"
#include "mir2map.hpp"
#include "threadpool.hpp"
#include "mir2xmapdata.hpp"

void convertMap(std::string fromName, std::string outName, ImageDB &imgDB)
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
