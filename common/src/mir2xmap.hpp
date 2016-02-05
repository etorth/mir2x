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
        TILEDESC   *m_TileDesc;
        CELLDESC   *m_CellDesc;

    private:
        uint16_t    m_W;
        uint16_t    m_H;

    private:
        uint32_t    m_dwAniSaveTime[8];
        uint8_t     m_bAniTileFrame[8][16];

    private:
        bool LoadObj1(uint8_t * &);
        bool LoadObj2(uint8_t * &);

    public:
        void DrawBaseTile(int, int, int, int);
        void DrawGroundObject(int, int, int, int);
        void DrawOverGroundObject(int, int, int, int, std::function<void(int, int)>);

    private:
        bool PickOneBit(const uint8_t *pData, long nOffset)
        {
            return (pData[nOffset / 8] & (0X01 << (nOffset) % 8)) != 0;
        }

    public:
        bool Overlap(int);

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

        uint8_t *m_Buf;

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
        void ParseWalk(int, int, const uint8_t *, long &);
        void SetWalk(int, int, int, bool);
        void SetOneWalk(int, int, int, bool);

    private:
        bool LoadObj(uint8_t * &, int);
        void ParseObj(int, int, int, int, const uint8_t *, long &, const uint8_t *, long &);
        void SetObj(int, int, int, int, const uint8_t *, long &, const uint8_t *, long &);
        void SetOneObj(int, int, int, const uint8_t *, long &, const uint8_t *, long &);
        void SetOneObjMask(int, int, int, bool, bool);


    private:
        void ExtendBuf(long);

    private:
        void DrawObject(int, int, int, int,
                std::function<bool(int, int)>,
                std::function<void(int, int)>);
};
