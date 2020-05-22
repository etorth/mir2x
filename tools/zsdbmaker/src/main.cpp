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
#include "zsdb.hpp"
#include "argparser.hpp"

static int cmd_help()
{
    std::printf("--help\n");
    std::printf("--create-db\n");
    std::printf("--list\n");
    std::printf("--decomp-db\n");
    std::printf("--input-data-dir\n");
    std::printf("--input-dict\n");

    return 0;
}

static bool has_option(const argh::parser &cmd, const std::string &opt)
{
    if(opt.empty()){
        throw std::invalid_argument("call has_option() with empty option");
    }

    return cmd[opt] || cmd(opt);
}

static int cmd_create_db(const argh::parser &cmd)
{
    auto szDBOutputName = [&cmd]() -> std::string
    {
        if(cmd["create-db"]){
            return "out.zsdb";
        }else{
            if(cmd("create-db").str().empty()){
                return "out.zsdb";
            }else{
                return cmd("create-db").str();
            }
        }
    }();

    auto szFileNameRegex = [&cmd]() -> std::string
    {
        if(!has_option(cmd, "input-file-name-regex")){
            return "";
        }

        if(cmd["input-file-name-regex"] || cmd("input-file-name").str().empty()){
            throw std::invalid_argument("input-file-name-regex requires an argument");
        }

        return cmd("input-file-name").str();
    }();

    auto szDBInputDirName = [&cmd]() -> std::string
    {
        if(!has_option(cmd, "input-data-dir")){
            return ".";
        }

        if(cmd["input-data-dir"] || cmd("input-data-dir").str().empty()){
            throw std::invalid_argument("input-data-dir requires an argument");
        }

        return cmd("input-data-dir").str();
    }();

    auto szDictInputName = [&cmd]() -> std::string
    {
        if(!has_option(cmd, "input-dict")){
            return "";
        }

        if(cmd["input-dict"] || cmd("input-dict").str().empty()){
            throw std::invalid_argument("input-dict requires an argument");
        }

        return cmd("input-dict").str();
    }();

    auto fCompressThreshold = [&cmd]() -> double
    {
        if(!has_option(cmd, "compress-threshold")){
            return 0.90;
        }

        if(cmd["compress-threshold"] || cmd("compress-threshold").str().empty()){
            throw std::invalid_argument("compress-threshold requires an argument");
        }

        double fCompressThreshold; cmd("compress-threshold", 0.90) >> fCompressThreshold;
        return fCompressThreshold;
    }();

    if(ZSDB::BuildDB(
                szDBOutputName.c_str(),
                szFileNameRegex.empty() ? nullptr : szFileNameRegex.c_str(),
                szDBInputDirName.c_str(),
                szDictInputName.empty() ? nullptr : szDictInputName.c_str(),
                fCompressThreshold
                )){
        return 0;
    }

    std::printf("build zsdb failed...\n");
    return -1;
}

static int cmd_list(const argh::parser &cmd)
{
    auto szDBFileName = [&cmd]() -> std::string
    {
        if(cmd["list"] || cmd("list").str().empty()){
            throw std::invalid_argument("option list requires an argument");
        }

        return cmd("list").str();
    }();

    ZSDB stZSDB(szDBFileName.c_str());
    auto stEntryList = stZSDB.GetEntryList();

    auto szRegex = [&cmd]() -> std::string
    {
        if(!has_option(cmd, "data-name-regex")){
            return "";
        }

        if(auto szRegex = cmd("data-name-regex").str(); !szRegex.empty()){
            return szRegex;
        }

        throw std::invalid_argument("option --data-name-regex requires an argument");
    }();

    std::regex stFileNameReg(szRegex.empty() ? ".*" : szRegex.c_str());
    for(auto rstEntry: stEntryList){
        if(!szRegex.empty()){
            if(!std::regex_match(rstEntry.FileName, stFileNameReg)){
                continue;
            }
        }
        std::printf("%32s %8zu %8" PRIu64 "\n", rstEntry.FileName, rstEntry.Length, rstEntry.Attribute);
    }
    return 0;
}

static int cmd_uncomp_db(const argh::parser &cmd)
{
    auto szDBFileName = [&cmd]() -> std::string
    {
        if(cmd["decomp-db"] || cmd("decomp-db").str().empty()){
            throw std::invalid_argument("option --decomp-db requires an argument");
        }

        return cmd("decomp-db").str();
    }();

    auto szDataNameRegex = [&cmd]() -> std::string
    {
        if(cmd["decomp-db"] || cmd("decomp-db").str().empty()){
            throw std::invalid_argument("option --decomp-db requires an argument");
        }

        return cmd("decomp-db").str();
    }();


    ZSDB stZSDB(szDBFileName.c_str());
    auto stEntryList = stZSDB.GetEntryList();

    for(auto rstEntry: stEntryList){
        std::vector<uint8_t> stReadBuf;
        if(stZSDB.Decomp(rstEntry.FileName, 0, &stReadBuf)){
            std::ofstream f(rstEntry.FileName, std::ios::out | std::ios::binary);
            f.write((const char *)(stReadBuf.data()), stReadBuf.size());
            std::printf("%32s %8zu -> %8zu [%3d%%] %8" PRIu64 "\n", rstEntry.FileName, rstEntry.Length, stReadBuf.size(), (int)(rstEntry.Length * 100 / stReadBuf.size()), rstEntry.Attribute);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    try{
        arg_parser cmd(argc, argv);
        if(cmd.has_option("help")){
            return cmd_help();
        }

        if(cmd.has_option("create-db")){
            return cmd_create_db(cmd);
        }

        if(cmd.has_option("list")){
            return cmd_list(cmd);
        }

        if(has_option(cmd, "decomp-db")){
            return cmd_uncomp_db(cmd);
        }

    }catch(std::exception &e){
        std::printf("%s\n", e.what());
        return -1;
    }
    return 0;
}
