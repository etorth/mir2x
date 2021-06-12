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
#include "strf.hpp"
#include "imagedb.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"
#include "mir2map.hpp"
#include "mir2xmapdata.hpp"

void convertMap(std::string fromName, std::string outName, ImageDB &imgDB)
{
    std::cout << str_printf("converting %s", fromName.c_str()) << std::endl;
    auto mapPtr = std::make_unique<Mir2Map>(fromName.c_str());
    auto outPtr = std::make_unique<Mir2xMapData>();

    outPtr->allocate(mapPtr->w(), mapPtr->h());
    for(int y = 0; y < to_d(mapPtr->h()); ++y){
        for(int x = 0; x < to_d(mapPtr->w()); ++x){
            if((x % 2) == 0 && (y % 2) == 0){
                if(mapPtr->tileValid(x, y, imgDB)){
                    outPtr->tile(x, y).texIDValid = 1;
                    outPtr->tile(x, y).texID = mapPtr->tile(x, y) & 0X00FFFFFF;
                }
            }

            if(mapPtr->lightValid(x, y)){
                outPtr->cell(x, y).light.valid  = 1;
                outPtr->cell(x, y).light.radius = mapPtr->lightSize(x, y)  & 0X03;
                outPtr->cell(x, y).light.alpha  = 0;
                outPtr->cell(x, y).light.color  = mapPtr->lightColor(x, y) & 0X07;
            }

            if(mapPtr->groundValid(x, y)){
                outPtr->cell(x, y).canWalk  = 1;
                outPtr->cell(x, y).canFly   = 1;
                outPtr->cell(x, y).landType = 0;
            }

            for(const int i: {0, 1}){
                if(mapPtr->objectValid(x, y, i, imgDB)){
                    outPtr->cell(x, y).obj[i].texIDValid = 1;
                    outPtr->cell(x, y).obj[i].texID = mapPtr->object(x, y, i);
                    outPtr->cell(x, y).obj[i].depthType = mapPtr->groundObjectValid(x, y, i, imgDB) ? OBJD_GROUND : OBJD_OVERGROUND0;

                    if(mapPtr->aniObjectValid(x, y, i, imgDB)){
                        outPtr->cell(x, y).obj[i].animated   = 1;
                        outPtr->cell(x, y).obj[i].alpha      = to_u8(mapPtr->object(x, y, i) & 0X80000000) >> (7 + 8 + 16);
                        outPtr->cell(x, y).obj[i].tickType   = to_u8(mapPtr->object(x, y, i) & 0X70000000) >> (4 + 8 + 16);
                        outPtr->cell(x, y).obj[i].frameCount = to_u8(mapPtr->object(x, y, i) & 0X0F000000) >> (0 + 8 + 16);
                    }
                }
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

    ImageDB imgDB(argv[1]);
    for(const auto &mapName: filesys::getFileList(argv[2], false, R"#(.*\.[mM][aA][pP]$)#")){
        const auto srcPath = str_printf("%s/%s",     argv[2], mapName.c_str());
        const auto dstPath = str_printf("%s/%s.bin", argv[3], mapName.c_str());
        convertMap(srcPath, dstPath, imgDB);
    }
}
