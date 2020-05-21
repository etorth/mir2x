/*
 * =====================================================================================
 *
 *       Filename: rawbuf.cpp
 *        Created: 12/23/2018 03:48:03
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

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <cinttypes>
#include <initializer_list>

#include "zstd.h"
#include "rawbuf.hpp"
#include "strf.hpp"
#include "fileptr.hpp"
#include "fflerror.hpp"

Rawbuf::Rawbuf(std::initializer_list<uint8_t> stInitList)
    : m_data()
{
    switch(auto nDecompSize = ZSTD_getFrameContentSize(stInitList.begin(), stInitList.size())){
        case ZSTD_CONTENTSIZE_ERROR:
        case ZSTD_CONTENTSIZE_UNKNOWN:
            {
                throw fflerror("not a zstd compressed data buffer");
            }
        default:
            {
                m_data.resize(nDecompSize);
                break;
            }
    }

    size_t nRC = ZSTD_decompress(m_data.data(), m_data.size(), stInitList.begin(), stInitList.size());

    if(ZSTD_isError(nRC)){
        throw fflerror("failed to decompress data buffer");
    }

    m_data.resize(nRC);
}

std::vector<uint8_t> Rawbuf::BuildBuf(const char *szInFileName)
{
    std::vector<uint8_t> stReadBuf;
    {
        auto fp_in = make_fileptr(szInFileName, "rb");

        std::fseek(fp_in.get(), 0, SEEK_END);
        auto nReadFileLen = std::ftell(fp_in.get());
        std::fseek(fp_in.get(), 0, SEEK_SET);

        stReadBuf.resize(nReadFileLen, 0);
        if(std::fread(stReadBuf.data(), nReadFileLen, 1, fp_in.get()) != 1){
            throw fflerror("failed to read file: %s", szInFileName);
        }
    }

    std::vector<uint8_t> stCompBuf(ZSTD_compressBound(stReadBuf.size()), 0);
    size_t nRC = ZSTD_compress(stCompBuf.data(), stCompBuf.size(), stReadBuf.data(), stReadBuf.size(), ZSTD_maxCLevel());

    if(ZSTD_isError(nRC)){
        throw fflerror("failed to compress file: %s", szInFileName);
    }

    stCompBuf.resize(nRC);
    return stCompBuf;
}

void Rawbuf::BuildBinFile(const char *szInFileName, const char *szOutFileName)
{
    auto stCompBuf = BuildBuf(szInFileName);
    auto fp_out = make_fileptr(szOutFileName, "wb");

    if(std::fwrite(stCompBuf.data(), stCompBuf.size(), 1, fp_out.get()) != 1){
        throw fflerror("failed to write compressed data to file: %s, reason: %s", szOutFileName, std::strerror(errno));
    }
}

void Rawbuf::BuildHexFile(const char *szInFileName, const char *szOutFileName, size_t nBytePerLine)
{
    if(nBytePerLine == 0){
        nBytePerLine = 8;
    }

    auto stCompBuf = BuildBuf(szInFileName);
    auto fp_out = make_fileptr(szOutFileName, "w");

    for(size_t nBufOff = 0; nBufOff < stCompBuf.size(); ++nBufOff){
        bool bNewLine = (((nBufOff % nBytePerLine) == 0) && (nBufOff != 0));
        bool bNeedSpace = (((nBufOff % nBytePerLine) != (nBytePerLine - 1)) && (nBufOff != (stCompBuf.size() - 1)));
        std::fprintf(fp_out.get(), "%s0x%02" PRIx8 ",%s", bNewLine ? "\n" : "", stCompBuf[nBufOff], bNeedSpace ? " " : "");
    }
}
