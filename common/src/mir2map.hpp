/*
 * =====================================================================================
 *
 *       Filename: mir2map.hpp
 *        Created: 05/03/2016 15:00:35
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

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

#include "totype.hpp"
#include "imagedb.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "mir2xmapdata.hpp"
#include "wilimagepackage.hpp"

#pragma pack(push, 1)

struct MAPFILEHEADER
{
    char        version[20];
    uint16_t    attr;
    int16_t     width;
    int16_t     height;
    char        eventFileIndex;
    char        fogColor;
};

struct TILEINFO
{
    uint8_t     fileIndex;
    uint16_t    imageIndex;
};

struct CELLINFO
{
    uint8_t     flag;
    uint8_t     obj0Ani;
    uint8_t     obj1Ani;
    uint8_t     fileIndex1;   // not 0
    uint8_t     fileIndex0;   // not 1
    uint16_t    obj0;
    uint16_t    obj1;
    uint16_t    doorIndex;
    uint8_t     doorOffset;
    uint16_t    lightEvent;
};

#pragma pack(pop)

class Mir2Map final
{
    private:
        MAPFILEHEADER m_mapFileHeader;

    private:
        std::vector<TILEINFO> m_tileInfo;
        std::vector<CELLINFO> m_cellInfo;

    public:
        explicit Mir2Map(const char *mapFileName)
        {
            auto fptr = make_fileptr(mapFileName, "rb");
            read_fileptr(fptr, &m_mapFileHeader, sizeof(m_mapFileHeader));

            fflassert(m_mapFileHeader.width  > 0);
            fflassert(m_mapFileHeader.height > 0);

            fflassert(m_mapFileHeader.width  % 2 == 0);
            fflassert(m_mapFileHeader.height % 2 == 0);

            const size_t cellCount = m_mapFileHeader.width * m_mapFileHeader.height;
            read_fileptr(fptr, m_tileInfo, cellCount / 4);
            read_fileptr(fptr, m_cellInfo, cellCount);
        }

    public:
        bool validC(int x, int y) const
        {
            return true
                && x >= 0 && x < to_d(w())
                && y >= 0 && y < to_d(h());
        }

        bool ValidP(int x, int y) const
        {
            return true
                && x >= 0 && x < to_d(SYS_MAPGRIDXP * w())
                && y >= 0 && y < to_d(SYS_MAPGRIDYP * h());
        }

    public:
        size_t w() const
        {
            return m_mapFileHeader.width;
        }

        size_t h() const
        {
            return m_mapFileHeader.height;
        }

    public:
        uint16_t lightEvent(int x, int y) const
        {
            return cellInfo(x, y).lightEvent;
        }

        bool lightValid(int x, int y) const
        {
            return (cellInfo(x, y).lightEvent != 0) || ((cellInfo(x, y).lightEvent & 0X0007) == 1);
        }

        // in mir2 code only checked if light valid
        // the light size and color information is not used

        uint8_t lightSize(int x, int y) const
        {
            return (lightEvent(x, y) & 0XC000) >> 14;
        }

        uint16_t lightColor(int x, int y) const
        {
            return (lightEvent(x, y) & 0X3FF0) >> 4;
        }

    public:
        uint8_t groundFlag(int x, int y) const
        {
            return cellInfo(x, y).flag;
        }

        bool groundValid(int x, int y) const
        {
            return groundFlag(x, y) & 0X01;
        }

        bool aniObjectValid(int x, int y, int index, ImageDB &imgDB) const
        {
            if(objectValid(x, y, index, imgDB)){
                if(index == 0){
                    return cellInfo(x, y).obj0Ani != 255;
                }
                else{
                    return cellInfo(x, y).obj1Ani != 255;
                }
            }
            return false;
        }

        bool groundObjectValid(int x, int y, int index, ImageDB &imgDB) const
        {
            const auto [fileIndex, imageIndex] = [x, y, index, this]() -> std::tuple<uint8_t, uint16_t>
            {
                if(index == 0){
                    return {cellInfo(x, y).fileIndex0, cellInfo(x, y).obj0};
                }else{
                    return {cellInfo(x, y).fileIndex1, cellInfo(x, y).obj1};
                }
            }();

            return true
                && (fileIndex % 15) >  2 // 0 1 2 are for big tiles
                && (fileIndex % 15) < 14 // 14 is xxx/Custom

                &&  fileIndex !=   255
                && imageIndex != 65535
                && imgDB.setIndex(fileIndex, imageIndex)

                && imgDB.currImageInfo()->width  == SYS_MAPGRIDXP
                && imgDB.currImageInfo()->height == SYS_MAPGRIDYP;
        }

        bool objectValid(int x, int y, int index, ImageDB &imgDB) const
        {
            const auto [fileIndex, imageIndex] = [x, y, index, this]() -> std::tuple<uint8_t, uint16_t>
            {
                if(index == 0){
                    return {cellInfo(x, y).fileIndex0, cellInfo(x, y).obj0};
                }else{
                    return {cellInfo(x, y).fileIndex1, cellInfo(x, y).obj1};
                }
            }();

            return true
                && (fileIndex % 15) >  2 // 0 1 2 are for big tiles
                && (fileIndex % 15) < 14 // 14 is xxx/Custom

                &&  fileIndex !=   255
                && imageIndex != 65535
                && imgDB.setIndex(fileIndex, imageIndex);
        }

        uint32_t object(int x, int y, int index) const
        {
            const auto [fileIndex, imageIndex, aniByte] = [x, y, index, this]() -> std::tuple<uint8_t, uint16_t, uint8_t>
            {
                if(index == 0){
                    return {cellInfo(x, y).fileIndex0, cellInfo(x, y).obj0, cellInfo(x, y).obj0Ani};
                }else{
                    return {cellInfo(x, y).fileIndex1, cellInfo(x, y).obj1, cellInfo(x, y).obj1Ani};
                }
            }();

            // doesn't mask off highest bit of aniByte
            // it's for animation existance
            return (to_u32(aniByte) << 24) + (to_u32(fileIndex) << 16) + to_u32(imageIndex);
        }

    public:
        bool doorValid(int x, int y) const
        {
            return cellInfo(x, y).doorIndex & 0X80;
        }

        uint8_t doorIndex(int x, int y) const
        {
            return cellInfo(x, y).doorIndex & 0X7F;
        }

        uint32_t getDoorImageIndex(int x, int y) const // extra index offset to draw open/close
        {
            // seems doorOffset & 0X80 shows open or close
            //       doorIndex  & 0X80 shows whether there is a door

            // door is closed by default
            // if open bit is set, need to add extra index offset: doorOffset & 0X7F

            if((cellInfo(x, y).doorOffset & 0X80)){
                if(doorValid(x, y) && doorIndex(x, y) > 0){ // can't understand why check doorIndex() > 0
                    return cellInfo(x, y).doorOffset & 0X7F;
                }
            }
            return 0;
        }

    public:
        void openDoor(int x, int y, uint8_t index)
        {
            for(int iy = y - 8; iy < y + 10; iy++){
                for(int ix = x - 8; ix < x + 10; ix++){
                    if(validC(ix, iy) && doorValid(ix, iy) && doorIndex(ix, iy) == index){
                        cellInfo(ix, iy).doorOffset |= 0X80;
                    }
                }
            }
        }

        void closeDoor(int x, int y, uint8_t index)
        {
            for(int iy = y - 8; iy < y + 10; iy++){
                for(int ix = x - 8; ix < x + 10; ix++){
                    if(validC(ix, iy) && doorValid(ix, iy) && doorIndex(ix, iy) == index){
                        cellInfo(ix, iy).doorOffset &= 0X7F;
                    }
                }
            }
        }

        void openAllDoor()
        {
            for(int x = 0; x < to_d(w()); ++x){
                for(int y = 0; y < to_d(h()); ++y){
                    if(doorValid(x, y)){
                        openDoor(x, y, doorIndex(x, y));
                    }
                }
            }
        }

        void closeAllDoor()
        {
            for(int x = 0; x < to_d(w()); ++x){
                for(int y = 0; y < to_d(h()); ++y){
                    if(doorValid(x, y)){
                        closeDoor(x, y, doorIndex(x, y));
                    }
                }
            }
        }

    public:
        bool tileValid(int x, int y, ImageDB &imgDB) const
        {
            return imgDB.setIndex(tileInfo(x, y).fileIndex, tileInfo(x, y).imageIndex);
        }

        uint32_t tile(int x, int y) const
        {
            return (to_u32(tileInfo(x, y).fileIndex) << 16) + tileInfo(x, y).imageIndex;
        }

    private:
        const TILEINFO &tileInfo(int x, int y) const
        {
            return m_tileInfo[(x / 2) * h() / 2 + y / 2];
        }

        const CELLINFO &cellInfo(int x, int y) const
        {
            return m_cellInfo[x * h() + y];
        }

        TILEINFO &tileInfo(int x, int y)
        {
            return m_tileInfo[(x / 2) * h() / 2 + y / 2];
        }

        CELLINFO &cellInfo(int x, int y)
        {
            return m_cellInfo[x * h() + y];
        }

    public:
        void convBlock(int x, int y, Mir2xMapData::BLOCK &block, ImageDB &imgDB) const
        {
            fflassert((x % 2) == 0);
            fflassert((y % 2) == 0);

            block = {};
            if(tileValid(x, y, imgDB)){
                block.tile.texIDValid = 1;
                block.tile.texID = tile(x, y) & 0X00FFFFFF;
            }

            for(const int dy: {0, 1}){
                for(const int dx: {0, 1}){
                    const int ix = x + dx;
                    const int iy = y + dy;
                    const int ic = 2 * dy + dx;

                    if(groundValid(ix, iy)){
                        block.cell[ic].canWalk  = 1;
                        block.cell[ic].canFly   = 1;
                        block.cell[ic].landType = 0;
                    }

                    if(lightValid(ix, iy)){
                        block.cell[ic].light.valid  = 1;
                        block.cell[ic].light.radius = lightSize(ix, iy)  & 0X03;
                        block.cell[ic].light.alpha  = 0;
                        block.cell[ic].light.color  = lightColor(ix, iy) & 0X03;
                    }

                    for(const int io: {0, 1}){
                        if(objectValid(ix, iy, io, imgDB)){
                            block.cell[ic].obj[io].texIDValid = 1;
                            block.cell[ic].obj[io].texID = object(ix, iy, io) & 0X00FFFFFF;
                            block.cell[ic].obj[io].depthType = groundObjectValid(ix, iy, io, imgDB) ? OBJD_GROUND : OBJD_OVERGROUND0;

                            if(aniObjectValid(ix, iy, io, imgDB)){
                                block.cell[ic].obj[io].animated   = 1;
                                block.cell[ic].obj[io].alpha      = to_u8((object(ix, iy, io) & 0X80000000) >> (7 + 8 + 16));
                                block.cell[ic].obj[io].tickType   = to_u8((object(ix, iy, io) & 0X70000000) >> (4 + 8 + 16));
                                block.cell[ic].obj[io].frameCount = to_u8((object(ix, iy, io) & 0X0F000000) >> (0 + 8 + 16));
                            }
                        }
                    }

                    // optional
                    // sort the objects in same cell
                    std::sort(block.cell[ic].obj, block.cell[ic].obj + 2, [](const auto &obj1, const auto &obj2) -> bool
                    {
                        if(obj1.texIDValid && obj2.texIDValid){
                            return obj1.depthType < obj2.depthType;
                        }
                        return obj1.texIDValid;
                    });
                }
            }
        }

    public:
        std::string dumpMapInfo() const
        {
            size_t lightCount = 0;
            size_t  doorCount = 0;
            size_t alphaCount = 0;
            size_t   aniCount = 0;

            // tile doesn't contain any information for alpha-blend, light or door
            for(int x = 0; x < to_d(w()); ++x){
                for(int y = 0; y < to_d(h()); ++y){

                    // light info
                    // seems the 15~4 for specified light-frog
                    //            3~0 for   general light-frog
                    if(lightValid(x, y)){
                        lightCount++;
                    }

                    // door info
                    // for one cell there are two object fields but only one door info field

                    // doorIndex & 0X80 for if there is a door
                    // doorIndex & 0X7F for door index, if non-zero
                    // doorOffset & 0X80 for open/close the door
                    // doorOffset & 0X7F for door image offset
                    if(doorValid(x, y)){
                        doorCount++;
                    }

                    // alpha/ani info, 1st layer:
                    // doesn't use the imageDB to check object existance
                    if(cellInfo(x, y).fileIndex0 != 255 && cellInfo(x, y).obj0 != 65535){
                        if(cellInfo(x, y).obj0Ani != 255){
                            aniCount++;
                            if((cellInfo(x, y).obj0Ani & 0X80)){
                                alphaCount++;
                            }
                        }
                    }

                    // alpha/ani info, 2nd layer:
                    // doesn't use the imageDB to check object existance
                    if(cellInfo(x, y).fileIndex1 != 255 && cellInfo(x, y).obj1 != 65535){
                        if(cellInfo(x, y).obj1Ani != 255){
                            aniCount++;
                            if((cellInfo(x, y).obj1Ani & 0X80)){
                                alphaCount++;
                            }
                        }
                    }
                }
            }

            return str_printf(
                    "width : %zu\n"
                    "height: %zu\n"
                    "light : %zu\n"
                    "door  : %zu\n"
                    "alpha : %zu\n"
                    "ani   : %zu\n", w(), h(), lightCount, doorCount, alphaCount, aniCount);
        }
};
