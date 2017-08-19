/*
 * =====================================================================================
 *
 *       Filename: editormap.hpp
 *        Created: 02/08/2016 22:17:08
 *  Last Modified: 08/18/2017 17:50:36
 *
 *    Description: EditorMap has no idea of ImageDB, WilImagePackage, etc..
 *                 Use function handler to handle draw, cache, etc
 *
 *                 I spent really a long time to think about this part, for how to
 *                 arrange the object and show it correctly, following are some
 *                 problems I need to handle.
 *
 *                 =====================================================================
 *                 Problem 1:
 *                             
 *                             |     |     |     |
 *                             |*****|     |     |
 *                             |**A**|     |     |
 *                             +-----+     |     |
 *                                   |*****|     |
 *                                   |**B**|  M  |
 *                                   +-----+     |    <---- L-1
 *                                      K  |*****|
 *                                         |**C**|
 *                                         +-----+    <---- L-2
 *
 *                 say now A, B, C are all wall slices, then they should be as overground
 *                 object slices, now if at point K stands an object, then by the old
 *                 rendering method, K will be rendered after B, but before C.
 *
 *                 now if object at K is ``fat" enough that its right part partially
 *                 coincides with C, then we get problem since C is drawn after the object
 *                 and the coinciding part of it will be covered by C
 *
 *                 =====================================================================
 *                 Problem 2:
 *
 *                                          |  .  |
 *                                          |  .  |
 *                                          |  .  |
 *                                          |*****|
 *                                          |*****|  2
 *                                          |*****|
 *                                          |-----|
 *                                          |*****|
 *                                          |**** |  1
 *                                          |***  |
 *                                          |-----|
 *                                          |*    |
 *                                          |     |  0
 *                                          | K   |
 *                                          +-----+
 *
 *                 This problem is caused by how the visual data arranged in mir2, as
 *                 above now this is an object slice, we cut it into small cells of size
 *                 48 * 32, then even if there is only a very small part of the visable
 *                 object available inside grid 0, still we have an whole grid and make
 *                 it as ``over-ground"
 *
 *                 Now if there is a object standing at point K, then we expect that it
 *                 should stand in front of the slice, but sadly by current rendering
 *                 rule since we will draw object first and then slice, then the object
 *                 will be covered by the slice
 *
 *                 =====================================================================
 *                 Problem 2:
 *
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

#include <string>
#include <vector>
#include <cstdint>
#include <utility>
#include <functional>

#include "mir2map.hpp"
#include "sysconst.hpp"
#include "landtype.hpp"
#include "pushstream.hpp"
#include "mir2xmapdata.hpp"
#include "wilimagepackage.hpp"

class EditorMap
{
    private:
        struct stTile_t
        {
            bool     Valid;
            uint32_t Image;

            uint32_t MakeU32() const
            {
                if(Valid){
                    return 0X80000000 | (Image & 0X00FFFFFF);
                }else{
                    return 0X00000000;
                }
            }
        };

        struct stObj_t
        {
            bool Valid;

            bool Alpha;
            bool Ground;
            bool Animated;

            uint8_t AniType;
            uint8_t AniCount;

            uint32_t Image;

            std::array<uint8_t, 5> MakeArray() const
            {
                if(Valid){
                    uint8_t nByte4 = 0
                        | (((uint8_t)(Valid  ? 1 : 0)) << 7)
                        | (((uint8_t)(Alpha  ? 1 : 0)) << 1)
                        | (((uint8_t)(Ground ? 1 : 0)) << 0);

                    uint8_t nByte3 = 0
                        | (((uint8_t)(Animated ? 1 : 0)) << 7)
                        | (((uint8_t)(AniType  & 0X07 )) << 4)
                        | (((uint8_t)(AniCount & 0X0F )) << 0);

                    uint8_t nByte2 = (uint8_t)((Image & 0X00FF0000) >> 16);
                    uint8_t nByte1 = (uint8_t)((Image & 0X0000FF00) >>  8);
                    uint8_t nByte0 = (uint8_t)((Image & 0X000000FF) >>  0);

                    return {nByte0, nByte1, nByte2, nByte3, nByte4};
                }else{
                    return {0, 0, 0, 0, 0};
                }
            }
        };

        struct stLight_t
        {
            bool Valid;

            // TODO
            // need design light struture

            // 1. index of light color
            // 2. index of light alpha
            // 3. index of light radius
            uint8_t Color;
            uint8_t Alpha;
            uint8_t Radius;

            uint8_t MakeU8() const
            {
                if(Valid){
                    return 0
                        | (((uint8_t)(Valid ? 1 : 0)) << 7)     // [ :7]
                        | (((uint8_t)(Radius & 0X03)) << 5)     // [6:5]
                        | (((uint8_t)(Alpha  & 0X03)) << 3)     // [4:3]
                        | (((uint8_t)(Color  & 0X07)) << 0);    // [2:0]
                }else{
                    return 0X00;
                }
            }
        };

        struct stCell_t
        {
            bool    CanWalk;
            bool    CanFly;
            uint8_t LandType;

            stLight_t Light;
            stObj_t   Obj[2];

            // rest for editor use only
            bool SelectGround;

            // an cell is always valid
            // it may contains light and objects and indicated by ``Valid"
            uint8_t MakeLandU8() const
            {
                return 0
                    | (((uint8_t)(CanWalk ? 1 : 0)) << 7)   // [  7]
                    | (((uint8_t)(CanFly  ? 1 : 0)) << 6)   // [  6]
                    | (((uint8_t)(LandType & 0X3F)) << 0);  // [5:0]
            }

            bool CanThrough()
            {
                return (CanWalk || CanFly);
            }
        };

        struct stBlock_t
        {
            stTile_t Tile;
            stCell_t Cell[2][2];
        };

    private:
        int     m_W;
        int     m_H;
        bool    m_Valid;

    private:
        uint32_t m_AniSaveTime [8];
        uint8_t  m_AniTileFrame[8][16];

    private:
        Mir2Map      *m_Mir2Map;
        Mir2xMapData *m_Mir2xMapData;

    private:
        std::vector<std::vector<stBlock_t>> m_BlockBuf;

    public:
        EditorMap();
       ~EditorMap();

    public:
        bool LoadMir2Map(const char *);
        bool LoadMir2xMapData(const char *);

    public:
        bool Valid() const
        {
            return m_Valid;
        }

        bool ValidC(int nX, int nY) const
        {
            return nX >= 0 && nX < W() && nY >= 0 && nY < H();
        }

        bool ValidP(int nX, int nY) const
        {
            return nX >= 0 && nX < SYS_MAPGRIDXP * W() && nY >= 0 && nY < SYS_MAPGRIDYP * H();
        }

    public:
        int W() const
        {
            return m_W;
        }

        int H() const
        {
            return m_H;
        }

        auto &Tile(int nX, int nY)
        {
            return m_BlockBuf[nX / 2][nY / 2].Tile;
        }

        auto &Cell(int nX, int nY)
        {
            return m_BlockBuf[nX / 2][nY / 2].Cell[nX % 2][nY % 2];
        }

        auto &Light(int nX, int nY)
        {
            return Cell(nX, nY).Light;
        }

        auto &Object(int nX, int nY, int nIndex)
        {
            return Cell(nX, nY).Obj[nIndex];
        }

    public:
        void ExtractTile(std::function<void(uint8_t, uint16_t)>);
        void ExtractOneTile(int, int, std::function<void(uint8_t, uint16_t)>);

        void ExtractObject(std::function<void(uint8_t, uint16_t, uint32_t)>);
        void ExtractOneObject(int, int, int, std::function<void(uint8_t, uint16_t, uint32_t)>);

    public:
        void DrawTile(int, int, int, int, std::function<void(uint8_t, uint16_t, int, int)>);
        void DrawObject(int, int, int, int, bool, std::function<void(uint8_t, uint16_t, int, int)>, std::function<void(int, int)>);
        void DrawLight(int, int, int, int, std::function<void(int, int)>);
        void DrawSelectGround(int, int, int, int, std::function<void(int, int, int)>);

    public:
        void ClearGroundSelect();

    public:
        void UpdateFrame(int);
        bool Resize(int, int, int, int, int, int, int, int);

    public:
        bool SaveMir2xMapData(const char *);

    public:
        void Optimize();
        void OptimizeTile(int, int);
        void OptimizeCell(int, int);

    private:
        bool InitBuf();
        void ClearBuf();
        void MakeBuf(int, int);

        void SetBufTile  (int, int);
        void SetBufLight (int, int);
        void SetBufGround(int, int);
        void SetBufObj   (int, int, int);
};
