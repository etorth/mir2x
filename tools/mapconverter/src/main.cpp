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

#include <regex>
#include <memory>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "strf.hpp"
#include "imgf.hpp"
#include "utf8f.hpp"
#include "mir2map.hpp"
#include "imagedb.hpp"
#include "filesys.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "argparser.hpp"
#include "threadpool.hpp"
#include "mir2xmapdata.hpp"

// parser of /home/anhong/Dropbox/mir3_config/Mapinfo.txt.utf8.txt
// Mapinfo.txt.utf8.txt is same as Mapinfo.txt, but converted to utf8 encoding otherwise this code can't parse it

class MapInfoParser
{
    private:
        struct MapSwitchPoint
        {
            int from_x = -1;
            int from_y = -1;

            std::string to_map {};
            int to_x = -1;
            int to_y = -1;
        };

        struct MapEntry
        {
            std::string name {};
            std::vector<MapSwitchPoint> transp {};
        };

    private:
        std::unordered_map<std::string, MapEntry> m_mapList;

    public:
        MapInfoParser(const std::string &mapInfoFileName)
        {
            // do seperated parsing
            // otherwise map switch point could not find map name

            parseMapFileName(mapInfoFileName);
            parseMapSwitchPoint(mapInfoFileName);
        }

    public:
        std::string mapName(std::string fileName) const
        {
            if(auto p = m_mapList.find(utf8f::toupper(fileName)); p != m_mapList.end()){
                return p->second.name + "_" + utf8f::toupper(fileName);
            }
            return std::string("未知地图") + "_" + utf8f::toupper(fileName);
        }

    private:
        void parseMapFileName(const std::string &mapInfoFileName)
        {
            std::ifstream f(mapInfoFileName);
            fflassert(f);

            const std::regex expr(R"#(^.*\[([0-9a-zA-Z_]+)\s+([^ ]+)\s+.*\].*\r*$)#");
            //                         -- - -------------     -----
            //                         ^  ^       ^             ^
            //                         |  |       |             |
            //                         |  |       |             +------ map name
            //                         |  |       +-------------------- map file name
            //                         |  +---------------------------- marker for map file name lines: [ ... ]
            //                         +------------------------------- matches comment markers: ;;

            std::string line;
            std::match_results<std::string::iterator> result;

            while(std::getline(f, line)){
                fflassert(utf8f::valid(line));
                if(std::regex_match(line.begin(), line.end(), result, expr)){
                    std::string  mapName;
                    std::string fileName;
                    for(int i = 0; const auto &m: result){
                        switch(i++){
                            case 1 : fileName = utf8f::toupper(m.str()); break;
                            case 2 :  mapName = utf8f::toupper(m.str()); break;
                            default:                                     break;

                        }
                    }

                    if(m_mapList.count(fileName)){
                        std::cout << str_printf("[ERROR]: (%s:%s) is already in the map file list", fileName.c_str(), mapName.c_str()) << std::endl;
                    }
                    else{
                        m_mapList[fileName] = MapEntry
                        {
                            .name = mapName,
                        };
                        std::cout << str_printf("[FILE] %s: %s", fileName.c_str(), mapName.c_str()) << std::endl;
                    }
                }
            }
        }

        void parseMapSwitchPoint(const std::string &mapInfoFileName)
        {
            std::ifstream f(mapInfoFileName);
            fflassert(f);

            const std::regex expr(R"#(^.*([0-9a-zA-Z_]+)\s+(\d+),\s*(\d+)\s*->\s*([0-9a-zA-Z_]+)\s+(\d+),\s*(\d+).*$)#");
            //                         -- -------------     ---      ---    --    -------------     ---      ---
            //                         ^        ^            ^        ^     ^          ^             ^        ^
            //                         |        |            |        |     |          |             |        |
            //                         |        |            |        |     |          |             |        +---------- to_y
            //                         |        |            |        |     |          |             +------------------- to_x
            //                         |        |            |        |     |          +--------------------------------- to_map
            //                         |        |            |        |     +-------------------------------------------- transport marker: ->
            //                         |        |            |        +-------------------------------------------------- from_y
            //                         |        |            +----------------------------------------------------------- from_x
            //                         |        +------------------------------------------------------------------------ from_map
            //                         +--------------------------------------------------------------------------------- matches comments mark: ;;

            std::string line;
            std::match_results<std::string::iterator> result;

            while(std::getline(f, line)){
                fflassert(utf8f::valid(line));
                if(std::regex_match(line.begin(), line.end(), result, expr)){
                    std::string fromFileName;
                    MapSwitchPoint switchPoint;

                    for(int i = 0; const auto &m: result){
                        switch(i++){
                            case 1 : fromFileName       = utf8f::toupper(m.str()); break;
                            case 2 : switchPoint.from_x =      std::stoi(m.str()); break;
                            case 3 : switchPoint.from_y =      std::stoi(m.str()); break;
                            case 4 : switchPoint.to_map = utf8f::toupper(m.str()); break;
                            case 5 : switchPoint.to_x   =      std::stoi(m.str()); break;
                            case 6 : switchPoint.to_y   =      std::stoi(m.str()); break;
                            default:                                               break;
                        }
                    }

                    // we don't have enough information here to check if the from/to points are valid
                    // leave the check to file generation time

                    fflassert(!fromFileName.empty());
                    fflassert(!switchPoint.to_map.empty());

                    fflassert(switchPoint.from_x >= 0);
                    fflassert(switchPoint.from_y >= 0);
                    fflassert(switchPoint.to_x >= 0);
                    fflassert(switchPoint.to_y >= 0);

                    if(!m_mapList.count(fromFileName)){
                        std::cout << str_printf("[ERROR] no from_map found: %s", fromFileName.c_str()) << std::endl;
                    }
                    else if(!m_mapList.count(switchPoint.to_map)){
                        std::cout << str_printf("[ERROR] no to_map found: %s", switchPoint.to_map.c_str()) << std::endl;
                    }
                    else{
                        m_mapList[fromFileName].transp.push_back(switchPoint);
                        std::cout << str_printf("[SWITCH] (%s, %d, %d) -> (%s, %d, %d)", fromFileName.c_str(), switchPoint.from_x, switchPoint.from_y, switchPoint.to_map.c_str(), switchPoint.to_x, switchPoint.to_y) << std::endl;
                    }
                }
            }
        }
};

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

static void convertMap(std::string mapDir, std::string mapName, std::string outDir, const MapInfoParser *parser, ImageDB &imgDB)
{
    auto mapPtr = std::make_unique<Mir2Map>(str_printf("%s/%s", mapDir.c_str(), mapName.c_str()).c_str());

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

    const auto [pathName, baseName, extName] = filesys::decompFileName(mapName.c_str(), true);
    fflassert(pathName.empty());

    outPtr->save(str_printf("%s/%s.bin", outDir.c_str(), parser->mapName(baseName).c_str()));
    exportOverview(outPtr.get(), str_printf("%s/%s.png", outDir.c_str(), parser->mapName(baseName).c_str()), imgDB);
}

int main(int argc, char *argv[])
{
    if(argc == 1){
        std::cout << "mapconvert"                                                   << std::endl;
        std::cout << "      [1] output-dir              # output"                   << std::endl;
        std::cout << "      [2] thread-pool-size        # 0 means use maximium"     << std::endl;
        std::cout << "      [3] mir2-map-wil-data-dir   # input wil data dir"       << std::endl;
        std::cout << "      [4] mir2-map-dir            # input map dir"            << std::endl;
        std::cout << "      [5] mir2-map-info-file-path # Mapinfo.utf8.txt path"    << std::endl;
        return 0;
    }

    if(argc != 1 + 5 /* parameters listed above */){
        throw fflerror("run \"%s\" without parameter to show supported options", argv[0]);
    }

    threadPool pool(std::stoi(argv[2]));
    threadPool::abortedTag hasDecodeError;
    std::vector<std::unique_ptr<ImageDB>> dbList(pool.poolSize);
    const auto mapInfoParser = std::make_unique<MapInfoParser>(argv[5]);

    for(const auto &mapName: filesys::getFileList(argv[4], false, R"#(.*\.[mM][aA][pP]$)#")){
        pool.addTask(hasDecodeError, [argv, mapName, &dbList, &mapInfoParser](int threadId)
        {
            // ImageDB is not thread safe
            // need to allocate for each thread

            if(!dbList[threadId]){
                dbList[threadId] = std::make_unique<ImageDB>(argv[3]);
            }
            convertMap(argv[4], mapName, argv[1], mapInfoParser.get(), *dbList[threadId]);
        });
    }

    pool.finish();
    hasDecodeError.checkError();
    return 0;
}
