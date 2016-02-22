#pragma once

#include <string>
#include "wilimagepackage.hpp"
#include <cstdint>
#include <functional>
#include <vector>

#include "imagedb.hpp"

#pragma pack(push, 1)

typedef struct{
    int32_t     bIsLight;
    char        cLightSizeType;
    char        cLightColorType;
}LIGHTINFO;

typedef struct{
    char        szDesc[20];
    uint16_t    wAttr;
    int16_t     shWidth;
    int16_t     shHeight;
    char        cEventFileIndex;
    char        cFogColor;
}MAPFILEHEADER;

typedef struct{
    uint8_t     bFileIndex;
    uint16_t    wTileIndex;
}TILEINFO;

typedef struct{
    uint8_t     bFlag;
    uint8_t     bObj1Ani;
    uint8_t     bObj2Ani;
    uint16_t    wFileIndex;
    uint16_t    wObj1;
    uint16_t    wObj2;
    uint16_t    bDoorIndex;
    uint8_t     bDoorOffset;
    uint16_t    wLightNEvent;
}CELLINFO;

#pragma pack(pop)

class Mir2Map
{
    public:
        Mir2Map();
        ~Mir2Map();

    private:
        bool            m_Valid;

    private:
        TILEINFO       *m_pstTileInfo;
        CELLINFO       *m_pstCellInfo;
        MAPFILEHEADER   m_stMapFileHeader;

    public:
        bool Load(const char *);

    public:
        uint32_t GetDoorImageIndex(int, int);

    public:
        // use this API set to extract needed information only
        bool Valid()
        {
            return m_Valid;
        }

        bool ValidC(int nX, int nY)
        {
            return true
                && nX >= 0
                && nX <  m_stMapFileHeader.shWidth
                && nY >= 0
                && nY <  m_stMapFileHeader.shHeight;
        }

        bool ValidP(int nX, int nY)
        {
            return true
                && nX >= 0
                && nX <  48 * m_stMapFileHeader.shWidth
                && nY >= 0
                && nY <  32 * m_stMapFileHeader.shHeight;
        }

    public:
        int  W()
        {
            return m_stMapFileHeader.shWidth;
        }

        int  H()
        {
            return m_stMapFileHeader.shHeight;
        }

    public:
        bool LightValid(int, int);
        uint16_t Light(int, int);

        bool GroundValid(int, int);
        uint8_t Ground(int, int);

        bool ObjectValid(int, int, int, ImageDB &);
        bool GroundObjectValid(int, int, int, ImageDB &);
        bool AniObjectValid(int, int, int, ImageDB &);
        uint32_t Object(int, int, int);

        bool TileValid(int, int, ImageDB &);
        uint32_t Tile(int, int);

    private:
        const TILEINFO &BaseTileInfo(int nX, int nY)
        {
            return m_pstTileInfo[(nX / 2) * m_stMapFileHeader.shHeight / 2 + nY / 2];
        }

        const CELLINFO &CellInfo(int nX, int nY)
        {
            return m_pstCellInfo[nY + nX * m_stMapFileHeader.shHeight];
        }

    public:
        uint8_t GetDoor(int, int);
        void    OpenDoor(int, int, uint8_t);
        void    CloseDoor(int, int, uint8_t);
        void    OpenAllDoor();
        void    CloseAllDoor();

    public:
        std::string MapInfo();
};
