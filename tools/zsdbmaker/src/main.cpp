#include <regex>
#include <string>
#include <charconv>
#include <zstd.h>
#include <zdict.h>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <cinttypes>
#include "strf.hpp"
#include "zsdb.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "argf.hpp"

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
    std::printf("--comp-level=[0|min|max|<integer>], default 0 (zstd default level)\n");
    std::printf("--comp-ratio-threshold=[0-1], default 0.9, store compressed only if compressed/raw size is below it\n");
    std::printf("--train-dict=<dict file>, train a zstd dictionary from --input-dir and --input-file-name-regex\n");
    std::printf("--dict-size=<bytes>, default 112640, dictionary size for --train-dict\n");
}

static void cmd_train_dict(const argf::parser &cmd)
{
    const auto getOptStr = [&cmd](const char *opt, const char *defVal) -> std::string
    {
        if(!cmd.has_option(opt)){
            if(defVal){
                return defVal;
            }
            throw fflpanic("option --{} is required", opt);
        }

        if(cmd[opt] || cmd(opt).str().empty()){
            throw fflpanic("option --{} requires an argument", opt);
        }
        return cmd(opt).str();
    };

    const auto dictFileName = getOptStr("train-dict"           , nullptr);
    const auto inputDirName = getOptStr("input-dir"            , "."    );
    const auto fileNameRegex = getOptStr("input-file-name-regex", ".*"   );

    const auto dictSize = [&getOptStr]() -> size_t
    {
        const auto sizeStr = getOptStr("dict-size", "112640");
        size_t size = 0;

        if(const auto [ptr, ec] = std::from_chars(sizeStr.data(), sizeStr.data() + sizeStr.size(), size); ec != std::errc{} || ptr != sizeStr.data() + sizeStr.size() || size < 256){
            throw fflpanic("invalid dict-size: {}", sizeStr);
        }
        return size;
    }();

    std::vector<std::filesystem::path> pathList;
    const std::regex fileNameMatchRegex(fileNameRegex);

    for(const auto &p: std::filesystem::directory_iterator(inputDirName)){
        if(p.is_regular_file() && std::regex_match(reinterpret_cast<const char *>(p.path().filename().u8string().c_str()), fileNameMatchRegex)){
            pathList.push_back(p.path());
        }
    }

    // sorted and evenly strided, so the same input always trains the same dict
    std::sort(pathList.begin(), pathList.end());

    constexpr size_t maxSampleSize = 128 * 1024;
    constexpr size_t maxTotalSampleSize = 64 * 1024 * 1024;

    size_t totalSize = 0;
    for(const auto &path: pathList){
        totalSize += std::min<size_t>(maxSampleSize, std::filesystem::file_size(path));
    }

    const size_t stride = std::max<size_t>(1, (totalSize + maxTotalSampleSize - 1) / maxTotalSampleSize);

    std::vector<uint8_t> sampleBuf;
    std::vector<size_t> sampleSizeList;

    for(size_t i = 0; i < pathList.size(); i += stride){
        std::ifstream f(pathList[i], std::ios::binary);
        std::vector<char> buf(maxSampleSize);

        f.read(buf.data(), buf.size());
        if(const auto readSize = to_uz(f.gcount()); readSize > 0){
            sampleBuf.insert(sampleBuf.end(), buf.begin(), buf.begin() + readSize);
            sampleSizeList.push_back(readSize);
        }
    }

    if(sampleSizeList.empty()){
        throw fflpanic("no sample found in {}", inputDirName);
    }

    std::vector<uint8_t> dictBuf(dictSize);
    const auto rc = ZDICT_trainFromBuffer(dictBuf.data(), dictBuf.size(), sampleBuf.data(), sampleSizeList.data(), to_u32(sampleSizeList.size()));

    if(ZDICT_isError(rc)){
        throw fflpanic("failed to train dict from {}: {}", inputDirName, ZDICT_getErrorName(rc));
    }

    dictBuf.resize(rc);
    std::ofstream(dictFileName, std::ios::binary).write(reinterpret_cast<const char *>(dictBuf.data()), dictBuf.size());
    std::printf("trained dict %s: %zu bytes from %zu samples, %zu bytes\n", dictFileName.c_str(), dictBuf.size(), sampleSizeList.size(), sampleBuf.size());
}

static void cmd_create_db(const argf::parser &cmd)
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
            throw fflpanic("input-file-name-regex requires an argument");
        }
        return cmd("input-file-name-regex").str();
    }();

    const auto inputDirName = [&cmd]() -> std::string
    {
        if(!cmd.has_option("input-dir")){
            return ".";
        }

        if(cmd["input-dir"] || cmd("input-dir").str().empty()){
            throw fflpanic("input-dir requires an argument");
        }
        return cmd("input-dir").str();
    }();

    const auto inputDictName = [&cmd]() -> std::string
    {
        if(!cmd.has_option("input-dict")){
            return "";
        }

        if(cmd["input-dict"] || cmd("input-dict").str().empty()){
            throw fflpanic("input-dict requires an argument");
        }
        return cmd("input-dict").str();
    }();

    const auto compLevel = [&cmd]() -> int
    {
        if(!cmd.has_option("comp-level")){
            return 0;
        }

        if(cmd["comp-level"] || cmd("comp-level").str().empty()){
            throw fflpanic("comp-level requires an argument");
        }

        const auto levelStr = cmd("comp-level").str();
        if(levelStr == "min"){
            return ZSTD_minCLevel();
        }

        if(levelStr == "max"){
            return ZSTD_maxCLevel();
        }

        int level = 0;
        if(const auto [ptr, ec] = std::from_chars(levelStr.data(), levelStr.data() + levelStr.size(), level); ec != std::errc{} || ptr != levelStr.data() + levelStr.size()){
            throw fflpanic("invalid comp-level: {}", levelStr);
        }

        if(level != 0){
            if(level < ZSTD_minCLevel() || level > ZSTD_maxCLevel()){
                throw fflpanic("comp-level {} out of range [{}, {}]", level, ZSTD_minCLevel(), ZSTD_maxCLevel());
            }
        }
        return level;
    }();

    const auto compRatioThreshold = [&cmd]() -> double
    {
        if(!cmd.has_option("comp-ratio-threshold")){
            return 0.90;
        }

        if(cmd["comp-ratio-threshold"] || cmd("comp-ratio-threshold").str().empty()){
            throw fflpanic("comp-ratio-threshold requires an argument");
        }

        const auto thresholdStr = cmd("comp-ratio-threshold").str();
        double threshold = 0.0;

        if(const auto [ptr, ec] = std::from_chars(thresholdStr.data(), thresholdStr.data() + thresholdStr.size(), threshold); ec != std::errc{} || ptr != thresholdStr.data() + thresholdStr.size()){
            throw fflpanic("invalid comp-ratio-threshold: {}", thresholdStr);
        }

        if(!(threshold >= 0.0 && threshold <= 1.0)){
            throw fflpanic("comp-ratio-threshold {} out of range [0, 1]", threshold);
        }
        return threshold;
    }();

    ZSDB::buildDB(outputFileName.c_str(), inputFileNameRegex.c_str(), inputDirName.c_str(), inputDictName.c_str(), compLevel, compRatioThreshold);
}

static void cmd_list(const argf::parser &cmd)
{
    auto dbFileName = [&cmd]() -> std::string
    {
        if(cmd["list"] || cmd("list").str().empty()){
            throw fflpanic("option list requires an argument");
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
        throw fflpanic("option --data-name-regex requires an argument");
    }();

    std::regex fileNameReg(regexStr.empty() ? ".*" : regexStr.c_str());
    for(const auto &entry: entryList){
        if(!regexStr.empty()){
            if(!std::regex_match(entry.fileName, fileNameReg)){
                continue;
            }
        }
        std::printf("%-32s %8zu %8llu\n", entry.fileName, entry.length, to_llu(entry.attribute));
    }
}

static void cmd_uncomp_db(const argf::parser &cmd)
{
    const auto dbFileName = [&cmd]() -> std::string
    {
        if(cmd["decomp-db"] || cmd("decomp-db").str().empty()){
            throw fflpanic("option --decomp-db requires an argument");
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
                throw fflpanic("failed to write to file: {}, err = {}", fileName, std::strerror(errno));
            }
            std::printf("%-32s %8zu -> %8zu [%3d%%] %8llu\n", entry.fileName, entry.length, readBuf.size(), to_d(entry.length * 100 / readBuf.size()), to_llu(entry.attribute));
        }
    }
}

int main(int argc, char *argv[])
{
    argf::parser cmd(argc, argv);
    if(cmd.has_option("help")){
        cmd_help();
    }
    else if(cmd.has_option("create-db")){
        cmd_create_db(cmd);
    }
    else if(cmd.has_option("train-dict")){
        cmd_train_dict(cmd);
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
