#pragma once
#include <cstdint>
#include <functional>

class Mir2xMap
{
    private:
        typedef struct
        {
            uint8_t     Desc;           // 0x<-87654321:
                                        //   8: valid or not
                                        // 7~1: specified info of tile
            uint8_t     FileIndex;
            uint16_t    ImageIndex;
        }TILEDESC;

        typedef struct
        {
            uint8_t     Desc;           // description for animated object
                                        // 0x <- 87654321
                                        //     8: animated
                                        //   7-5: tick type, define animation fps
                                        //   4-1: frame count, for max = 16
            uint8_t     FileIndex;  
            uint16_t    ImageIndex;
        }OBJDESC;

        typedef struct
        {
            uint16_t    Desc;           //  0x b(87654321) a(87654321)
                                        //
                                        //  b8: obj-1 is alpha blended
                                        //  b7: 
                                        //  b6: obj-1 is ground obj
                                        //  b5  obj-1 valid
                                        //
                                        //  b4: obj-0 is alpha blended
                                        //  b3: 
                                        //  b2: obj-0 is ground obj
                                        //  b1: obj-0 valid
                                        //
                                        //  a8: light
                                        //  a7:
                                        //  a6:
                                        //  a5:
                                        //  a4: gournd subgrid-4 is valid
                                        //  a3: gournd subgrid-3 is valid
                                        //  a2: gournd subgrid-2 is valid
                                        //  a1: gournd subgrid-1 is valid
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

    public:
        void Draw(int, int, int, int, int, int,
                const std::function<void(int, int, uint32_t)> &,
                const std::function<void(int, int, uint32_t)> &,
                const std::function<void(int, int)> &,
                const std::function<void(int, int)> &);

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
            // desc is for animation mark and animation info
            // not for alpha anymore
            return (((uint32_t)stDesc.Desc) << 24) + (((uint32_t)stDesc.FileIndex) << 16) + ((uint32_t)stDesc.ImageIndex);
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
            return (CellDesc(nX, nY).Desc & 0X0080) != 0;
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
            return (TileDesc(nX, nY).Desc & 0X80) != 0;
        }

        bool ObjectValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & ((nIndex == 0) ? 0X0100 : 0X1000)) != 0;
        }

        bool AniObjectValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Obj[nIndex].Desc & 0X80) != 0;
        }

        bool GroundObjectValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & ((nIndex == 0) ? 0X0200 : 0X2000)) != 0;
        }

        bool AlphaObjectValid(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & ((nIndex == 0) ? 0X0800 : 0X8000)) != 0;
        }

    private:
        bool PickOneBit(const uint8_t *pData, long nOffset)
        {
            return (pData[nOffset / 8] & (0X01 << (nOffset) % 8)) != 0;
        }

    public:
        bool Overlap(int, int, int, int, int);

    public:
        bool CanWalk(int nX, int nY, int nIndex)
        {
            return (CellDesc(nX, nY).Desc & ((0X01) << (nIndex % 4))) != 0;
        }

        bool CanWalk(int nX, int nY){
            // TODO
            // maybe it should be ``or", not ``and"
            // go over it later
            return true
                && CanWalk(nX, nY, 0)
                && CanWalk(nX, nY, 1)
                && CanWalk(nX, nY, 2)
                && CanWalk(nX, nY, 3);
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
