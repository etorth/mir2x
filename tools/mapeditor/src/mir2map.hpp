#pragma once
#include "wilimagepackage.hpp"
#include <cstdint>
#include <functional>
#include <vector>
#include <Fl/Fl_Shared_Image.H>

#pragma pack(push, 1)
// this is abandoned
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
    uint16_t    wLigntNEvent;
}CELLINFO;

typedef struct{
    // for door or dynamic tile
    // conceptually they are different
    // but they are the same in view of bit set
    //
    // for door or dynamic tile we only need 3 byte
    // here we have 4 byte so use 1 byte as descriptor
    //
    // use dwDesc == 0 to indicate currently it's a null block
    // why not use dwObject1/2 & 0XFF00FFFF == 0
    // because here not-null doesn't means no obj
    // also means no light, no door etc.
    //
    // but dwObject1/2 & 0XFF00FFFF can only indicate there is no obj
    uint32_t    dwDesc;
    uint32_t    dwObject1;
    uint32_t    dwObject2;
    uint32_t    dwLight;
}CELLDESC;

#pragma pack(pop)

class Mir2Map
{
    private:
        bool            m_Valid;
    private:
        uint32_t        m_dwAniSaveTime[8];
        uint8_t         m_bAniTileFrame[8][16];

    private:
        WilImagePackage *m_pxTileImage;

    public:
        Mir2Map();
        ~Mir2Map();

    public:
        bool LoadMap(const char *);
        void LoadMapImage(WilImagePackage *);

    public:

        void ExtractOneBaseTile(std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)>, int, int);
        void ExtractOneObjectTile(std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)>, int, int);

        void ExtractBaseTile(std::function<bool(uint32_t, uint32_t)>, std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)>);
        void ExtractObjectTile(std::function<bool(uint32_t, uint32_t)>, std::function<void(uint32_t *, uint32_t, uint32_t, int, int, int, int)>);
        void ExtractGroundInfo(std::function<void(uint32_t, int, int, int)>);

    public:
        void     SetAniTileFrame(int);
        uint32_t GetDoorImageIndex(int, int);

    public:
        // use this API set to extract needed information only
        int  Width();
        int  Height();
        bool Valid();

    public:
        bool Expand(int, int);
        bool CropSize(int, int, int, int);
    public:
        const TILEINFO &BaseTileInfo(int, int);
        const CELLINFO &CellInfo(int, int);
    public:
        void DrawBaseTile(int, int, int, int, std::function<void(uint32_t, uint32_t, int, int)>);
        void DrawObjectTile(int, int, int, int, std::function<bool(uint32_t, uint32_t, Fl_Shared_Image * &, int, int)>, std::function<void(uint32_t, uint32_t, Fl_Shared_Image *, int, int)>);

    public:
        void CompressBaseTileInfo(std::vector<bool> &, std::vector<uint32_t> &);
        void CompressBaseTileInfoPreOrder(int, int, int, std::vector<bool> &, std::vector<uint32_t> &);
        bool EmptyBaseTileBlock(int, int, int);

    public:
        void CompressCellTileInfo(std::vector<bool> &, std::vector<CELLDESC> &);
        void CompressCellTileInfoPreOrder(int, int, int, std::vector<bool> &, std::vector<CELLDESC> &);
        bool EmptyCellTileBlock(int, int, int);

    public:
        bool NewLoadMap(const char *);

    private:
        TILEINFO       *m_pstTileInfo;
        CELLINFO       *m_pstCellInfo;
        MAPFILEHEADER   m_stMapFileHeader;

    private:
        uint32_t    *m_BaseTileInfo;
        uint32_t    *m_GroundInfo;
        CELLDESC    *m_CellDesc;

    private:
        uint32_t    BitPickOne(uint32_t *, uint32_t);


    private:
        bool LoadGroundInfo(uint32_t *, uint32_t, uint32_t *, uint32_t);
        void SetOneGroundInfoGrid(int, int, int, uint32_t);
        void SetGroundInfoBlock(int, int, int, uint32_t);
        void ParseGroundInfoStream(int, int, int, uint32_t *, uint32_t &, uint32_t *, uint32_t &);

    private:
        bool LoadCellDesc(uint32_t *, uint32_t, CELLDESC *, uint32_t);
        void SetCellDescBlock(int, int, int, const CELLDESC &);
        void ParseCellDescStream(int, int, int, uint32_t *, uint32_t &, CELLDESC *, uint32_t &);

    private:
        void SetBaseTileBlock(int, int, int, uint32_t);
        bool LoadBaseTileInfo(uint32_t *, uint32_t, uint32_t *, uint32_t);
        void ParseBaseTileStream(int, int, int, uint32_t *, uint32_t &, uint32_t *, uint32_t &);

    public:
        void Optimize();
        void OptimizeBaseTile(int, int);
        void OptimizeCell(int, int);

};
