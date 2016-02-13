#pragma once
#include <cstdint>
#include <functional>

class Mir2xMap
{
    private:
        typedef struct
        {
            uint8_t     Desc;
            uint8_t     FileIndex;
            uint16_t    ImageIndex;
        }TILEDESC;

        typedef struct
        {
            uint8_t     Desc;
            uint8_t     FileIndex;  
            uint16_t    ImageIndex;
        }OBJDESC;

        typedef struct
        {
            uint16_t    Desc;
            OBJDESC     Obj[2];
            uint16_t    Light;
        }CELLDESC;

    public:
        Mir2xMap();
        ~Mir2xMap();

    public:
        int W()
        {
            return m_W;
        }
        int H()
        {
            return m_H;
        }

    public:
        bool Load(const char *);



    private:
        uint32_t    m_dwAniSaveTime[8];
        uint8_t     m_bAniTileFrame[8][16];

    private:
        bool LoadObj1(uint8_t * &);
        bool LoadObj2(uint8_t * &);

    public:
        void Draw(int, int, int, int, int,
                std::function<void(int, int, uint32_t)>,
                std::function<void(int, int, uint32_t)>,
                std::function<void(int, int)>,
                std::function<void(int, int)>);

    private:
        void DrawGround(int, int, int, int, std::function<void(int, int, uint32_t)>);
        void DrawGroundObj(int, int, int, int, int, std::function<void(int, int, uint32_t)>);
        void DrawOverGroundObj(int, int, int, int, int, std::function<void(int, int, uint32_t)>, std::function<void(int, int)>);
        void DrawExt(int, int, int, int, std::function<void(int, int)>);

    private:
        uint16_t CellObjImageOff(int, int, int)
        {
            return 0;
        }

        uint16_t TileImageOff(int, int)
        {
            return 0;
        }

    private:
        uint32_t CellObjKey(int nX, int nY, int nIndex)
        {
            return (((uint32_t)(CellDesc(nX, nY).Obj[nIndex % 2].Desc & 0X80)) << 24)
                + (((uint32_t)(CellDesc(nX, nY).Obj[nIndex % 2].FileIndex)) << 16)
                + (((uint32_t)(CellDesc(nX, nY).Obj[nIndex % 2].ImageIndex + CellObjImageOff(nX, nY, nIndex))));
        }

        uint32_t TileKey(int nX, int nY)
        {
            return ((uint32_t)(TileDesc(nX, nY).FileIndex) << 16) + ((uint32_t)(TileDesc(nX, nY).ImageIndex));
        }

    private:
        bool TileValid(int nX, int nY)
        {
            return (TileDesc(nX, nY).Desc & 0X01) == 0X01;
        }

        bool GroundObjValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & (0X0300 << ((nIndex % 2) * 2))) == (0X0200 << ((nIndex % 2) * 2));
        }

        bool OverGroundObjValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & (0X0300 << ((nIndex % 2) * 2))) == (0X0300 << ((nIndex % 2) * 2));
        }

    private:
        bool PickOneBit(const uint8_t *pData, long nOffset)
        {
            return (pData[nOffset / 8] & (0X01 << (nOffset) % 8)) != 0;
        }

    public:
        bool Overlap(int, int, int, int, int);

    private:
        bool CanWalk(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & ((0X01) << (nIndex % 4))) != 0;
        }

    public:
        void SetViewPoint(int nX, int nY);

    private:
        uint32_t GetDoorImageIndex(int, int);


    private:
        TILEDESC &TileDesc(int nX, int nY)
        {
            return m_TileDesc[nX / 2 + nY / 2 * m_W / 2];
        }

        CELLDESC &CellDesc(int nX, int nY)
        {
            return m_CellDesc[nX + nY * m_W];
        }

    private:
        bool ValidP(int nX, int nY)
        {
            return nX >= 0 && nX < m_W * 48 && nY >= 0 && nY < m_H * 32;
        }

        bool ValidC(int nX, int nY)
        {
            return nX >= 0 && nX < m_W && nY >= 0 && nY < m_H;
        }

    private:
        void DrawGround();

    private:
        TILEDESC   *m_TileDesc;
        CELLDESC   *m_CellDesc;

    private:
        uint8_t    *m_Buf;
        size_t      m_BufSize;
        size_t      m_BufMaxSize;

    private:
        uint16_t    m_W;
        uint16_t    m_H;


    private:
        bool LoadHead(uint8_t * &);

    private:
        bool LoadTile(uint8_t * &);
        void ParseTile(int, int, int, const uint8_t *, long &, const uint8_t *, long &);
        void SetTile(int, int, int, const uint8_t *, long &);

    private:
        bool LoadLight(uint8_t * &);
        void ParseLight(int, int, int, const uint8_t *, long &, const uint8_t *, long &);
        void SetLight(int, int, int, const uint8_t *, long &);


    private:
        bool LoadWalk(uint8_t * &);
        void ParseWalk(int, int, int, const uint8_t *, long &);
        void SetWalk(int, int, int, bool);
        void SetOneWalk(int, int, int, bool);

    private:
        bool LoadObj(uint8_t * &, int);
        void ParseObj(int, int, int, int, const uint8_t *, long &, const uint8_t *, long &);
        void SetObj(int, int, int, int, const uint8_t *, long &, const uint8_t *, long &);
        void SetOneObj(int, int, int, const uint8_t *, long &, const uint8_t *, long &);
        void SetOneObjMask(int, int, int, bool, bool);


    private:
        void ExtendBuf(size_t);

    private:
        void DrawObject(int, int, int, int,
                std::function<bool(int, int)>,
                std::function<void(int, int)>);
};
