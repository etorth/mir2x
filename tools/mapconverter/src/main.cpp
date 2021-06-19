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
#include "totype.hpp"
#include "fflerror.hpp"
#include "argparser.hpp"
#include "threadpool.hpp"
#include "mir2xmapdata.hpp"

// parser of /home/anhong/Dropbox/mir3_config/Mapinfo.txt.utf8.txt
// Mapinfo.txt.utf8.txt is same as Mapinfo.txt, but converted to utf8 encoding otherwise this code can't parse it

class MapInfoParser
{
    // use mapName:fileName as key, from the Mapinfo.txt we know:
    // 1. different map file can use same map name, i.e. 沙漠 for 42.map, 43.map, 44.map
    // 2. different map name can use same map file, i.e. 行会大战 and 半兽天然洞穴 use same map file E001.map
    // 3. there are files not present in file:name entry but has switch points

    // this makes difficulty when there is switch point
    // we have to manually define which map exactly it needs to switch to
    // MapInfoParser only record lines, it doesn't validate anything

    public:
        struct MapEntry
        {
            std::string  mapName {};
            std::string fileName {};
        };

        struct MapSwitchPoint
        {
            std::string from_fileName {};
            int from_x = -1;
            int from_y = -1;

            std::string to_fileName {};
            int to_x = -1;
            int to_y = -1;
        };

        struct MiniMapEntry
        {
            std::string fileName {};
            int miniMapId = -1;
        };

    private:
        std::vector<MapEntry> m_mapEntryList;
        std::vector<MapSwitchPoint> m_mapSwitchPointList;
        std::vector<MiniMapEntry> m_miniMapEntryList;

    public:
        MapInfoParser(const std::string &mapInfoFileName, const std::string &miniMapFileName)
        {
            // do seperated parsing
            // otherwise map switch point could not find map name

            parseMapFileName(mapInfoFileName);
            parseMapSwitchPoint(mapInfoFileName);
            parseMiniMapID(miniMapFileName);
        }

    public:
        std::vector<std::string> hasMapName(const std::string &fileName) const
        {
            std::vector<std::string> result;
            for(const auto &line: m_mapEntryList){
                if(line.fileName == utf8f::toupper(fileName)){
                    result.push_back(line.mapName);
                }
            }

            std::sort(result.begin(), result.end());

            if(!result.empty()){
                return result;
            }
            return {"未命名地图"};
        }

        std::vector<MapSwitchPoint> hasSwitchPoint(const std::string &fileName) const
        {
            std::vector<MapSwitchPoint> result;
            for(const auto &switchPoint: m_mapSwitchPointList){
                if(switchPoint.from_fileName == utf8f::toupper(fileName)){
                    result.push_back(switchPoint);
                }
            }

            std::sort(result.begin(), result.end(), [](const auto &parm1, const auto &parm2) -> bool
            {
                // don't need to check from_fileName
                return std::make_tuple(parm1.to_fileName, parm1.from_x, parm1.from_y, parm1.to_x, parm1.to_y) < std::make_tuple(parm2.to_fileName, parm2.from_x, parm2.from_y, parm2.to_x, parm2.to_y);
            });
            return result;
        }

        int hasMiniMapID(const std::string &fileName) const
        {
            int miniMapId = -1;
            for(const auto &line: m_miniMapEntryList){
                if(line.fileName == utf8f::toupper(fileName)){
                    if(miniMapId >= 0 && miniMapId != line.miniMapId){
                        std::cout << str_printf("[ERROR] map file has more than 1 mini map id: %s -> %d, %d", line.fileName.c_str(), miniMapId, line.miniMapId) << std::endl;
                    }
                    else{
                        miniMapId = line.miniMapId;
                    }
                }
            }
            return miniMapId;
        }

    private:
        void parseMapFileName(const std::string &mapInfoFileName)
        {
            std::ifstream f(mapInfoFileName);
            fflassert(f);

            const std::regex expr(R"#(^.*?\[([0-9a-zA-Z_]+)\s+([^ ]+)\s+.*\].*\r*$)#");
            //                         --- - -------------     -----
            //                          ^  ^       ^             ^
            //                          |  |       |             |
            //                          |  |       |             +------ map name
            //                          |  |       +-------------------- map file name
            //                          |  +---------------------------- marker for map file name lines: [ ... ]
            //                          +------------------------------- matches comment markers: ;;

            std::string line;
            std::match_results<std::string::iterator> result;

            while(std::getline(f, line)){
                fflassert(utf8f::valid(line));
                if(std::regex_match(line.begin(), line.end(), result, expr)){
                    MapEntry entry;
                    for(int i = 0; const auto &m: result){
                        switch(i++){
                            case 1 : entry.fileName = utf8f::toupper(m.str()); break;
                            case 2 : entry. mapName = utf8f::toupper(m.str()); break;
                            default:                                           break;
                        }
                    }

                    m_mapEntryList.push_back(entry);
                    std::cout << str_printf("[FILE] %s:%s", entry.mapName.c_str(), entry.fileName.c_str()) << std::endl;
                }
            }
        }

        void parseMapSwitchPoint(const std::string &mapInfoFileName)
        {
            std::ifstream f(mapInfoFileName);
            fflassert(f);

            const std::regex expr(R"#(^.*?([0-9a-zA-Z_]+)\s+(\d+),\s*(\d+)\s*->\s*([0-9a-zA-Z_]+)\s+(\d+),\s*(\d+).*$)#");
            //                         --- -------------     ---      ---    --    -------------     ---      ---
            //                          ^        ^            ^        ^     ^          ^             ^        ^
            //                          |        |            |        |     |          |             |        |
            //                          |        |            |        |     |          |             |        +---------- to_y
            //                          |        |            |        |     |          |             +------------------- to_x
            //                          |        |            |        |     |          +--------------------------------- to_map
            //                          |        |            |        |     +-------------------------------------------- transport marker: ->
            //                          |        |            |        +-------------------------------------------------- from_y
            //                          |        |            +----------------------------------------------------------- from_x
            //                          |        +------------------------------------------------------------------------ from_map
            //                          +--------------------------------------------------------------------------------- matches comments mark: ;;

            std::string line;
            std::match_results<std::string::iterator> result;

            while(std::getline(f, line)){
                fflassert(utf8f::valid(line));
                if(std::regex_match(line.begin(), line.end(), result, expr)){
                    MapSwitchPoint switchPoint;
                    for(int i = 0; const auto &m: result){
                        switch(i++){
                            case 1 : switchPoint.from_fileName = utf8f::toupper(m.str()); break;
                            case 2 : switchPoint.from_x        =      std::stoi(m.str()); break;
                            case 3 : switchPoint.from_y        =      std::stoi(m.str()); break;
                            case 4 : switchPoint.to_fileName   = utf8f::toupper(m.str()); break;
                            case 5 : switchPoint.to_x          =      std::stoi(m.str()); break;
                            case 6 : switchPoint.to_y          =      std::stoi(m.str()); break;
                            default:                                                      break;
                        }
                    }

                    // we don't have enough information here to check if the from/to maps and points are valid
                    // leave the check to file generation time

                    fflassert(!switchPoint.from_fileName.empty());
                    fflassert(!switchPoint.to_fileName.empty());

                    fflassert(switchPoint.from_x >= 0);
                    fflassert(switchPoint.from_y >= 0);

                    fflassert(switchPoint.to_x >= 0);
                    fflassert(switchPoint.to_y >= 0);

                    m_mapSwitchPointList.push_back(switchPoint);
                    std::cout << str_printf("[SWITCH] (%s, %d, %d) -> (%s, %d, %d)", switchPoint.from_fileName.c_str(), switchPoint.from_x, switchPoint.from_y, switchPoint.to_fileName.c_str(), switchPoint.to_x, switchPoint.to_y) << std::endl;
                }
            }
        }

        void parseMiniMapID(const std::string &miniMapFileName)
        {
            std::ifstream f(miniMapFileName);
            fflassert(f);

            const std::regex expr(R"#(^.*?([0-9a-zA-Z_]+)\s+(\d+).*$)#");
            //                         --- -------------     ---
            //                          ^       ^             ^
            //                          |       |             |
            //                          |       |             +------ id
            //                          |       +-------------------- map file name
            //                          +---------------------------- matches comment markers: ;; actually I didn't see commented line

            std::string line;
            std::match_results<std::string::iterator> result;

            while(std::getline(f, line)){
                fflassert(utf8f::valid(line));
                if(std::regex_match(line.begin(), line.end(), result, expr)){
                    MiniMapEntry entry;
                    for(int i = 0; const auto &m: result){
                        switch(i++){
                            case 1 : entry.fileName  = utf8f::toupper(m.str()); break;
                            case 2 : entry.miniMapId =      std::stoi(m.str()); break;
                            default:                                            break;
                        }
                    }

                    m_miniMapEntryList.push_back(entry);
                    std::cout << str_printf("[MINI] %s:%d", entry.fileName.c_str(), entry.miniMapId) << std::endl;
                }
            }
        }
};

static void printCodeLine(std::vector<std::string> s)
{
    static std::mutex lock;
    std::lock_guard<std::mutex> lockGuard(lock);

    for(auto &line: s){
        while(!line.empty() && line.back() == ' '){
            line.pop_back();
        }
        std::cout << "##### <-|" << line << std::endl;
    }
}

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

static void convertMap(std::string mapDir, std::string mapFileName, std::string outDir, const MapInfoParser *parser, bool incFileOnly, ImageDB &imgDB)
{
    std::unique_ptr<Mir2xMapData> outPtr;
    if(!incFileOnly){
        outPtr = std::make_unique<Mir2xMapData>();
        auto mapPtr = std::make_unique<Mir2Map>(str_printf("%s/%s", mapDir.c_str(), mapFileName.c_str()).c_str());

        fflassert((mapPtr->w() % 2) == 0);
        fflassert((mapPtr->h() % 2) == 0);
        outPtr->allocate(mapPtr->w(), mapPtr->h());

        for(int y = 0; y < to_d(mapPtr->h()); ++y){
            for(int x = 0; x < to_d(mapPtr->w()); ++x){
                if((x % 2) == 0 && (y % 2) == 0){
                    mapPtr->convBlock(x, y, outPtr->block(x, y), imgDB);
                }
            }
        }
    }

    const auto [pathName, fileName, extName] = filesys::decompFileName(mapFileName.c_str(), true);
    fflassert(pathName.empty());

    // different map can use same map binary file
    // we use "mapName_fileName" as string key in mir2x code, but only save "fileName.bin" as map binary
    if(!incFileOnly){
        outPtr->save(str_printf("%s/%s.bin", outDir.c_str(), utf8f::toupper(fileName).c_str()));
    }

    std::string srcPNGName;
    const auto mapNameList = parser->hasMapName(fileName);

    for(const auto &mapName: mapNameList){
        if(!incFileOnly){
            if(srcPNGName.empty()){ // first save
                srcPNGName = str_printf("%s/%s_%s.png", outDir.c_str(), mapName.c_str(), utf8f::toupper(fileName).c_str());
                exportOverview(outPtr.get(), srcPNGName, imgDB);
            }
            else{
                filesys::copyFile(str_printf("%s/%s_%s.png", outDir.c_str(), mapName.c_str(), utf8f::toupper(fileName).c_str()).c_str(), srcPNGName.c_str());
            }
        }

        // generate code
        //
        std::vector<std::string> codeList;
        codeList.push_back(str_printf(R"#({   .name = u8"%s_%s",)#", mapName.c_str(), utf8f::toupper(fileName).c_str()));

        if(const auto miniMapID = parser->hasMiniMapID(fileName); miniMapID >= 1){
            codeList.push_back(str_printf(R"#(    .miniMapID = 0X19%06d,)#", miniMapID - 1));
        }

        // map switch points
        // there are duplicated switch points

        std::unordered_set<std::string> seenSwitchCodeList;
        if(const auto &switchPointList = parser->hasSwitchPoint(fileName); !switchPointList.empty()){
            codeList.push_back(R"#(    .mapSwitchList)#");
            codeList.push_back(R"#(    {             )#");
            for(const auto &switchPoint: switchPointList){
                const auto toMapNameList = parser->hasMapName(switchPoint.to_fileName);
                for(const auto toMapName: toMapNameList){
                    const auto switchCodeLine = str_printf(R"#(        {.x = %d, .y = %d, .endName = u8"%s_%s", .endX = %d, .endY = %d},%s)#", switchPoint.from_x, switchPoint.from_y, toMapName.c_str(), switchPoint.to_fileName.c_str(), switchPoint.to_x, switchPoint.to_y, (toMapNameList.size() == 1) ? "" : "// TODO select one");
                    if(!seenSwitchCodeList.count(switchCodeLine)){
                        seenSwitchCodeList.insert(switchCodeLine);
                        codeList.push_back(switchCodeLine);
                    }
                }
            }
            codeList.push_back(R"#(    })#");
        }

        codeList.push_back(R"#(},)#");
        codeList.push_back(R"#(  )#");
        printCodeLine(codeList);
    }
}

int main(int argc, char *argv[])
{
    if(argc == 1){
        std::cout << "mapconvert"                                                    << std::endl;
        std::cout << "      [1] output-dir              # output"                    << std::endl;
        std::cout << "      [2] thread-pool-size        # 0 means use maximium"      << std::endl;
        std::cout << "      [3] mir2-map-wil-data-dir   # input wil data dir"        << std::endl;
        std::cout << "      [4] mir2-map-dir            # input map dir"             << std::endl;
        std::cout << "      [5] mir2-map-info-file-path # Mapinfo.utf8.txt path"     << std::endl;
        std::cout << "      [6] mir2-mini-map-file-path # MiniMap.utf8.txt path"     << std::endl;
        std::cout << "      [7] create-inc-file-only    # only create maprecord.inc" << std::endl;
        return 0;
    }

    if(argc != 1 + 7 /* parameters listed above */){
        throw fflerror("run \"%s\" without parameter to show supported options", argv[0]);
    }

    printCodeLine
    ({
        R"#({   .name = u8"",)#",
        R"#(},               )#",
        R"#(                 )#",
    });

    threadPool pool(std::stoi(argv[2]));
    threadPool::abortedTag hasDecodeError;
    std::vector<std::unique_ptr<ImageDB>> dbList(pool.poolSize);
    const auto mapInfoParser = std::make_unique<MapInfoParser>(argv[5], argv[6]);

    for(const auto &mapName: filesys::getFileList(argv[4], false, R"#(.*\.[mM][aA][pP]$)#")){
        pool.addTask(hasDecodeError, [argv, mapName, &dbList, &mapInfoParser](int threadId)
        {
            // ImageDB is not thread safe
            // need to allocate for each thread

            if(!dbList[threadId]){
                dbList[threadId] = std::make_unique<ImageDB>(argv[3]);
            }
            convertMap(argv[4], mapName, argv[1], mapInfoParser.get(), to_bool(argv[7]), *dbList[threadId]);
        });
    }

    pool.finish();
    hasDecodeError.checkError();
    return 0;
}
