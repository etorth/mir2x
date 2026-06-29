#include <regex>
#include <zstd.h>
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
        throw fflpanic("invalid arguments: dataBuf = {:p}, dataLen = {}", to_cvptr(dataBuf), dataLen);
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
        throw fflpanic("failed to compress file: {}", ZSTD_getErrorName(rc));
    }

    result.resize(rc);
    return result;
}

static std::vector<uint8_t> decompressDataBuf(const uint8_t *dataBuf, size_t dataLen, ZSTD_DCtx *dctxPtr, const ZSTD_DDict *ddictPtr)
{
    if(!(dataBuf && dataLen)){
        throw fflpanic("invalid arguments: dataBuf = {:p}, dataLen = {}", to_cvptr(dataBuf), dataLen);
    }

    std::vector<uint8_t> result;
    switch(auto origSize = ZSTD_getFrameContentSize(dataBuf, dataLen)){
        case ZSTD_CONTENTSIZE_ERROR:
            {
                throw fflpanic("ZSTD_CONTENTSIZE_UNKNOWN");
            }
        case ZSTD_CONTENTSIZE_UNKNOWN:
            {
                throw fflpanic("ZSTD_CONTENTSIZE_ERROR");
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
        throw fflpanic("failed to compress file: {}", ZSTD_getErrorName(rc));
    }

    result.resize(rc);
    return result;
}

static std::vector<uint8_t> readFileData(const char *filePath)
{
    auto fp = make_fileptr(filePath, "rb");
    return read_fileptr<std::vector<uint8_t>>(fp);
}

static std::vector<uint8_t> readFileOffData(fileptr_t &fp, size_t dataOff, size_t dataLen)
{
    seek_fileptr(fp, check_cast<int64_t>(dataOff), SEEK_SET);
    return read_fileptr<std::vector<uint8_t>>(fp, dataLen);
}

static std::vector<uint8_t> decompFileOffData(fileptr_t &fp, size_t dataOff, size_t dataLen, ZSTD_DCtx *dctxPtr, const ZSTD_DDict *ddictPtr)
{
    const auto compBuf = readFileOffData(fp, dataOff, dataLen);
    if(compBuf.empty()){
        throw fflpanic("failed to read file offset block");
    }
    return decompressDataBuf(compBuf.data(), compBuf.size(), dctxPtr, ddictPtr);
}

ZSDB::InnEntry ZSDB::getErrorEntry()
{
    InnEntry errorEntry;
    std::memset(&errorEntry, 0, sizeof(errorEntry));

    errorEntry.offset    = 0X0123456789ABCDEF;
    errorEntry.length    = 0XFEDCBA9876543210;
    errorEntry.fileName  = 0X0123456789ABCDEF;
    errorEntry.attribute = 0XFEDCBA9876543210;
    return errorEntry;
}

ZSDB::ZSDB(const char *filePath)
    : m_fp(make_fileptr(filePath, "rb"))
{
    m_DCtx.reset(ZSTD_createDCtx());
    if(!m_DCtx){
        throw fflpanic("failed to create decompress context");
    }

    read_fileptr(m_fp, &m_header, sizeof(m_header));

    if(std::memcmp(m_header.magic, FORMAT_MAGIC, sizeof(FORMAT_MAGIC))){
        throw fflpanic("not a zsdb file: {}", filePath);
    }

    if(m_header.formatVersion != FORMAT_VERSION){
        throw fflpanic("unsupported zsdb format version: got {}, expected {}", m_header.formatVersion, FORMAT_VERSION);
    }

    if(m_header.dictLength){
        const auto offset = check_cast<size_t>(m_header.dictOffset);
        const auto length = check_cast<size_t>(m_header.dictLength);
        // The dict block is itself zstd-compressed without using any dictionary
        // (it's the dictionary we're about to load); decompress with ddict=nullptr.
        if(const auto dictBuf = decompFileOffData(m_fp, offset, length, m_DCtx.get(), nullptr); dictBuf.empty()){
            throw fflpanic("failed to load data at (off = {}, length = {})", offset, length);
        }
        else{
            m_DDict.reset(ZSTD_createDDict(dictBuf.data(), dictBuf.size()));
            if(!m_DDict){
                throw fflpanic("create decompression dictory failed");
            }
        }
    }

    if(m_header.entryLength){
        const auto offset = check_cast<size_t>(m_header.entryOffset);
        const auto length = check_cast<size_t>(m_header.entryLength);
        // Entry table is compressed without dict (see buildDB), so decompress without ddict.
        if(const auto entryBuf = decompFileOffData(m_fp, offset, length, m_DCtx.get(), nullptr); entryBuf.empty()){
            throw fflpanic("failed to load data at (off = {}, length = {})", offset, length);
        }
        else{
            if(entryBuf.size() != (1 + m_header.entryNum) * sizeof(InnEntry)){
                throw fflpanic("zsdb database file corrupted");
            }

            // memcpy rather than reinterpret_cast: entryBuf.data() is byte-aligned but
            // InnEntry has uint64_t members that require 8-byte alignment on strict
            // platforms (ARM, RISC-V). #pragma pack(1) guarantees a contiguous on-disk
            // layout but does not license type-punning.
            m_entryList.resize(m_header.entryNum + 1);
            std::memcpy(m_entryList.data(), entryBuf.data(), entryBuf.size());

            const auto errEntry = getErrorEntry();
            if(std::memcmp(&m_entryList.back(), &errEntry, sizeof(InnEntry))){
                throw fflpanic("zsdb database file corrupted");
            }
            m_entryList.pop_back();
        }
    }

    if(m_header.fileNameLength){
        const auto offset = check_cast<size_t>(m_header.fileNameOffset);
        const auto length = check_cast<size_t>(m_header.fileNameLength);
        if(const auto fileNameBuf = decompFileOffData(m_fp, offset, length, m_DCtx.get(), nullptr); fileNameBuf.empty()){
            throw fflpanic("failed to load data at (off = {}, length = {})", offset, length);
        }
        else{
            m_fileNameBuf.assign(fileNameBuf.begin(), fileNameBuf.end());
        }
    }
}

const char *ZSDB::decomp(const char *fileName, size_t checkLen, std::vector<uint8_t> *dstBuf)
{
    if(!str_haschar(fileName)){
        throw fflpanic("invalid arguments: {}", fileName);
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
        result = decompFileOffData(m_fp, m_header.streamOffset + entry.offset, entry.length, m_DCtx.get(), m_DDict.get());
    }
    else{
        result = readFileOffData(m_fp, m_header.streamOffset + entry.offset, entry.length);
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
    if(!(str_haschar(savePath) && str_haschar(dataPath))){
        throw fflpanic("invalid arguments: savePath = {}, dataPath = {}", to_cstr(savePath), to_cstr(dataPath));
    }

    std::unique_ptr<ZSTD_CDict, size_t (*)(ZSTD_CDict *)> cdictPtr{nullptr, ZSTD_freeCDict};
    std::vector<uint8_t> cdictBuf;

    if(str_haschar(dictPath)){
        cdictBuf = readFileData(dictPath);
        if(cdictBuf.empty()){
            throw fflpanic("dict file is empty: {}", dictPath);
        }

        cdictPtr.reset(ZSTD_createCDict(cdictBuf.data(), cdictBuf.size(), g_compLevel));
        if(!cdictPtr){
            throw fflpanic("failed to create dict: {}", dictPath);
        }
    }

    std::unique_ptr<ZSTD_CCtx, size_t (*)(ZSTD_CCtx *)> cctxPtr{ZSTD_createCCtx(), ZSTD_freeCCtx};
    if(!cctxPtr){
        throw fflpanic("failed to create ZSTD_CCtx");
    }

    std::vector<char> fileNameBuf;
    std::vector<uint8_t> streamBuf;
    std::vector<InnEntry> entryList;

    size_t entryCount = 0;
    std::regex fileNameMatchRegex(str_haschar(fileNameRegex) ? fileNameRegex : ".*");

    for(auto &p: std::filesystem::directory_iterator(dataPath)){
        if(!p.is_regular_file()){
            continue;
        }

        auto fileName = p.path().filename().u8string();
        if(str_haschar(fileNameRegex)){
            if(!std::regex_match(reinterpret_cast<const char *>(fileName.c_str()), fileNameMatchRegex)){
                continue;
            }
        }

        const auto srcBuf = readFileData(reinterpret_cast<const char *>(p.path().u8string().c_str()));
        if(srcBuf.empty()){
            continue;
        }

        const auto dstBuf = compressDataBuf(srcBuf.data(), srcBuf.size(), cctxPtr.get(), cdictPtr.get());
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

    // The dict block is stored zstd-compressed (without using itself as a dictionary,
    // for obvious bootstrap reasons). Skip emitting any dict bytes when no dict was
    // provided — header.dictLength == 0 then signals the reader to skip the block.
    const auto cdictCompBuf = cdictBuf.empty()
        ? std::vector<uint8_t>{}
        : compressDataBuf(cdictBuf.data(), cdictBuf.size(), cctxPtr.get(), nullptr);

    ZSDBHeader header;
    std::memset(&header, 0, sizeof(header));

    std::memcpy(header.magic, FORMAT_MAGIC, sizeof(FORMAT_MAGIC));
    header.formatVersion = FORMAT_VERSION;
    header.entryNum      = entryCount;

    header.dictOffset = sizeof(header);
    header.dictLength = cdictCompBuf.size();

    const auto entryCompBuf = compressDataBuf((uint8_t *)(entryList.data()), entryList.size() * sizeof(InnEntry), cctxPtr.get(), nullptr);
    header.entryOffset = header.dictOffset + header.dictLength;
    header.entryLength = entryCompBuf.size();

    const auto fileNameCompBuf = compressDataBuf((uint8_t *)(fileNameBuf.data()), fileNameBuf.size(), cctxPtr.get(), nullptr);
    header.fileNameOffset = header.entryOffset + header.entryLength;
    header.fileNameLength = fileNameCompBuf.size();

    header.streamOffset = header.fileNameOffset + header.fileNameLength;
    header.streamLength = streamBuf.size();

    auto fp = make_fileptr(savePath, "wb");
    write_fileptr(fp, header);
    if(!cdictCompBuf.empty()){
        write_fileptr(fp, cdictCompBuf);
    }
    write_fileptr(fp, entryCompBuf);
    write_fileptr(fp, fileNameCompBuf);
    write_fileptr(fp, streamBuf);
    close_fileptr(fp);
}
