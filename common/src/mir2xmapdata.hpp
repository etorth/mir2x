/*
 * =====================================================================================
 *
 *       Filename: mir2xmapdata.hpp
 *        Created: 08/31/2015 18:26:57
 *  Last Modified: 08/18/2017 15:59:23
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
#include <array>
#include <vector>
#include <cstdint>
#include <functional>
#include "landtype.hpp"
#include "sysconst.hpp"

class Mir2xMapData final
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
            bool Valid() const
            {
                return (Param & 0X80000000) ? true : false;
            }

            uint32_t Image() const
            {
                return Param & 0X00FFFFFF;
            }
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
            bool Valid() const
            {
                return (Param & 0X80000000) ? true : false;
            }

            uint32_t Image() const
            {
                return Param & 0X00FFFFFF;
            }
        }OBJ;

        typedef struct
        {
            // removed the ``valid" field for cell
            // one cell should always be there and ``valid"
            // but its objects, light, etc could be ``invalid" and specify independently

            uint32_t Param;     // bit field definition
                                // 31 - 24 :
                                //
                                //      23 : can walk
                                //      22 : can fly
                                // 21 - 16 : land type
                                //
                                //      15 : light valid
                                // 14 - 13 : light radius
                                // 12 - 11 : light alpha
                                // 10 - 08 : light color
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

            bool CanWalk() const
            {
                return (Param & 0X00800000) ? true : false;
            }

            bool CanFly() const
            {
                return (Param & 0X00400000) ? true : false;
            }

            uint8_t LandType() const
            {
                return (uint8_t)((Param & 0X003F0000) >> 16);
            }

            bool CanThrough() const
            {
                return (CanWalk() || CanFly())
                    && (LandType() > LANDTYPE_NONE && LandType() < LANDTYPE_MAX);
            }

            uint8_t LandByte() const
            {
                return (Param & 0X00FF0000) >> 16;
            }

            uint8_t LightByte() const
            {
                return (Param & 0X0000FF00) >> 8;
            }

            std::array<uint8_t, 5> ObjectArray(int nIndex) const
            {
                if(nIndex == 0){
                    uint8_t nByte4 = 0
                        | (((uint8_t)((Obj[0].Param & 0X80000000) ? 1 : 0)) << 7)   // valid
                        | (((uint8_t)((ObjParam     & 0X00000080) ? 1 : 0)) << 1)   // alpha
                        | (((uint8_t)((ObjParam     & 0X00000040) ? 1 : 0)) << 0);  // ground

                    uint8_t nByte3 = (uint8_t)((ObjParam     & 0X0000FF00) >>  8);
                    uint8_t nByte2 = (uint8_t)((Obj[0].Param & 0X00FF0000) >> 16);
                    uint8_t nByte1 = (uint8_t)((Obj[0].Param & 0X0000FF00) >>  8);
                    uint8_t nByte0 = (uint8_t)((Obj[0].Param & 0X000000FF) >>  0);
                    return {nByte0, nByte1, nByte2, nByte3, nByte4};
                }else if(nIndex == 1){
                    uint8_t nByte4 = 0
                        | (((uint8_t)((Obj[1].Param & 0X80000000) ? 1 : 0)) << 7)   // valid
                        | (((uint8_t)((ObjParam     & 0X00800000) ? 1 : 0)) << 1)   // alpha
                        | (((uint8_t)((ObjParam     & 0X00400000) ? 1 : 0)) << 0);  // ground

                    uint8_t nByte3 = (uint8_t)((ObjParam     & 0XFF000000) >> 24);
                    uint8_t nByte2 = (uint8_t)((Obj[1].Param & 0X00FF0000) >> 16);
                    uint8_t nByte1 = (uint8_t)((Obj[1].Param & 0X0000FF00) >>  8);
                    uint8_t nByte0 = (uint8_t)((Obj[1].Param & 0X000000FF) >>  0);
                    return {nByte0, nByte1, nByte2, nByte3, nByte4};
                }else{
                    return {0, 0, 0, 0, 0};
                }
            }
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

        Mir2xMapData(const char *pName)
            : Mir2xMapData()
        {
            Load(pName);
        }

    public:
        const uint8_t *Data() const
        {
            return (uint8_t *)(&m_Data[0]);
        }

        size_t DataLen() const
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
        const auto &Block(int nX, int nY) const
        {
            return m_Data[nX / 2 + (nY / 2) * (m_W / 2)];
        }

        const auto &Tile(int nX, int nY) const
        {
            return Block(nX, nY).Tile[0];
        }

        const auto &Cell(int nX, int nY) const
        {
            return Block(nX, nY).Cell[(nY % 2) * 2 + (nX % 2)];
        }

    public:
        int Load(const char *);
        int Save(const char *);

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
            return nX >= 0 && nX < m_W * SYS_MAPGRIDXP && nY >= 0 && nY < m_H * SYS_MAPGRIDYP;
        }

    private:
        int LoadHead(uint8_t * &);
        int LoadGrid(uint8_t * &, std::function<int(int, int, int, const uint8_t *, size_t &, const uint8_t *, size_t &)>);
        int ParseGrid(int, int, size_t, size_t, const uint8_t *, size_t &, const uint8_t *, size_t &, std::function<int(int, int, int, const uint8_t *, size_t &)>);

        int GridAttrType(int, int, int, int, std::function<int(int, int)>);
        int SaveGrid(std::vector<bool> &, std::vector<uint8_t> &, std::function<int(int, int, int, std::vector<bool> &, std::vector<uint8_t> &)>);
        int CompressGrid(int, int, int, int, std::vector<bool> &, std::vector<uint8_t> &, std::function<int(int, int, int)>, std::function<int(int, int, std::vector<bool> &, std::vector<uint8_t> &)>);

    public:
        int SaveHead (std::vector<uint8_t> &);
        int SaveTile (std::vector<uint8_t> &);
        int SaveLand (std::vector<uint8_t> &);
        int SaveLight(std::vector<uint8_t> &);
        int SaveObj  (std::vector<uint8_t> &, int);

    public:
        int SetTile (int, int, int,      const uint8_t *, size_t &);
        int SetLight(int, int, int,      const uint8_t *, size_t &);
        int SetLand (int, int, int,      const uint8_t *, size_t &);
        int SetObj  (int, int, int, int, const uint8_t *, size_t &);

    public:
        void PushData(std::vector<uint8_t> &, const std::vector<bool> &, const std::vector<uint8_t> &);
};
