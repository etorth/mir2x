#pragma once
#include <cstdint>
#include <functional>
#include "triangle.hpp"
#include "directiverectcover.hpp"

typedef struct{
    uint8_t     Desc;
    uint8_t     FileIndex;
    uint16_t    ImageIndex;
}TILEDESC;

typedef struct{
    uint8_t     Desc;
    uint8_t     Ani;
    uint8_t     FileIndex;
    uint16_t    ImageIndex;
}OBJDESC;

typedef struct{
    uint8_t     Desc;
    OBJDESC     Obj[2];
    uint16_t    Light;
}CELLDESC;

#pragma pack(push, 1)
typedef struct{
    uint8_t  Desc;
    uint8_t  Light;
    uint32_t Obj1;
    uint32_t Obj2;
}CELLDESC;

typedef struct{
    uint8_t  FileIndex;
    uint16_t ImageIndex;
}TILEDESC;
#pragma pack(pop)

class Monster;
class ClientMap
{
    public:
        ClientMap();
        ~ClientMap();

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
        bool ValidPosition(int, int, Monster *);

    public:
        void DrawBaseTile(int, int, int, int);
        void DrawGroundObject(int, int, int, int);
        void DrawOverGroundObject(int, int, int, int, std::function<void(int, int)>);

    private:
        template<typename T> bool PickOneBit(T *pData, long nOffset)
        {
            long nShift = sizeof(T) - (nOffset % sizeof(T));
            return (((T)(pData[nOffset / sizeof(T)] & ((T)1) << nShift)) >> nShift) != 0;
        }

        uint32_t ClientMap::BitPickOne(uint32_t *pU32BitStream, uint32_t nOffset)
        {
            // nOffset can only be even number
            uint32_t nShift = 31 - (nOffset % 32);
        }

    public:
        bool Overlap(const DirectiveRectCover &);
        bool Overlap(const Triangle &);

    public:
        void SetViewPoint(int nX, int nY);

    private:
        uint32_t GetDoorImageIndex(int, int);

    private:
        void DrawObject(int, int, int, int,
                std::function<bool(int, int)>,
                std::function<void(int, int)>);
    private:
        uint32_t BaseTileInfo(int, int);
};
