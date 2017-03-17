/*
 * =====================================================================================
 *
 *       Filename: mir2xmapdata.hpp
 *        Created: 08/31/2015 18:26:57
 *  Last Modified: 03/16/2017 23:30:56
 *
 *    Description: class to record data for mir2x map
 *                 this class won't define operation over the data
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
#include <cstdint>
#include <functional>

class Mir2xMapData
{
    private:
        uint16_t m_W;
        uint16_t m_H;

    private:
        std::vector<BLOCK> m_Data;

    private:
        // pod types
        // can be initialized by { xx, xx, xxx }
        typedef struct
        {
            uint32_t Param;     // bit field definition:
                                //      31 : valid
                                // 30 - 24 :
                                // 23 - 16 : index for file
                                // 15 -  0 : index for image
            auto Valid()
            {
                return Param & 0X80000000;
            }

            auto TexID()
            {
                return Param & 0X00FFFFFF;
            }

            void Init(bool bValid, uint32_t nTexID)
            {
                Param = (bValid ? 0X80000000 : 0X00000000) | (nTexID & 0X00FFFFFF);
            }
        }TILE;

        typedef struct
        {
            uint32_t Param;     // bit field definition:
                                //      31 : animated
                                // 30 - 28 : tick type for animation FPS
                                // 27 - 24 : frame count, max = 16
                                // 23 - 16 : index for file
                                // 15 -  0 : index for image
            auto Animated()
            {
                return Param & 0X80000000;
            }

            auto TickType()
            {
                return (Param & 0X70000000) >> 28;
            }

            auto FrameCount()
            {
                return (Param & 0X0F000000) >> 24;
            }

            auto TexID()
            {
                return Param & 0X00FFFFFF;
            }

            void Init(bool bAnimated, int nTickType, int nFrameCount, uint32_t nTexID)
            {
                Param = (bAnimated ? 0X80000000 : 0X00000000) | ((((uint32_t)(nTickType)) & 0X07) << 28) | ((((uint32_t)(nFrameCount)) & 0X0F) << 24) | (nTexID & 0X00FFFFFF);
            }
        }OBJ;

        typedef struct
        {
            uint32_t Param;     // bit field definition:
                                //      31 : obj-1 is alpha blended
                                //      30 :
                                //      29 : obj-1 is ground
                                //      28 : obj-1 is valid
                                //
                                //      27 : obj-0 is alpha blended
                                //      26 :
                                //      25 : obj-0 is ground
                                //      24 : obj-0 is valid
                                //
                                //      23 : light
                                //      15 : can walk
                                // 14 -  0 :

            // object 1, 2
            // validness information stays in Param
            // OBJ has no member to indicate its validness
            OBJ Obj[2];

            auto AlphaObjectValid(int nIndex)
            {
                return Param & (nIndex ? 0X80000000 : 0X08000000);
            }

            auto GroundObjectValid(int nIndex)
            {
                return Param & (nIndex ? 0X20000000 : 0X02000000);
            }

            auto ObjectValid(int nIndex)
            {
                return Param & (nIndex ? 0X10000000 : 0X01000000);
            }

            auto Light()
            {
                return Param & 0X00800000;
            }

            auto CanWalk()
            {
                return Param & 0X00008000;
            }
        }CELL;

        typedef struct
        {
            TILE   Tile[1];
            CELL   Cell[4];
        }BLOCK;

    public:
        Mir2xMapData()
            : m_W(0)
            , m_H(0)
            , m_Data()
        {}

    private:
        auto &Block(int nX, int nY)
        {
            return m_Data[nX / 2 + (nY / 2) * m_W / 2];
        }

    public:
        auto &Tile(int nX, int nY)
        {
            return Block(nX, nY).Tile[0];
        }

        auto &Cell(int nX, int nY)
        {
            return Block(nX, nY).Cell[(nY % 2) * 2 + (nX % 2)];
        }

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
        bool PickOneBit(const uint8_t *pData, long nOffset)
        {
            return (pData[nOffset / 8] & (0X01 << (nOffset) % 8)) != 0;
        }

    public:
        bool Overlap(int, int, int, int, int);

    public:
        bool CanWalkP(int, int, int);

    public:
        bool Valid()
        {
            return !m_Data.empty();
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
};
