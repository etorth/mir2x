/*
 * =====================================================================================
 *
 *       Filename: zsdb.cpp
 *        Created: 11/13/2018 22:58:28
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
#include <zstd.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <cinttypes>
#include <algorithm>
#include <filesystem>

#include "zsdb.hpp"
#include "fileptr.hpp"
#include "fflerror.hpp"
#include "totype.hpp"

static constexpr int g_compLevel = 3;
static std::vector<uint8_t> compressDataBuf(const uint8_t *dataBuf, size_t dataLen, ZSTD_CCtx *cctxPtr, const ZSTD_CDict *cdictPtr)
{
    if(!(dataBuf && dataLen)){
        throw fflerror("invalid arguments: dataBuf = %p, dataLen = %zu", to_cvptr(dataBuf), dataLen);
    }

    size_t rc = 0;
    std::vector<uint8_t> result(ZSTD_compressBound(dataLen), 0);

    if(cctxPtr && cdictPtr){
        rc = ZSTD_compress_usingCDict(cctxPtr, result.data(), result.size(), dataBuf, dataLen, cdictPtr);
    }
    else if(cctxPtr){
        rc = ZSTD_compressCCtx(cctxPtr, result.data(), result.size(), dataBuf, dataLen, g_compLevel);
    }
    else{
        rc = ZSTD_compress(result.data(), result.size(), dataBuf, dataLen, g_compLevel);
    }

    if(ZSTD_isError(rc)){
        throw fflerror("failed to compress file: %s", ZSTD_getErrorName(rc));
    }

    result.resize(rc);
    return result;
}

static std::vector<uint8_t> decompressDataBuf(const uint8_t *dataBuf, size_t dataLen, ZSTD_DCtx *dctxPtr, const ZSTD_DDict *ddictPtr)
{
    if(!(dataBuf && dataLen)){
        throw fflerror("invalid arguments: dataBuf = %p, dataLen = %zu", to_cvptr(dataBuf), dataLen);
    }

    std::vector<uint8_t> result;
    switch(auto origSize = ZSTD_getFrameContentSize(dataBuf, dataLen)){
        case ZSTD_CONTENTSIZE_ERROR:
            {
                throw fflerror("ZSTD_CONTENTSIZE_UNKNOWN");
            }
        case ZSTD_CONTENTSIZE_UNKNOWN:
            {
                throw fflerror("ZSTD_CONTENTSIZE_ERROR");
            }
        default:
            {
                result.resize(origSize);
                break;
            }
    }

    size_t rc = 0;
    if(dctxPtr && ddictPtr){
        rc = ZSTD_decompress_usingDDict(dctxPtr, result.data(), result.size(), dataBuf, dataLen, ddictPtr);
    }
    else if(dctxPtr){
        rc = ZSTD_decompressDCtx(dctxPtr, result.data(), result.size(), dataBuf, dataLen);
    }
    else{
        rc = ZSTD_decompress(result.data(), result.size(), dataBuf, dataLen);
    }

    if(ZSTD_isError(rc)){
        throw fflerror("failed to compress file: %s", ZSTD_getErrorName(rc));
    }

    result.resize(rc);
    return result;
}

static std::vector<uint8_t> readFileData(const char *filePath)
{
    if(!filePath){
        throw fflerror("invalid filePath: (null)");
    }

    auto fp = make_fileptr(filePath, "rb");
    std::fseek(fp.get(), 0, SEEK_END);
    const auto fileSize = std::ftell(fp.get());
    std::fseek(fp.get(), 0, SEEK_SET);

    std::vector<uint8_t> dataBuf(fileSize, 0);
    if(std::fread(dataBuf.data(), fileSize, 1, fp.get()) != 1){
        throw fflerror("failed to read file: %s, err = %s", filePath, std::strerror(errno));
    }
    return dataBuf;
}

static std::vector<uint8_t> readFileOffData(std::FILE *fp, size_t dataOff, size_t dataLen)
{
    if(!fp){
        throw fflerror("invalid argument: fp = nullptr");
    }

    std::vector<uint8_t> readBuf(dataLen);
    if(std::fseek(fp, dataOff, SEEK_SET)){
        throw fflerror("failed to seek file: err = %s", std::strerror(errno));
    }

    if(std::fread(readBuf.data(), readBuf.size(), 1, fp) != 1){
        throw fflerror("failed to read file: err = %s", std::strerror(errno));
    }
    return readBuf;
}

static std::vector<uint8_t> decompFileOffData(std::FILE *fp, size_t dataOff, size_t dataLen, ZSTD_DCtx *dctxPtr, const ZSTD_DDict *ddictPtr)
{
    const auto compBuf = readFileOffData(fp, dataOff, dataLen);
    if(compBuf.empty()){
        throw fflerror("failed to read file offset block");
    }
    return decompressDataBuf(compBuf.data(), compBuf.size(), dctxPtr, ddictPtr);
}

const ZSDB::InnEntry &ZSDB::getErrorEntry()
{
    const static auto s_errorEntry = []() -> InnEntry
    {
        InnEntry errorEntry;
        std::memset(&errorEntry, 0, sizeof(errorEntry));

        errorEntry.offset    = 0X0123456789ABCDEF;
        errorEntry.length    = 0XFEDCBA9876543210;
        errorEntry.fileName  = 0X0123456789ABCDEF;
        errorEntry.attribute = 0XFEDCBA9876543210;
        return errorEntry;
    }();
    return s_errorEntry;
}

ZSDB::ZSDB(const char *filePath)
    : m_fp(make_fileptr(filePath, "rb"))
{
    m_DCtx = ZSTD_createDCtx();
    if(!m_DCtx){
        throw fflerror("failed to create decompress context");
    }

    if(const auto headerBuf = readFileOffData(m_fp.get(), 0, sizeof(ZSDBHeader)); headerBuf.empty()){
        throw fflerror("failed to load zsdb header");
    }
    else{
        std::memcpy(&m_header, headerBuf.data(), sizeof(m_header));
    }

    if(m_header.dictLength){
        const auto offset = check_cast<size_t>(m_header.dictOffset);
        const auto length = check_cast<size_t>(m_header.dictLength);
        if(const auto dictBuf = decompFileOffData(m_fp.get(), offset, length, m_DCtx, m_DDict); dictBuf.empty()){
            throw fflerror("failed to load data at (off = %zu, length = %zu)", offset, length);
        }
        else{
            m_DDict = ZSTD_createDDict(dictBuf.data(), dictBuf.size());
            if(!m_DDict){
                throw fflerror("create decompression dictory failed");
            }
        }
    }

    if(m_header.entryLength){
        const auto offset = check_cast<size_t>(m_header.entryOffset);
        const auto length = check_cast<size_t>(m_header.entryLength);
        if(const auto entryBuf = decompFileOffData(m_fp.get(), offset, length, m_DCtx, m_DDict); entryBuf.empty()){
            throw fflerror("failed to load data at (off = %zu, length = %zu)", offset, length);
        }
        else{
            if(entryBuf.size() != (1 + m_header.entryNum) * sizeof(InnEntry)){
                throw fflerror("zsdb database file corrupted");
            }

            const auto *headPtr = (InnEntry *)(entryBuf.data());
            m_entryList.clear();
            m_entryList.insert(m_entryList.end(), headPtr, headPtr + m_header.entryNum + 1);

            if(std::memcmp(&m_entryList.back(), &getErrorEntry(), sizeof(InnEntry))){
                throw fflerror("zsdb database file corrupted");
            }
            m_entryList.pop_back();
        }
    }

    if(m_header.fileNameLength){
        const auto offset = check_cast<size_t>(m_header.fileNameOffset);
        const auto length = check_cast<size_t>(m_header.fileNameLength);
        if(const auto fileNameBuf = decompFileOffData(m_fp.get(), offset, length, m_DCtx, nullptr); fileNameBuf.empty()){
            throw fflerror("failed to load data at (off = %zu, length = %zu)", offset, length);
        }
        else{
            const auto *headPtr = (char *)(fileNameBuf.data());
            m_fileNameBuf.clear();
            m_fileNameBuf.insert(m_fileNameBuf.end(), headPtr, headPtr + fileNameBuf.size());
        }
    }
}

ZSDB::~ZSDB()
{
    ZSTD_freeDCtx(m_DCtx);
    ZSTD_freeDDict(m_DDict);
}

const char *ZSDB::decomp(const char *fileName, size_t checkLen, std::vector<uint8_t> *dstBuf)
{
    if(!str_nonempty(fileName)){
        throw fflerror("invalid arguments: %s", fileName);
    }

    auto p = std::lower_bound(m_entryList.cbegin(), m_entryList.cend(), fileName, [this, checkLen](const InnEntry &lhs, const char *rhs) -> bool
    {
        if(checkLen){
            return std::strncmp(m_fileNameBuf.data() + lhs.fileName, rhs, checkLen) < 0;
        }
        else{
            return std::strcmp(m_fileNameBuf.data() + lhs.fileName, rhs) < 0;
        }
    });

    if(p == m_entryList.cend()){
        return nullptr;
    }

    if(checkLen){
        if(std::strncmp(fileName, m_fileNameBuf.data() + p->fileName, checkLen)){
            return nullptr;
        }
    }else{
        if(std::strcmp(fileName, m_fileNameBuf.data() + p->fileName)){
            return nullptr;
        }
    }
    return ZSDB::decompEntry(*p, dstBuf) ? (m_fileNameBuf.data() + p->fileName) : nullptr;
}

bool ZSDB::decompEntry(const ZSDB::InnEntry &entry, std::vector<uint8_t> *dstBuf)
{
    if(!dstBuf){
        return false;
    }

    if(!entry.length){
        dstBuf->clear();
        return true;
    }

    std::vector<uint8_t> result;
    if(entry.attribute & F_COMPRESSED){
        result = decompFileOffData(m_fp.get(), m_header.streamOffset + entry.offset, entry.length, m_DCtx, m_DDict);
    }
    else{
        result = readFileOffData(m_fp.get(), m_header.streamOffset + entry.offset, entry.length);
    }

    if(result.empty()){
        return false;
    }

    dstBuf->swap(result);
    return true;
}

std::vector<ZSDB::Entry> ZSDB::getEntryList() const
{
    std::vector<ZSDB::Entry> result;
    for(auto &entry: m_entryList){
        result.emplace_back(m_fileNameBuf.data() + entry.fileName, entry.length, entry.attribute);
    }
    return result;
}

void ZSDB::buildDB(const char *savePath, const char *fileNameRegex, const char *dataPath, const char *dictPath, double compRatio)
{
    if(!(str_nonempty(savePath) && str_nonempty(dataPath))){
        throw fflerror("invalid arguments: savePath = %s, dataPath = %s", to_cstr(savePath), to_cstr(dataPath));
    }

    ZSTD_CDict *cdictPtr = nullptr;
    std::vector<uint8_t> cdictBuf;

    if(dictPath){
        cdictBuf = readFileData(dictPath);
        if(cdictBuf.empty()){
            throw fflerror("dict file is empty: %s", dictPath);
        }

        cdictPtr = ZSTD_createCDict(cdictBuf.data(), cdictBuf.size(), g_compLevel);
        if(!cdictPtr){
            throw fflerror("failed to create dict: %s", dictPath);
        }
    }

    ZSTD_CCtx *cctxPtr = ZSTD_createCCtx();
    if(!cctxPtr){
        throw fflerror("failed to create ZSTD_CCtx");
    }

    std::vector<char> fileNameBuf;
    std::vector<uint8_t> streamBuf;
    std::vector<InnEntry> entryList;

    size_t entryCount = 0;
    std::regex fileNameMatchRegex(fileNameRegex ? fileNameRegex : ".*");

    for(auto &p: std::filesystem::directory_iterator(dataPath)){
        if(!p.is_regular_file()){
            continue;
        }

        auto fileName = p.path().filename().u8string();
        if(fileNameRegex){
            if(!std::regex_match(reinterpret_cast<const char *>(fileName.c_str()), fileNameMatchRegex)){
                continue;
            }
        }

        const auto srcBuf = readFileData(reinterpret_cast<const char *>(p.path().u8string().c_str()));
        if(srcBuf.empty()){
            continue;
        }

        const auto dstBuf = compressDataBuf(srcBuf.data(), srcBuf.size(), cctxPtr, cdictPtr);
        if(dstBuf.empty()){
            continue;
        }

        const bool compressed = ((1.00 * dstBuf.size() / srcBuf.size()) < compRatio);
        const auto &tookBuf = compressed ? dstBuf : srcBuf;

        InnEntry entry;
        std::memset(&entry, 0, sizeof(entry));

        entry.offset = streamBuf.size();
        entry.length = tookBuf.size();
        streamBuf.insert(streamBuf.end(), tookBuf.begin(), tookBuf.end());

        entry.fileName = fileNameBuf.size();
        fileNameBuf.insert(fileNameBuf.end(), fileName.begin(), fileName.end());
        fileNameBuf.push_back('\0');

        if(compressed){
            entry.attribute |= F_COMPRESSED;
        }

        entryList.push_back(entry);
        entryCount++;
    }

    std::sort(entryList.begin(), entryList.end(), [&fileNameBuf](const InnEntry &lhs, const InnEntry &rhs) -> bool
    {
        return std::strcmp(fileNameBuf.data() + lhs.fileName, fileNameBuf.data() + rhs.fileName) < 0;
    });
    entryList.push_back(getErrorEntry());

    ZSDBHeader header;
    std::memset(&header, 0, sizeof(header));

    header.zstdVersion = ZSTD_versionNumber();
    header.entryNum    = entryCount;

    header.dictOffset = sizeof(header);
    header.dictLength = cdictBuf.size();

    const auto entryCompBuf = compressDataBuf((uint8_t *)(entryList.data()), entryList.size() * sizeof(InnEntry), cctxPtr, nullptr);
    header.entryOffset = header.dictOffset + header.dictLength;
    header.entryLength = entryCompBuf.size();

    const auto fileNameCompBuf = compressDataBuf((uint8_t *)(fileNameBuf.data()), fileNameBuf.size(), cctxPtr, nullptr);
    header.fileNameOffset = header.entryOffset + header.entryLength;
    header.fileNameLength = fileNameCompBuf.size();

    header.streamOffset = header.fileNameOffset + header.fileNameLength;
    header.streamLength = streamBuf.size();

    auto fp = make_fileptr(savePath, "wb");
    if(std::fwrite(&header, sizeof(header), 1, fp.get()) != 1){
        throw fflerror("failed to save zsdb header: %s", savePath);
    }

    if(std::fwrite(entryCompBuf.data(), entryCompBuf.size(), 1, fp.get()) != 1){
        throw fflerror("failed to save zsdb entry buffer: %s", savePath);
    }

    if(std::fwrite(fileNameCompBuf.data(), fileNameCompBuf.size(), 1, fp.get()) != 1){
        throw fflerror("failed to save zsdb file name buffer: %s", savePath);
    }

    if(std::fwrite(streamBuf.data(), streamBuf.size(), 1, fp.get()) != 1){
        throw fflerror("failed to save zsdb stream buffer: %s", savePath);
    }
}
