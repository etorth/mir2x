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
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <cinttypes>
#include <algorithm>
#include <filesystem>

#include <zstd.h>
#include "zsdb.hpp"

const static int g_CompLevel = 3;

template<typename T, typename F> static T check_cast(F from)
{
    auto to = static_cast<T>(from);
    if(static_cast<F>(to) != from){
        throw std::runtime_error("cast fails to preserve original value");
    }
    return to;
}

static std::vector<uint8_t> compressDataBuf(const uint8_t *pDataBuf, size_t nDataLen, ZSTD_CCtx *pCCtx, const ZSTD_CDict *pCDict)
{
    if(!pDataBuf || !nDataLen){
        return {};
    }

    size_t nRC = 0;
    std::vector<uint8_t> stRetBuf(ZSTD_compressBound(nDataLen), 0);

    if(pCCtx && pCDict){
        nRC = ZSTD_compress_usingCDict(pCCtx, stRetBuf.data(), stRetBuf.size(), pDataBuf, nDataLen, pCDict);
    }else if(pCCtx){
        nRC = ZSTD_compressCCtx(pCCtx, stRetBuf.data(), stRetBuf.size(), pDataBuf, nDataLen, g_CompLevel);
    }else{
        nRC = ZSTD_compress(stRetBuf.data(), stRetBuf.size(), pDataBuf, nDataLen, g_CompLevel);
    }

    if(ZSTD_isError(nRC)){
        return {};
    }

    stRetBuf.resize(nRC);
    return stRetBuf;
}

static std::vector<uint8_t> decompressDataBuf(const uint8_t *pDataBuf, size_t nDataLen, ZSTD_DCtx *pDCtx, const ZSTD_DDict *pDDict)
{
    if(!pDataBuf || !nDataLen){
        return {};
    }

    std::vector<uint8_t> stRetBuf;
    switch(auto nDecompSize = ZSTD_getFrameContentSize(pDataBuf, nDataLen)){
        case ZSTD_CONTENTSIZE_ERROR:
        case ZSTD_CONTENTSIZE_UNKNOWN:
            {
                return {};
            }
        default:
            {
                stRetBuf.resize(nDecompSize);
                break;
            }
    }

    size_t nRC = 0;
    if(pDCtx && pDDict){
        nRC = ZSTD_decompress_usingDDict(pDCtx, stRetBuf.data(), stRetBuf.size(), pDataBuf, nDataLen, pDDict);
    }else if(pDCtx){
        nRC = ZSTD_decompressDCtx(pDCtx, stRetBuf.data(), stRetBuf.size(), pDataBuf, nDataLen);
    }else{
        nRC = ZSTD_decompress(stRetBuf.data(), stRetBuf.size(), pDataBuf, nDataLen);
    }

    if(ZSTD_isError(nRC)){
        return {};
    }

    stRetBuf.resize(nRC);
    return stRetBuf;
}

static std::vector<uint8_t> readFileData(const char *szPath)
{
    if(!szPath){
        return {};
    }

    std::FILE *fp = std::fopen(szPath, "rb");
    if(!fp){
        return {};
    }

    std::fseek(fp, 0, SEEK_END);
    auto nFileLen = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);

    std::vector<uint8_t> stDataBuf(nFileLen, 0);
    std::fread(stDataBuf.data(), nFileLen, 1, fp);
    return stDataBuf;
}

static std::vector<uint8_t> readFileOffData(std::FILE *fp, size_t nDataOff, size_t nDataLen)
{
    if(!fp){
        return {};
    }

    std::vector<uint8_t> stReadBuf(nDataLen);
    if(std::fseek(fp, nDataOff, SEEK_SET)){
        return {};
    }

    if(std::fread(stReadBuf.data(), stReadBuf.size(), 1, fp) != 1){
        return {};
    }

    return stReadBuf;
}

static std::vector<uint8_t> decompFileOffData(std::FILE *fp, size_t nDataOff, size_t nDataLen, ZSTD_DCtx *pDCtx, const ZSTD_DDict *pDDict)
{
    auto stCompBuf = readFileOffData(fp, nDataOff, nDataLen);
    if(stCompBuf.empty()){
        return {};
    }

    return decompressDataBuf(stCompBuf.data(), stCompBuf.size(), pDCtx, pDDict);
}

const ZSDB::ZSDBEntry &ZSDB::GetErrorEntry()
{
    const static auto s_ErrorEntry = []() -> ZSDBEntry
    {
        ZSDBEntry stErrorEntry;
        std::memset(&stErrorEntry, 0, sizeof(stErrorEntry));

        stErrorEntry.Offset    = 0X0123456789ABCDEF;
        stErrorEntry.Length    = 0XFEDCBA9876543210;
        stErrorEntry.FileName  = 0X0123456789ABCDEF;
        stErrorEntry.Attribute = 0XFEDCBA9876543210;

        return stErrorEntry;
    }();
    return s_ErrorEntry;
}

ZSDB::ZSDB(const char *szPath)
    : m_fp(nullptr)
    , m_DCtx(nullptr)
    , m_DDict(nullptr)
    , m_Header()
    , m_EntryList()
    , m_FileNameBuf()
{
    m_fp = std::fopen(szPath, "rb");
    if(!m_fp){
        throw std::runtime_error("failed to open database file");
    }

    if(auto stHeaderData = readFileOffData(m_fp, 0, sizeof(ZSDBHeader)); stHeaderData.empty()){
        throw std::runtime_error("failed to load izdb header");
    }else{
        std::memcpy(&m_Header, stHeaderData.data(), sizeof(m_Header));
    }

    if(m_Header.DictLength){
        auto nOffset = check_cast<size_t>(m_Header.DictOffset);
        auto nLength = check_cast<size_t>(m_Header.DictLength);
        if(auto stDictBuf = decompFileOffData(m_fp, nOffset, nLength, m_DCtx, m_DDict); stDictBuf.empty()){
            throw std::runtime_error(std::string("failed to load data at (") + ((std::to_string(nOffset) + ", ") + std::to_string(nLength) + ")"));
        }else{
            m_DDict = ZSTD_createDDict(stDictBuf.data(), stDictBuf.size());
            if(!m_DDict){
                throw std::runtime_error("create decompression dictory failed");
            }
        }
    }

    if(m_Header.EntryLength){
        if(m_Header.EntryLength % sizeof(ZSDBEntry)){
            throw std::runtime_error("zsdb database file corrupted");
        }

        auto nOffset = check_cast<size_t>(m_Header.EntryOffset);
        auto nLength = check_cast<size_t>(m_Header.EntryLength);

        if(auto stEntryBuf = decompFileOffData(m_fp, nOffset, nLength, m_DCtx, m_DDict); stEntryBuf.empty()){
            throw std::runtime_error(std::string("failed to load data at (") + ((std::to_string(nOffset) + ", ") + std::to_string(nLength) + ")"));
        }else{
            if(stEntryBuf.size() != (1 + m_Header.EntryNum) * sizeof(ZSDBEntry)){
                throw std::runtime_error("zsdb database file corrupted");
            }

            auto *pHead = (ZSDBEntry *)(stEntryBuf.data());
            m_EntryList.clear();
            m_EntryList.insert(m_EntryList.end(), pHead, pHead + m_Header.EntryNum + 1);

            if(std::memcmp(&m_EntryList.back(), &GetErrorEntry(), sizeof(ZSDBEntry))){
                throw std::runtime_error("zsdb database file corrupted");
            }
            m_EntryList.pop_back();
        }
    }

    if(m_Header.FileNameLength){
        auto nOffset = check_cast<size_t>(m_Header.FileNameOffset);
        auto nLength = check_cast<size_t>(m_Header.FileNameLength);
        if(auto stFileNameBuf = decompFileOffData(m_fp, nOffset, nLength, m_DCtx, nullptr); stFileNameBuf.empty()){
            throw std::runtime_error(std::string("failed to load data at (") + ((std::to_string(nOffset) + ", ") + std::to_string(nLength) + ")"));
        }else{
            auto *pHead = (char *)(stFileNameBuf.data());
            m_FileNameBuf.clear();
            m_FileNameBuf.insert(m_FileNameBuf.end(), pHead, pHead + stFileNameBuf.size());
        }
    }
}

bool ZSDB::Decomp(const char *szFileName, size_t nCheckLen, std::vector<uint8_t> *pDstBuf)
{
    if(!szFileName || !std::strlen(szFileName)){
        return false;
    }

    auto p = std::lower_bound(m_EntryList.begin(), m_EntryList.end(), szFileName, [this, nCheckLen](const ZSDBEntry &lhs, const char *rhs) -> bool
            {
            if(nCheckLen){
            return std::strncmp(m_FileNameBuf.data() + lhs.FileName, rhs, nCheckLen) < 0;
            }else{
            return std::strcmp(m_FileNameBuf.data() + lhs.FileName, rhs) < 0;
            }
            });

    if(p == m_EntryList.end()){
        return false;
    }

    if(nCheckLen){
        if(std::strncmp(szFileName, m_FileNameBuf.data() + p->FileName, nCheckLen)){
            return false;
        }
    }else{
        if(std::strcmp(szFileName, m_FileNameBuf.data() + p->FileName)){
            return false;
        }
    }

    return ZSDB::DecompEntry(*p, pDstBuf);
}

bool ZSDB::DecompEntry(const ZSDB::ZSDBEntry &rstEntry, std::vector<uint8_t> *pDstBuf)
{
    if(!pDstBuf){
        return false;
    }

    if(!rstEntry.Length){
        pDstBuf->clear();
        return true;
    }

    std::vector<uint8_t> stRetBuf;
    if(rstEntry.Attribute & F_COMPRESSED){
        stRetBuf = decompFileOffData(m_fp, rstEntry.Offset, rstEntry.Length, m_DCtx, m_DDict);
    }else{
        stRetBuf = readFileOffData(m_fp, rstEntry.Offset, rstEntry.Length);
    }

    if(stRetBuf.empty()){
        return false;
    }

    pDstBuf->swap(stRetBuf);
    return true;
}

bool ZSDB::BuildDB(const char *szSaveFullName, const char *szFileNameRegex, const char *szDataPath, const char *szDictPath, double fCompRatio)
{
    if(!szSaveFullName){
        return false;
    }

    if(!szDataPath){
        return false;
    }

    ZSTD_CDict *pCDict = nullptr;
    std::vector<uint8_t> stCDictBuf;

    if(szDictPath){
        stCDictBuf = readFileData(szDictPath);
        if(stCDictBuf.empty()){
            return false;
        }

        pCDict = ZSTD_createCDict(stCDictBuf.data(), stCDictBuf.size(), g_CompLevel);
        if(!pCDict){
            return false;
        }
    }

    ZSTD_CCtx *pCCtx = ZSTD_createCCtx();
    if(!pCCtx){
        return false;
    }

    std::vector<char> stFileNameBuf;
    std::vector<uint8_t> stStreamBuf;
    std::vector<ZSDBEntry> stEntryList;

    size_t nCount = 0;
    std::regex stFileNameReg(szFileNameRegex ? szFileNameRegex : ".*");

    for(auto &p: std::filesystem::directory_iterator(szDataPath)){
        std::string szFileName = p.path().filename();
        if(!p.is_regular_file()){
            continue;
        }

        if(szFileNameRegex){
            if(!std::regex_match(szFileName.c_str(), stFileNameReg)){
                continue;
            }
        }

        auto stSrcBuf = readFileData(((std::string(szDataPath) + "/") + szFileName).c_str());
        if(stSrcBuf.empty()){
            continue;
        }

        auto stDstBuf = compressDataBuf(stSrcBuf.data(), stSrcBuf.size(), pCCtx, pCDict);
        if(stDstBuf.empty()){
            continue;
        }

        bool bCompressed = ((1.00 * stDstBuf.size() / stSrcBuf.size()) < fCompRatio);
        auto &rstCurrBuf = bCompressed ? stDstBuf : stSrcBuf;

        ZSDBEntry stEntry;
        std::memset(&stEntry, 0, sizeof(stEntry));

        stEntry.Offset = stStreamBuf.size();
        stEntry.Length = rstCurrBuf.size();
        stStreamBuf.insert(stStreamBuf.end(), rstCurrBuf.begin(), rstCurrBuf.end());

        stEntry.FileName = stFileNameBuf.size();
        stFileNameBuf.insert(stFileNameBuf.end(), szFileName.begin(), szFileName.end());
        stFileNameBuf.push_back('\0');

        if(bCompressed){
            stEntry.Attribute |= F_COMPRESSED;
        }

        stEntryList.push_back(stEntry);
        nCount++;
    }

    std::sort(stEntryList.begin(), stEntryList.end(), [&stFileNameBuf](const ZSDBEntry &lhs, const ZSDBEntry &rhs) -> bool
    {
        return std::strcmp(stFileNameBuf.data() + lhs.FileName, stFileNameBuf.data() + rhs.FileName) < 0;
    });

    stEntryList.push_back(GetErrorEntry());

    ZSDBHeader stHeader;
    std::memset(&stHeader, 0, sizeof(stHeader));

    stHeader.ZStdVersion = ZSTD_versionNumber();
    stHeader.EntryNum    = nCount;

    stHeader.DictOffset = sizeof(stHeader);
    stHeader.DictLength = stCDictBuf.size();

    auto stEntryCompBuf = compressDataBuf((uint8_t *)(stEntryList.data()), stEntryList.size() * sizeof(ZSDBEntry), pCCtx, nullptr);
    stHeader.EntryOffset = stHeader.DictOffset + stHeader.DictLength;
    stHeader.EntryLength = stEntryCompBuf.size();

    auto stFileNameCompBuf = compressDataBuf((uint8_t *)(stFileNameBuf.data()), stFileNameBuf.size(), pCCtx, nullptr);
    stHeader.FileNameOffset = stHeader.EntryOffset + stHeader.EntryLength;
    stHeader.FileNameLength = stFileNameBuf.size();

    stHeader.StreamOffset = stHeader.FileNameOffset + stHeader.FileNameLength;
    stHeader.StreamLength = stStreamBuf.size();

    auto fp = std::fopen(szSaveFullName, "wb");
    if(!fp){
        return false;
    }

    if(std::fwrite(&stHeader, sizeof(stHeader), 1, fp) != 1){
        return false;
    }

    if(std::fwrite(stEntryCompBuf.data(), stEntryCompBuf.size(), 1, fp) != 1){
        return false;
    }

    if(std::fwrite(stFileNameCompBuf.data(), stFileNameCompBuf.size(), 1, fp) != 1){
        return false;
    }

    if(std::fwrite(stStreamBuf.data(), stStreamBuf.size(), 1, fp) != 1){
        return false;
    }

    if(std::fclose(fp)){
        return false;
    }

    return true;
}
