#include <cerrno>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <initializer_list>

#include "strf.hpp"
#include "zcompf.hpp"
#include "rawbuf.hpp"
#include "fileptr.hpp"
#include "fflerror.hpp"

Rawbuf::Rawbuf(std::initializer_list<uint8_t> ilist)
{
    zcompf::zstdDecode(m_data, ilist.begin(), ilist.size());
}

std::vector<uint8_t> Rawbuf::buildBuf(const char *fileName)
{
    std::vector<uint8_t> readBuf;
    {
        auto fptr = make_fileptr(fileName, "rb");
        auto fp   = fptr.get();

        std::fseek(fp, 0, SEEK_END);
        const auto readFileSize = std::ftell(fp);
        std::fseek(fp, 0, SEEK_SET);

        readBuf.resize(readFileSize);
        if(std::fread(readBuf.data(), readFileSize, 1, fp) != 1){
            throw fflerror("failed to read file %s: %s", fileName, std::strerror(errno));
        }
    }

    std::vector<uint8_t> compBuf;
    zcompf::zstdEncode(compBuf, readBuf.data(), readBuf.size());
    return compBuf;
}

void Rawbuf::buildBinFile(const char *inFileName, const char *outFileName)
{
    const auto compBuf = buildBuf(inFileName);
    auto fptr = make_fileptr(outFileName, "wb");
    auto fp   = fptr.get();

    if(std::fwrite(compBuf.data(), compBuf.size(), 1, fp) != 1){
        throw fflerror("failed to write compressed data to file %s: %s", outFileName, std::strerror(errno));
    }
}

void Rawbuf::buildHexFile(const char *inFileName, const char *outFileName, size_t byteCountPerLine)
{
    if(byteCountPerLine == 0){
        byteCountPerLine = 8;
    }

    const auto compBuf = buildBuf(inFileName);
    auto fptr = make_fileptr(outFileName, "w");
    auto fp   = fptr.get();

    for(size_t off = 0; off < compBuf.size(); ++off){
        const bool newLine   = (((off % byteCountPerLine) == 0) && (off != 0));
        const bool needSpace = (((off % byteCountPerLine) != (byteCountPerLine - 1)) && (off != (compBuf.size() - 1)));
        std::fprintf(fp, "%s0x%02x,%s", newLine ? "\n" : "", to_d(compBuf[off]), needSpace ? " " : "");
    }
}
