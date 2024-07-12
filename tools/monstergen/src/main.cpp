#include <mutex>
#include <regex>
#include <memory>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "strf.hpp"
#include "mathf.hpp"
#include "utf8f.hpp"
#include "totype.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"

// parser of /home/anhong/Dropbox/mir3_config/Mon_Def_GB2312_to_utf8
// Mon_Def_GB2312_to_utf8 is from Mon_Def but converted to utf8:
//
//      iconv -f GB2312 -t UTF-8
//
// linux file command shows incorrect encoding, need to use another command: chardet

class MapRecordFileParser
{
    public:
        struct MapEntry
        {
            std::string  mapName;
            std::string fileName;
        };

    private:
        std::vector<MapEntry> m_mapEntryList;

    public:
        MapRecordFileParser(const std::string &mapincFile)
        {
            std::ifstream f(mapincFile);
            fflassert(f);

            const std::regex expr(R"#(^\{\s*\.name\s*=\s*u8"(.*?)_(.*?)".*$)#");
            //                         --   ---------------  ---   ---
            //                          ^          ^          ^     ^
            //                          |          |          |     |
            //                          |          |          |     +----------- map file name
            //                          |          |          +----------------- map name
            //                          |          +---------------------------- marker: .name = u8"
            //                          +--------------------------------------- marker: {

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

                    m_mapEntryList.push_back(entry);
                    std::cout << str_printf("[FILE] %s:%s", entry.mapName.c_str(), entry.fileName.c_str()) << std::endl;
                }
                else{
                    std::cout << str_printf("[SKIP] %s", line.c_str()) << std::endl;
                }
            }
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
            return result;
        }
};

class GenFileParser
{
    public:
        struct GenEntry
        {
            std::string fileName;
            int x = 0;
            int y = 0;

            std::string monName;
            int range  = 0;
            int num    = 0;
            int time   = 0;
            int cratio = 0;
        };

    private:
        std::vector<GenEntry> m_genEntryList;

    public:
        GenFileParser(const std::string &gendir)
        {
            const std::regex expr(R"#(^.*?([0-9a-zA-Z_]+)\s+(\d+)\s+(\d+)\s+(.*?)\s+(\d+)\s+(\d+)\s+(\d+)\s*(\d*)\s*.*$)#");
            //                         --- -------------     ---     ---     ---     ---     ---     ---     ---
            //                          ^        ^            ^       ^       ^       ^       ^       ^       ^
            //                          |        |            |       |       |       |       |       |       |
            //                          |        |            |       |       |       |       |       |       +---------- cratio, optional
            //                          |        |            |       |       |       |       |       +------------------ time
            //                          |        |            |       |       |       |       +-------------------------- num
            //                          |        |            |       |       |       +---------------------------------- range
            //                          |        |            |       |       +------------------------------------------ monster name
            //                          |        |            |       +-------------------------------------------------- map y
            //                          |        |            +---------------------------------------------------------- map x
            //                          |        +----------------------------------------------------------------------- map file name
            //                          +-------------------------------------------------------------------------------- matches comment markers: ;;

            std::string line;
            std::match_results<std::string::iterator> result;

            for(const auto &genfile: filesys::getFileList(gendir.c_str(), "^.*?.[Gg][Ee][Nn]$")){
                std::ifstream f(genfile);
                fflassert(f);

                while(std::getline(f, line)){
                    fflassert(utf8f::valid(line));
                    if(std::regex_match(line.begin(), line.end(), result, expr)){
                        GenEntry entry;
                        for(int i = 0; const auto &m: result){
                            switch(i++){
                                case 1 : entry.fileName =                  utf8f::toupper(m.str()); break;
                                case 2 : entry.       x =                       std::stoi(m.str()); break;
                                case 3 : entry.       y =                       std::stoi(m.str()); break;
                                case 4 : entry. monName =                  utf8f::toupper(m.str()); break;
                                case 5 : entry.   range =                       std::stoi(m.str()); break;
                                case 6 : entry.     num =                       std::stoi(m.str()); break;
                                case 7 : entry.    time =                       std::stoi(m.str()); break;
                                case 8 : entry.  cratio = m.str().empty() ? 0 : std::stoi(m.str()); break;
                                default:                                                            break;
                            }
                        }

                        m_genEntryList.push_back(entry);
                        std::cout << str_printf("[FILE] %s:%s", entry.fileName.c_str(), entry.monName.c_str()) << std::endl;
                    }
                    else{
                        std::cout << str_printf("[SKIP] %s", line.c_str()) << std::endl;
                    }
                }
            }
        }

    public:
        void print(const MapRecordFileParser &parser, const std::string &outDir)
        {
            std::map<std::string, std::map<std::string, std::vector<GenEntry>>> genInfo;
            for(const auto &entry: m_genEntryList){
                genInfo[entry.fileName][entry.monName].push_back(entry);
            }

            // create lua table
            // for each map, print format:
            // local monsterGenList = -- 道馆_01
            // {
            //     {
            //         name = '鸡',
            //         loc  = {
            //             {x = 1, y = 2, w = 3, h = 4, count = 4, time = 5, cratio = 6},
            //             {x = 1, y = 2, w = 3, h = 4, count = 4, time = 5, cratio = 6},
            //             {x = 1, y = 2, w = 3, h = 4, count = 4, time = 5, cratio = 6},
            //         }
            //     },
            //     {
            //         name = '羊',
            //         loc  = {
            //             {x = 1, y = 2, w = 3, h = 4, count = 4, time = 5, cratio = 6},
            //             {x = 1, y = 2, w = 3, h = 4, count = 4, time = 5, cratio = 6},
            //         }
            //     },
            // }

            std::vector<std::string> codeList;
            for(const auto &[file, monGenList]: genInfo){
                const auto mapNameList = parser.hasMapName(file);
                if(mapNameList.empty()){
                    std::cout << str_printf("[ERROR] no map uses file: %s", file.c_str()) << std::endl;
                }
                else{
                    for(const auto &mapName: mapNameList){
                        codeList.clear();
                        codeList.push_back(str_printf("local addmon = require('map.addmonster')"));
                        codeList.push_back(str_printf("local addMonCo = addmon.monGener( -- %s_%s%s", mapName.c_str(), file.c_str(), (mapNameList.size() > 1) ? " TODO" : ""));
                        codeList.push_back(str_printf("{"));

                        for(const auto &[monName, entryList]: monGenList){
                            codeList.push_back(str_printf("    {"));
                            codeList.push_back(str_printf("        name = '%s',", monName.c_str()));
                            codeList.push_back(str_printf("        loc = {"));

                            for(const auto &entry: entryList){
                                const auto x      = std::max<int>(entry.x, 0);
                                const auto y      = std::max<int>(entry.y, 0);
                                const auto r      = std::max<int>(entry.range, 1);
                                const auto count  = std::max<int>(entry.num, 1);
                                const auto time   = std::max<int>(entry.time * 60, 10);
                                const auto cratio = mathf::bound<int>(entry.cratio, 0, 100);
                                const auto cratioStr = (cratio == 0) ? std::string("") : str_printf(", cratio = %d", cratio);
                                codeList.push_back(str_printf("            {x = %d, y = %d, w = %d, h = %d, count = %d, time = %d%s},", x, y, r, r, count, time, cratioStr.c_str()));
                            }
                            codeList.push_back(str_printf("        }"));
                            codeList.push_back(str_printf("    },"));
                        }
                        codeList.push_back(str_printf("})"));
                        codeList.push_back({});

                        codeList.push_back(str_printf("function main()"));
                        codeList.push_back(str_printf("    while true do"));
                        codeList.push_back(str_printf("        local rc, errMsg = coroutine.resume(addMonCo)"));
                        codeList.push_back(str_printf("        if not rc then"));
                        codeList.push_back(str_printf("            fatalPrintf('addMonCo failed: %%s', argDef(errMsg, 'unknown error'))"));
                        codeList.push_back(str_printf("        end"));
                        codeList.push_back(str_printf("        asyncWait(1000 * 5)"));
                        codeList.push_back(str_printf("    end"));
                        codeList.push_back(str_printf("end"));

                        std::ofstream f(str_printf("%s/%s_%s.lua", outDir.c_str(), mapName.c_str(), file.c_str()));
                        for(const auto &codeLine: codeList){
                            std::cout << "[CODE] " << codeLine << std::endl;
                            f << codeLine << std::endl;
                        }
                    }
                }
            }
        }
};

int main(int argc, char *argv[])
{
    if(argc == 1){
        std::cout << "monstergen"                                                  << std::endl;
        std::cout << "      [1] lua out dir         # out dir"                     << std::endl;
        std::cout << "      [2] maprecord.inc path  # maprecord.inc"               << std::endl;
        std::cout << "      [3] monstergen dir      # Mon_Def_GB2312_to_utf8 path" << std::endl;
        return 0;
    }

    if(argc != 1 + 3 /* parameters listed above */){
        throw fflerror("run \"%s\" without parameter to show supported options", argv[0]);
    }

    MapRecordFileParser mapParser(argv[2]);
    GenFileParser genParser(argv[3]);

    genParser.print(mapParser, argv[1]);
    return 0;
}
