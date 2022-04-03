#include <cstdint>
#include <cstring>
#include "totype.hpp"
#include "fileptr.hpp"
#include "fflerror.hpp"

#pragma pack(push, 1)

struct BGMELIST
{
    uint8_t flag; // 0x5b or 0x00
    char mapName[9];
    char bgmName[14];
};

struct BGMELISTHEADER
{
    char title[40];
    char targetDir[10];

    uint32_t fieldCount;
    uint32_t  listCount;
};

#pragma pack(pop)

int main(int argc, char *argv[])
{
    if(argc != 2){
        throw fflerror("usage: soundlistdecoder <file>.wwl");
    }

    auto fptr = make_fileptr(argv[1], "rb");

    BGMELISTHEADER header;
    read_fileptr(fptr, &header, sizeof(header));

    std::printf("header::title      : %s\n", header.title);
    std::printf("header::targetDir  : %s\n", header.targetDir);
    std::printf("header::fieldCount : %d\n", to_d(header.fieldCount));
    std::printf("header:: listCount : %d\n", to_d(header.listCount));

    // filedCount : how many .wav files records it contains
    //  listCount : how many we need to iterate inside the file, file size = sizeof(BGMELIST) * listCount + sizeof(BGMELISTHEADER)

    for(uint32_t i = 0; i < header.listCount; ++i){
        BGMELIST node;
        read_fileptr(fptr, &node, sizeof(node));

        if(node.flag != 0){
            std::printf("%s %s\n", node.mapName, node.bgmName);
        }
    }
    return 0;
}
