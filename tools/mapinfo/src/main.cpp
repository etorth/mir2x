#include <cstdio>
#include "mir2map.hpp"
#include "fflerror.hpp"

int main(int argc, char *argv[])
{
    if(argc < 2){
        throw fflerror("Usage: mapinfo map1 map2 ... mapN");
    }

    for(int i = 1; i < argc; ++i){
        const auto p = std::make_unique<Mir2Map>(argv[i]);
        p->openAllDoor();

        std::printf("%s\n", argv[i]);
        std::printf("%s\n", p->dumpMapInfo().c_str());
    }
    return 0;
}
