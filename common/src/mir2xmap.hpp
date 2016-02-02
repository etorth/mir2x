#pragma once
#include <cstdint>
#include <functional>

typedef struct{
    uint8_t     Desc;
    uint8_t     FileIndex;
    uint16_t    ImageIndex;
}TILEDESC;

typedef struct{
    uint8_t     Desc;           // animation or not
    uint8_t     FileIndex;  
    uint16_t    ImageIndex;
}OBJDESC;

typedef struct{
    uint8_t     Desc;
    OBJDESC     Obj[2];
    uint16_t    Light;
}CELLDESC;

// #pragma pack(push, 1)
// typedef struct{
//     uint8_t  Desc;
//     uint8_t  Light;
//     uint32_t Obj1;
//     uint32_t Obj2;
// }CELLDESC;
//
// typedef struct{
//     uint8_t  FileIndex;
//     uint16_t ImageIndex;
// }TILEDESC;
// #pragma pack(pop)

class Mir2xMap
{
    public:
        Mir2xMap();
        ~Mir2xMap();

    public:
        bool Valid();
        int  W();
        int  H();
        int  ViewX();
        int  ViewY();

    public:
        bool NewLoadMap(const char *);

    private:
        uint32_t   *m_BaseTileInfo;
        uint32_t   *m_GroundInfo;
        CELLDESC   *m_CellDesc;

    private:
        bool        m_Valid;
        uint16_t    m_W;
        uint16_t    m_H;
        int         m_ViewX;
        int         m_ViewY;

    private:
        uint32_t    m_dwAniSaveTime[8];
        uint8_t     m_bAniTileFrame[8][16];

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
        void DrawBaseTile(int, int, int, int);
        void DrawGroundObject(int, int, int, int);
        void DrawOverGroundObject(int, int, int, int, std::function<void(int, int)>);

    private:
        template<typename T> bool PickOneBit(const T *pData, long nOffset)
        {
            long nShift = sizeof(T) - (nOffset % sizeof(T));
            return (((T)(pData[nOffset / sizeof(T)] & ((T)1) << nShift)) >> nShift) != 0;
        }

    public:
        bool Overlap(int);

    public:
        void SetViewPoint(int nX, int nY);

    private:
        uint32_t GetDoorImageIndex(int, int);


    private:
        TILEDESC &Mir2xMap::TileDesc(int nX, int nY)
        {
            return m_TileDesc[nX / 2 + nY / 2 * m_H / 2];
        }

        CELLDESC &Mir2xMap::CellDesc(int nX, int nY)
        {
            return m_TileDesc[nX + nY * m_H ];
        }

    private:
        void DrawObject(int, int, int, int,
                std::function<bool(int, int)>,
                std::function<void(int, int)>);
    private:
        uint32_t BaseTileInfo(int, int);
};
