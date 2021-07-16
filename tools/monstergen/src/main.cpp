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

#include <mutex>
#include <regex>
#include <memory>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "strf.hpp"
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
                    std::cout << str_printf("[SKIP]: %s", line.c_str()) << std::endl;
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
                        std::cout << str_printf("[SKIP]: %s", line.c_str()) << std::endl;
                    }
                }
            }
        }
};

// static void printCodeLine(std::vector<std::string> s)
// {
//     static std::mutex lock;
//     std::lock_guard<std::mutex> lockGuard(lock);
//
//     for(auto &line: s){
//         while(!line.empty() && line.back() == ' '){
//             line.pop_back();
//         }
//         std::cout << "##### <-|" << line << std::endl;
//     }
// }

int main(int argc, char *argv[])
{
    if(argc == 1){
        std::cout << "monstergen"                                                  << std::endl;
        std::cout << "      [1] maprecord.inc path  # maprecord.inc"               << std::endl;
        std::cout << "      [2] monstergen dir      # Mon_Def_GB2312_to_utf8 path" << std::endl;
        return 0;
    }

    if(argc != 1 + 2 /* parameters listed above */){
        throw fflerror("run \"%s\" without parameter to show supported options", argv[0]);
    }

    MapRecordFileParser mapParser(argv[1]);
    GenFileParser genParser(argv[2]);
    return 0;
}
