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
#include <fstream>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "strf.hpp"
#include "utf8f.hpp"
#include "filesys.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

// parser of /home/anhong/Dropbox/mir3_config/merchant.utf8.txt
// merchant.utf8.txt is same as merchant.txt, but converted to utf8 encoding otherwise this code can't parse it

class MerchantFileParser
{
    public:
        struct NPCEntry
        {
            std::string mapName {};
            std::string npcName {};

            int x = -1;
            int y = -1;

            int lookID = -1;
            int faceID = -1;
        };

    private:
        std::vector<NPCEntry> m_list;

    public:
        MerchantFileParser(const std::string &merchantFileName)
        {
            std::ifstream f(merchantFileName);
            fflassert(f);

            const std::regex expr(R"#(^.*?([0-9a-zA-Z_]+)\s+([0-9a-zA-Z_]+)\s+(\d+)\s+(\d+)\s+(\S+)\s+(\d+)\s+(\d+)\s*$)#");
            //                         --- -------------     -------------     ---     ---     ---     ---     ---
            //                          ^        ^                 ^            ^       ^       ^       ^       ^
            //                          |        |                 |            |       |       |       |       |
            //                          |        |                 |            |       |       |       |       +------ lookID
            //                          |        |                 |            |       |       |       +-------------- faceID
            //                          |        |                 |            |       |       +---------------------- name
            //                          |        |                 |            |       +------------------------------ y
            //                          |        |                 |            +-------------------------------------- x
            //                          |        |                 +--------------------------------------------------- map name
            //                          |        +--------------------------------------------------------------------- script name
            //                          +------------------------------------------------------------------------------ matches comment markers: ;;

            std::string line;
            std::unordered_map<std::string, int> seen;
            std::match_results<std::string::iterator> result;

            while(std::getline(f, line)){
                fflassert(utf8f::valid(line));
                if(std::regex_match(line.begin(), line.end(), result, expr)){
                    NPCEntry entry;
                    for(int i = 0; const auto &m: result){
                        switch(i++){
                            case 2 : entry.mapName = utf8f::toupper(m.str()); break;
                            case 3 : entry.x       =      std::stoi(m.str()); break;
                            case 4 : entry.y       =      std::stoi(m.str()); break;
                            case 5 : entry.npcName = utf8f::toupper(m.str()); break;
                            case 6 : entry.faceID  =      std::stoi(m.str()); break;
                            case 7 : entry.lookID  =      std::stoi(m.str()); break;
                            default:                                          break;
                        }
                    }

                    const auto count = seen[str_printf("%s.%s", entry.mapName.c_str(), entry.npcName.c_str())]++;
                    entry.npcName += str_printf("_%d", count + 1);

                    m_list.push_back(entry);
                    std::cout << str_printf("[FILE] %s:%s", entry.mapName.c_str(), entry.npcName.c_str()) << std::endl;
                }
            }
        }

    public:
        const auto & getNPCList() const
        {
            return m_list;
        }
};

class MapRecordFileParser
{
    public:
        struct MapEntry
        {
            std::string  mapName {};
            std::string fileName {};
        };

    private:
        std::vector<MapEntry> m_list;

    public:
        MapRecordFileParser(const std::string &maprecordFilePath)
        {
            std::ifstream f(maprecordFilePath);
            fflassert(f);

            const std::regex expr(R"#(^.*?\{\s*\.name\s*=\s*u8"([^_]+)_([0-9a-zA-Z_]+)"\s*,\s*$)#");
            //                         ------------------------ -----   -------------
            //                                    ^               ^            ^
            //                                    |               |            |
            //                                    |               |            +----- file name
            //                                    |               +------------------ map name
            //                                    +---------------------------------- matches pattern: { .name = u8"xxx_...

            std::string line;
            std::match_results<std::string::iterator> result;

            while(std::getline(f, line)){
                fflassert(utf8f::valid(line));
                if(std::regex_match(line.begin(), line.end(), result, expr)){
                    MapEntry entry;
                    for(int i = 0; const auto &m: result){
                        switch(i++){
                            case 1 : entry. mapName = utf8f::toupper(m.str()); break;
                            case 2 : entry.fileName = utf8f::toupper(m.str()); break;
                            default:                                           break;
                        }
                    }

                    m_list.push_back(entry);
                    std::cout << str_printf("[FILE] %s:%s", entry.mapName.c_str(), entry.fileName.c_str()) << std::endl;
                }
            }
        }

    public:
        std::vector<MapEntry> getMapList(const std::string &fileName) const
        {
            std::vector<MapEntry> result;
            for(const auto &entry: m_list){
                if(entry.fileName == fileName){
                    result.push_back(entry);
                }
            }
            return result;
        }
};

static void npcGen(const std::string &outDir, const std::string &merchantFilePath, const std::string &maprecordFilePath)
{
    const auto merchantParser = std::make_unique<MerchantFileParser>(merchantFilePath);
    const auto maprecordParser = std::make_unique<MapRecordFileParser>(maprecordFilePath);

    for(const auto &npc: merchantParser->getNPCList()){
        const auto &entry = maprecordParser->getMapList(npc.mapName);
        if(entry.empty()){
            continue;
        }

        if(entry.size() != 1){
            std::cout << str_printf("[WARN] more than one file maps to %s", npc.mapName.c_str()) << std::endl;
        }

        std::ofstream f(str_printf("%s/%s_%s.%s.lua", outDir.c_str(), entry[0].mapName.c_str(), entry[0].fileName.c_str(), npc.npcName.c_str()));
        f << str_printf("setNPCLook(%d)", npc.lookID) << std::endl;
        f << str_printf("setNPCGLoc(%d, %d)", npc.x, npc.y);
    }
}

int main(int argc, char *argv[])
{
    if(argc == 1){
        std::cout << "npcgen"                                                       << std::endl;
        std::cout << "      [1] output-dir              # output"                   << std::endl;
        std::cout << "      [2] mir2-merchant-info-path # merchant.utf8.txt"        << std::endl;
        std::cout << "      [3] maprecord-inc-path      # common/src/maprecord.inc" << std::endl;
        return 0;
    }

    if(argc != 1 + 3 /* parameters listed above */){
        throw fflerror("run \"%s\" without parameter to show supported options", argv[0]);
    }

    npcGen(argv[1], argv[2], argv[3]);
    return 0;
}
