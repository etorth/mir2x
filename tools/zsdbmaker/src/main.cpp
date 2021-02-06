/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 07/20/2017 00:34:13
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *
 * =====================================================================================
 */
#include <regex>
#include <cstdio>
#include <fstream>
#include <cinttypes>
#include "strf.hpp"
#include "zsdb.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "argparser.hpp"

static void cmd_help()
{
    std::printf("--help\n");
    std::printf("--create-db\n");
    std::printf("--prefix-index\n");
    std::printf("--list\n");
    std::printf("--decomp-db\n");
    std::printf("--input-dir\n");
    std::printf("--input-dict\n");
    std::printf("--input-file-name-regex\n");
}

static void cmd_create_db(const arg_parser &cmd)
{
    const auto outputFileName = [&cmd]() -> std::string
    {
        if(cmd["create-db"]){
            return "out.zsdb";
        }
        else{
            if(cmd("create-db").str().empty()){
                return "out.zsdb";
            }
            else{
                return cmd("create-db").str();
            }
        }
    }();

    const auto inputFileNameRegex = [&cmd]() -> std::string
    {
        if(!cmd.has_option("input-file-name-regex")){
            return "";
        }

        if(cmd["input-file-name-regex"] || cmd("input-file-name-regex").str().empty()){
            throw fflerror("input-file-name-regex requires an argument");
        }
        return cmd("input-file-name-regex").str();
    }();

    const auto inputDirName = [&cmd]() -> std::string
    {
        if(!cmd.has_option("input-dir")){
            return ".";
        }

        if(cmd["input-dir"] || cmd("input-dir").str().empty()){
            throw fflerror("input-dir requires an argument");
        }
        return cmd("input-dir").str();
    }();

    const auto inputDictName = [&cmd]() -> std::string
    {
        if(!cmd.has_option("input-dict")){
            return "";
        }

        if(cmd["input-dict"] || cmd("input-dict").str().empty()){
            throw fflerror("input-dict requires an argument");
        }
        return cmd("input-dict").str();
    }();

    const auto compressThreshold = [&cmd]() -> double
    {
        if(!cmd.has_option("compress-threshold")){
            return 0.90;
        }

        if(cmd["compress-threshold"] || cmd("compress-threshold").str().empty()){
            throw fflerror("compress-threshold requires an argument");
        }

        double threshold;
        cmd("compress-threshold", 0.90) >> threshold;
        return threshold;
    }();

    ZSDB::buildDB(outputFileName.c_str(), inputFileNameRegex.c_str(), inputDirName.c_str(), inputDictName.c_str(), compressThreshold);
}

static void cmd_list(const arg_parser &cmd)
{
    auto dbFileName = [&cmd]() -> std::string
    {
        if(cmd["list"] || cmd("list").str().empty()){
            throw fflerror("option list requires an argument");
        }

        return cmd("list").str();
    }();

    ZSDB db(dbFileName.c_str());
    const auto entryList = db.getEntryList();

    auto regexStr = [&cmd]() -> std::string
    {
        if(!cmd.has_option("data-name-regex")){
            return "";
        }

        if(auto regexStr = cmd("data-name-regex").str(); !regexStr.empty()){
            return regexStr;
        }
        throw fflerror("option --data-name-regex requires an argument");
    }();

    std::regex fileNameReg(regexStr.empty() ? ".*" : regexStr.c_str());
    for(const auto &entry: entryList){
        if(!regexStr.empty()){
            if(!std::regex_match(entry.fileName, fileNameReg)){
                continue;
            }
        }
        std::printf("%32s %8zu %8llu\n", entry.fileName, entry.length, to_llu(entry.attribute));
    }
}

static void cmd_uncomp_db(const arg_parser &cmd)
{
    const auto dbFileName = [&cmd]() -> std::string
    {
        if(cmd["decomp-db"] || cmd("decomp-db").str().empty()){
            throw fflerror("option --decomp-db requires an argument");
        }
        return cmd("decomp-db").str();
    }();

    ZSDB db(dbFileName.c_str());
    const auto entryList = db.getEntryList();
    const bool prefixIndexEnabled = cmd.has_option("prefix-index");

    uint64_t prefixIndex = 0;
    std::string prefixIndexFileName;
    std::vector<uint8_t> readBuf;

    for(const auto &entry: entryList){
        if(db.decomp(entry.fileName, 0, &readBuf)){
            const auto fileName = [prefixIndexEnabled, &entry, &prefixIndex, &prefixIndexFileName]() -> const char *
            {
                if(prefixIndexEnabled){
                    return str_printf(prefixIndexFileName, "%016llu_%s", to_llu(prefixIndex++), entry.fileName).c_str();
                }
                else{
                    return entry.fileName;
                }
            }();

            auto fptr = make_fileptr(fileName, "wb");
            if(std::fwrite(readBuf.data(), readBuf.size(), 1, fptr.get()) != 1){
                throw fflerror("failed to write to file: %s, err = %s", fileName, std::strerror(errno));
            }
            std::printf("%32s %8zu -> %8zu [%3d%%] %8llu\n", entry.fileName, entry.length, readBuf.size(), (int)(entry.length * 100 / readBuf.size()), to_llu(entry.attribute));
        }
    }
}

int main(int argc, char *argv[])
{
    arg_parser cmd(argc, argv);
    if(cmd.has_option("help")){
        cmd_help();
    }
    else if(cmd.has_option("create-db")){
        cmd_create_db(cmd);
    }
    else if(cmd.has_option("list")){
        cmd_list(cmd);
    }
    else if(cmd.has_option("decomp-db")){
        cmd_uncomp_db(cmd);
    }
    else{
        cmd_help();
    }
    return 0;
}
