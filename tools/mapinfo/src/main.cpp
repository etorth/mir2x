#include <string>
#include <cstring>
#include "wilimagepackage.hpp"
#include "mir2map.hpp"
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <array>
#include <vector>
#include <utility>

#include "filesys.hpp"

WilImagePackage     g_WilImagePackage[128];
std::string         g_WilFilePathName;
Mir2Map             g_Map;

bool LoadWilPackage()
{
    const char *szFileName[] = {
        "Tilesc",
        "Tiles30c",
        "Tiles5c",
        "Smtilesc",
        "Housesc",
        "Cliffsc",
        "Dungeonsc",
        "Innersc",
        "Furnituresc",
        "Wallsc",
        "SmObjectsc",
        "Animationsc",
        "Object1c",
        "Object2c",
        "Custom",
        "Wood/Tilesc",
        "Wood/Tiles30c",
        "Wood/Tiles5c",
        "Wood/Smtilesc",
        "Wood/Housesc",
        "Wood/Cliffsc",
        "Wood/Dungeonsc",
        "Wood/Innersc",
        "Wood/Furnituresc",
        "Wood/Wallsc",
        "Wood/SmObjectsc",
        "Wood/Animationsc",
        "Wood/Object1c",
        "Wood/Object2c",
        "Wood/Custom",
        "Sand/Tilesc",
        "Sand/Tiles30c",
        "Sand/Tiles5c",
        "Sand/Smtilesc",
        "Sand/Housesc",
        "Sand/Cliffsc",
        "Sand/Dungeonsc",
        "Sand/Innersc",
        "Sand/Furnituresc",
        "Sand/Wallsc",
        "Sand/SmObjectsc",
        "Sand/Animationsc",
        "Sand/Object1c",
        "Sand/Object2c",
        "Sand/Custom",
        "Snow/Tilesc",
        "Snow/Tiles30c",
        "Snow/Tiles5c",
        "Snow/Smtilesc",
        "Snow/Housesc",
        "Snow/Cliffsc",
        "Snow/Dungeonsc",
        "Snow/Innersc",
        "Snow/Furnituresc",
        "Snow/Wallsc",
        "Snow/SmObjectsc",
        "Snow/Animationsc",
        "Snow/Object1c",
        "Snow/Object2c",
        "Snow/Custom",
        "Forest/Tilesc",
        "Forest/Tiles30c",
        "Forest/Tiles5c",
        "Forest/Smtilesc",
        "Forest/Housesc",
        "Forest/Cliffsc",
        "Forest/Dungeonsc",
        "Forest/Innersc",
        "Forest/Furnituresc",
        "Forest/Wallsc",
        "Forest/SmObjectsc",
        "Forest/Animationsc",
        "Forest/Object1c",
        "Forest/Object2c",
        "Forest/Custom",
        ""
    };

    extern std::string		g_WilFilePathName;
    extern WilImagePackage	g_WilImagePackage[128];
    for(int i = 0; std::strlen(szFileName[i]) > 0; ++i){
        if(!FileExist((g_WilFilePathName + szFileName[i]).c_str())){
            continue;
        }
        if(!g_WilImagePackage[i].Load(g_WilFilePathName.c_str(), szFileName[i], ".wil")){
            return false;
        }
    }
    return true;
}



int main(int argc, char *argv[])
{
    if(argc < 3){
        printf(
                "Usage: mapinfo wil_package map1 map2 ... mapN\n"
                "\n"
                "wil_package: wil-package folder path\n"
                "mapN       : map list\n"
              );

    }

    g_WilFilePathName = argv[1];

    if(!LoadWilPackage()){
        std::printf("Invalid wil-package folder path");
        return 0;
    }

    g_Map.LoadMapImage(g_WilImagePackage);

    for(int nMapCnt = 2; nMapCnt < argc; ++nMapCnt){
        g_Map.LoadMap(argv[nMapCnt]);
        g_Map.OpenAllDoor();
        std::printf("%s\n", argv[nMapCnt]);
        std::printf("%s\n", g_Map.MapInfo().c_str());
    }

    return 0;
}
