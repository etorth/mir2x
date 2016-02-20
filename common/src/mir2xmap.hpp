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
            uint8_t     Ground[4];
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

    public:
        // fast api, user need to take care of validation of parameters
        //
        uint32_t Object(int nX, int nY, int nIndex)
        {
            auto &stDesc = CellDesc(nX, nY).Obj[nIndex];
            return (((uint32_t)stDesc.Desc) << 24) + (((uint32_t)stDesc.FileIndex) << 16) + ((uint32_t)stDesc.Desc);
        }

        uint32_t Tile(int nX, int nY)
        {
            return ((uint32_t)(TileDesc(nX, nY).FileIndex) << 16) + ((uint32_t)(TileDesc(nX, nY).ImageIndex));
        }

        uint8_t Ground(int nX, int nY, int nIndex)
        {
            return CellDesc(nX, nY).Ground[nIndex];
        }

        bool LightValid(int nX, int nY)
        {
            return (CellDesc(nX, nY).Desc & 0X8000) != 0;
        }

        uint16_t Light(int nX, int nY)
        {
            return CellDesc(nX, nY).Light;
        }

        bool GroundValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & (0X01 << nIndex)) != 0;
        }

        bool TileValid(int nX, int nY)
        {
            return (TileDesc(nX, nY).Desc & 0X80) == 0X80;
        }

        bool ObjectValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & (0X0200 << (nIndex * 2))) != 0;
        }

        bool AniObjectValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Obj[nIndex].Desc & 0X80) != 0;
        }

        bool GroundObjectValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & (0X0300 << (nIndex * 2))) == (0X0200 << (nIndex * 2));
        }

        bool OverGroundObjectValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & (0X0300 << (nIndex * 2))) == (0X0300 << (nIndex * 2));
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

    public:
        bool Valid()
        {
            return m_Valid;
        }

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
            bool LoadGround(uint8_t * &);
            void ParseGround(int, int, int, const uint8_t *, long &, const uint8_t *, long &);
            void SetGround(int, int, int, bool, uint8_t);
            void SetOneGround(int, int, int, bool, uint8_t);

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

    private:
            bool        m_Valid;
            uint32_t    m_dwAniSaveTime[8];
            uint8_t     m_bAniTileFrame[8][16];

};
