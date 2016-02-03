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
            uint8_t     Desc;
            OBJDESC     Obj[2];
            uint16_t    Light;
        }CELLDESC;

    public:
        Mir2xMap();
        ~Mir2xMap();

    public:
        int  W();
        int  H();

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
        bool LoadHead(uint8_t *);
        bool LoadWalk(uint8_t *);
        bool LoadLight(uint8_t *);
        bool LoadTile(uint8_t *);
        bool LoadObj1(uint8_t *);
        bool LoadObj2(uint8_t *);

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
        inline TILEDESC &TileDesc(int nX, int nY)
        {
            return m_TileDesc[nX / 2 + nY / 2 * m_H / 2];
        }

        inline CELLDESC &CellDesc(int nX, int nY)
        {
            return m_TileDesc[nX + nY * m_H ];
        }

    private:
        inline bool ValidP(int nX, int nY)
        {
            return nX >= 0 && nX < m_W && nY >= 0 && nY < m_H;
        }

    private:
        void DrawGround();

    private:
        void DrawObject(int, int, int, int,
                std::function<bool(int, int)>,
                std::function<void(int, int)>);
};
