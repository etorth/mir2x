/*
 * =====================================================================================
 *
 *       Filename: zsdb.hpp
 *        Created: 11/13/2018 22:31:02
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

#pragma once
#include <vector>
#include <cstdint>
#include "zstd.h"

class ZSDB final
{
    public:
        enum
        {
            F_COMPRESSED = 1,
        };

    private:
#pragma pack(push, 1)
        struct ZSDBHeader
        {
            uint64_t ZStdVersion;
            uint64_t EntryNum;

            uint64_t DictOffset;
            uint64_t DictLength;

            uint64_t EntryOffset;
            uint64_t EntryLength;

            uint64_t FileNameOffset;
            uint64_t FileNameLength;

            uint64_t StreamOffset;
            uint64_t StreamLength;
        };

        struct ZSDBEntry
        {
            uint64_t Offset;
            uint64_t Length;
            uint64_t FileName;
            uint64_t Attribute;
        };
#pragma pack(pop)

    private:
        std::FILE *m_fp;

    private:
        ZSTD_DCtx  *m_DCtx;
        ZSTD_DDict *m_DDict;

    private:
        ZSDBHeader m_Header;

    private:
        std::vector<ZSDBEntry> m_EntryList;

    private:
        std::vector<char> m_FileNameBuf;

    public:
        ZSDB(const char *);

    public:
        ~ZSDB();

    public:
        bool Decomp(const char *, size_t, std::vector<uint8_t> *);

    private:
        bool DecompEntry(const ZSDBEntry &, std::vector<uint8_t> *);

    private:
        static const ZSDBEntry &GetErrorEntry();

    public:
        static bool BuildDB(const char *, const char *, const char *, const char *, double);
};
