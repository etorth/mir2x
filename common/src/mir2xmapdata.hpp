/*
 * =====================================================================================
 *
 *       Filename: mir2xmapdata.hpp
 *        Created: 08/31/2015 18:26:57
 *  Last Modified: 03/24/2017 13:43:31
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
#include <vector>
#include <cstdint>
#include <functional>

class Mir2xMapData
{
    public:
        // pod types
        // can be initialized by { xx, xx, xxx }
#pragma pack(push, 1)
        typedef struct
        {
            uint32_t Param;     // bit field definition:
                                //      31 : valid
                                // 30 - 28 :
                                // 27 - 24 : index
                                // 23 - 16 : index for file
                                // 15 -  0 : index for image
        }TILE;

        // define OBJ as ``one frame of OBJ"
        // then move the animation information out
        // since we can define a sequence of animated obj as an animation
        typedef struct
        {
            uint32_t Param;     // bit field definition:
                                //      31 : valid
                                // 30 - 28 :
                                // 27 - 24 : index
                                // 23 - 16 : index for file
                                // 15 -  0 : index for image
        }OBJ;

        typedef struct
        {
            uint32_t Param;     // bit field definition
                                //      31 : valid
                                // 30 - 24 :
                                //
                                //      23 : can walk
                                // 22 - 16 :
                                //
                                //      15 : valid light
                                // 14 - 08 :
                                //
                                // 07 - 00 :

            uint32_t ObjParam;  // define how to use the Obj in current cell
                                // bit field definition:
                                //      31 : obj-1 : animated
                                // 30 - 28 :       : tick type of animation
                                // 27 - 24 :       : frame count, max = 16
                                //      23 :       : alpha
                                //      22 :       : ground
                                // 21 - 16 :       :
                                //
                                //      15 : obj-0 : animated
                                // 14 - 12 :       : tick type of animation
                                // 11 - 08 :       : frame count, max = 16
                                //      07 :       : alpha
                                //      06 :       : ground
                                // 05 - 00 :       :

            // information of ``one frame" of object
            // animation infomation and blending infomation are in ObjParam
            OBJ Obj[2];
        }CELL;

        typedef struct
        {
            TILE   Tile[1];
            CELL   Cell[4];
        }BLOCK;
#pragma pack(pop)

    private:
        uint16_t m_W;
        uint16_t m_H;

    private:
        // previously I would use std::vector<uint8_t> to work as a buffer
        // but never since now it's undefined because of the strict aliasing rule
        std::vector<BLOCK> m_Data;

    public:
        Mir2xMapData()
            : m_W(0)
            , m_H(0)
            , m_Data()
        {}

    public:
        const uint8_t *Data() const
        {
            return (uint8_t *)(&m_Data[0]);
        }

        size_t Size() const
        {
            return m_Data.size() * sizeof(m_Data[0]);
        }

    public:
        bool Allocate(uint16_t, uint16_t);

    public:
        int W() const { return m_W; }
        int H() const { return m_H; }

    public:
        auto &Block(int nX, int nY)
        {
            return m_Data[nX / 2 + (nY / 2) * (m_W / 2)];
        }

        auto &Tile(int nX, int nY)
        {
            return Block(nX, nY).Tile[0];
        }

        auto &Cell(int nX, int nY)
        {
            return Block(nX, nY).Cell[(nY % 2) * 2 + (nX % 2)];
        }

    public:
        int Load(const char *);
        int Save(const char *);

    private:
        bool PickOneBit(const uint8_t *pData, size_t nOffset)
        {
            return (pData[nOffset / 8] & (0X01 << (nOffset) % 8)) != 0;
        }

    public:
        bool Valid() const
        {
            return !m_Data.empty();
        }

        bool ValidC(int nX, int nY) const
        {
            return nX >= 0 && nX < m_W && nY >= 0 && nY < m_H;
        }

        bool ValidP(int nX, int nY) const
        {
            return nX >= 0 && nX < m_W * 48 && nY >= 0 && nY < m_H * 32;
        }

    private:
        int LoadHead(uint8_t * &);
        int LoadGrid(uint8_t * &, std::function<int(int, int, int, const uint8_t *, size_t &, const uint8_t *, size_t &)>);
        int ParseGrid(int, int, int, int, const uint8_t *, size_t &, const uint8_t *, size_t &, std::function<int(int, int, int, const uint8_t *, size_t &, const uint8_t *, size_t &)>);

        int GridAttrType(int, int, int, int, std::function<int(int, int)>);
        int SaveGrid(std::vector<bool> &, std::vector<uint8_t> &, std::function<int(int, int, int, std::vector<bool> &, std::vector<uint8_t> &)>);
        int CompressGrid(int, int, int, int, std::vector<bool> &, std::vector<uint8_t> &, std::function<int(int, int, int)>, std::function<int(int, int, std::vector<bool> &, std::vector<uint8_t> &)>);

        int SetTile(int, int, int, const uint8_t *, size_t &);
        int SetCell(int, int, int, const uint8_t *, size_t &);
        int SetObj(int, int, int, int, const uint8_t *, size_t &, const uint8_t *, size_t &);

    private:
        void PushBit(const std::vector<bool> &, std::vector<uint8_t> &);
        void PushData(const std::vector<bool> &, const std::vector<uint8_t> &, std::vector<uint8_t> &);
};
