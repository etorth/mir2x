#pragma once
#include <tuple>
#include <string>
#include <vector>

namespace filesys
{
    bool    hasDir(const char *);
    bool   makeDir(const char *);
    bool removeDir(const char *);

    bool  hasFile(const char *);
    void copyFile(const char *, const char *);

    std::string readFile(const char *);

    std::vector<std::string> getFileList  (const char *, bool /* fullPath */, const char * /* reg */ = nullptr);
    std::vector<std::string> getSubDirList(const char *, bool /* fullPath */, const char * /* reg */ = nullptr);

    std::tuple<std::string, std::string, std::string> decompFileName(const char *, bool);
}
