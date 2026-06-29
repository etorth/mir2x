#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include "zstd.h"
#include "fileptr.hpp"

class ZSDB final
{
    public:
        struct Entry
        {
            const char *fileName  = nullptr;
            size_t      length    = 0;
            uint64_t    attribute = 0;
        };

    public:
        enum : uint64_t
        {
            F_COMPRESSED = 1,
        };

    private:
        static constexpr char     FORMAT_MAGIC[4] = {'Z', 'S', 'D', 'B'};
        static constexpr uint32_t FORMAT_VERSION  = 1;

    private:
#pragma pack(push, 1)
        struct ZSDBHeader
        {
            char     magic[4];
            uint32_t formatVersion;
            uint64_t entryNum;

            uint64_t dictOffset;
            uint64_t dictLength;

            uint64_t entryOffset;
            uint64_t entryLength;

            uint64_t fileNameOffset;
            uint64_t fileNameLength;

            uint64_t streamOffset;
            uint64_t streamLength;
        };

        struct InnEntry
        {
            uint64_t offset;
            uint64_t length;
            uint64_t fileName;
            uint64_t attribute;
        };
#pragma pack(pop)

    private:
        fileptr_t m_fp;

    private:
        std::unique_ptr<ZSTD_DCtx,  size_t (*)(ZSTD_DCtx  *)> m_DCtx  {nullptr, ZSTD_freeDCtx };
        std::unique_ptr<ZSTD_DDict, size_t (*)(ZSTD_DDict *)> m_DDict {nullptr, ZSTD_freeDDict};

    private:
        ZSDBHeader m_header;

    private:
        std::vector<InnEntry> m_entryList;

    private:
        std::vector<char> m_fileNameBuf;

    public:
        ZSDB(const char *);

    public:
        const char *decomp(const char *, size_t, std::vector<uint8_t> *);

    private:
        bool decompEntry(const InnEntry &, std::vector<uint8_t> *);

    private:
        static InnEntry getErrorEntry();

    public:
        std::vector<ZSDB::Entry> getEntryList() const;

    public:
        static void buildDB(const char *, const char *, const char *, const char *, double);
};
