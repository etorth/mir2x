#include <cstdio>
#include <cstring>
#include "dbcomid.hpp"

int main(int argc, char *argv[])
{
    if(argc == 2){
        if(auto nMapID = DBCOM_MAPID((char8_t *)(argv[1]))){
            std::printf("%08X", nMapID);
            return 0;
        }
    }
    return 1;
}
