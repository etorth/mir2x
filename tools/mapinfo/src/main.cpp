#include <cstdio>
#include "mir2map.hpp"

int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("Usage: mapinfo map1 map2 ... mapN\n\n");
    }

    Mir2Map stMap;

    for(int nMapCnt = 1; nMapCnt < argc; ++nMapCnt){
        stMap.Load(argv[nMapCnt]);
        stMap.OpenAllDoor();
        std::printf("%s\n", argv[nMapCnt]);
        std::printf("%s\n", stMap.MapInfo().c_str());
    }

    return 0;
}
