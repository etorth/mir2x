/*
 * =====================================================================================
 *
 *       Filename: editormap.hpp
 *        Created: 02/08/2016 22:17:08
 *  Last Modified: 07/17/2016 21:25:10
 *
 *    Description: class EditorMap has no idea of ImageDB, WilImagePackage, etc., it use
 *                 function handler to handle drawing, caching, etc..
 *
 *                 previously there are problems when drawing, illustrated as following:
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
 *                 object slices, now if at point K stands an object, then K would be
 *                 drawed after B, but before C
 *
 *                 now if object at K is ``fat" enough that its right part is partially
 *                 under C, then we get visual disorder since part of the object will be
 *                 covered by C, then the object is partially visiable to us
 *
 *                 to avoid this problem I introduced the ``object grid", and put the
 *                 lowest part of C as ``always ground", and won't draw is at the
 *                 overground object drawing step
 *
 *                 and now slice C should be aligned with the new ``start point", from L-1
 *                 rather than L-2, then for object stand at K, it's always properly drawed
 *
 *                 this need cooperation with ground walkable mask, we should set the part
 *                 of object slice C between L1 and L2 as ``non-walkable", otherwise if we
 *                 have an object stand there, meaning an object stand behind the wall, it
 *                 will shows in-properly
 *
 *                 this method introduce another problem: if at point M there stands an
 *                 object shaped as
 *                                          +-----+
 *                                          |     |
 *                                          |     |
 *                                          |     |
 *                                          |  X  | <----
 *                                          |     |      part causes trouble
 *                                          +-----+ <----
 *                 this object is ``thin" enough that below its center point X there is
 *                 more part as illustrated. when this object stand at point M, if we draw
 *                 L1 ~ L2 first then object at M, then rest of slice C, we get problem
 *                 that second half of the object is shown but first half is covered
 *
 *
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

#include <vector>
#include <string>
#include <cstdint>
#include <utility>
#include <functional>

#include "mir2map.hpp"
#include "mir2xmap.hpp"
#include "wilimagepackage.hpp"

class EditorMap
{
    private:
        // TODO new feature for mir2x map, use two bits to desc object with ``grid", they
        //      are NA, GROUND, OVERGROUND, EMPTY, I hope this design could help
        //
        // for original mir2 map, there are only GROUND and OVERGROUND object, and there
        // is no concept of ``grid object", but now I need it
        //
        // 1. GROUND and tile are drawed at the same step
        // 2. OVERGROUND and creatures are drawed at the same step
        // 3. we can micmic ROOF with OVERGROUND and EMPTY, using long object
        // 4. EMPTY grid also helps to reduce unnecessary rendering
        //
        //
        // for original mir2 map resource, empty part are always at the second half of the
        // object texture, the first pixel line of a texture should always be non-empty, so
        // with current NA, EMPTY, GROUND, OVERGROUND we can describe all grids
        //
        //
        //                   NA, to end the object slice
        //        +--------+
        //        |        |
        //        |        | GROUND / OVERGROUND
        //        |        |
        //        +--------+
        //        |        |
        //        |        | GROUND / OVERGROUND
        //        |        |
        //        +--------+
        //        |   .    |
        //
        //        |   .    |
        //        |   .    |
        //        +--------+
        //        |        |
        //        |        | EMPTY
        //        |        |
        //        +--------+
        //        |        |
        //        |        | EMPTY
        //        |        |
        //        +--------+
        //
        // how to draw this piece:
        // 1. for ground object grid, draw it at place
        // 2. for overground object grid, draw it with align point at the new ``bottom line", see
        //    explanition above in the file description
        // 3. for empty object grid, won't draw it
        //

        enum ObjectGridType: int{
            OBJGRID_NA          = 0,
            OBJGRID_EMPTY       = 1,
            OBJGRID_GROUND      = 2,
            OGJGRID_OVERGROUND  = 3,
        };

        // TODO finally I decide to make a compact cell descriptor, this is a lesson to me
        typedef struct _EditCellDesc{
            // tile
            // only use 1 / 4 of this
            uint32_t Tile;
            int      TileMark;

            // ground
            uint8_t  Ground[4];
            int      GroundMark[4];
            int      GroundSelectMark[4];

            // light
            uint16_t Light;
            int      LightMark;

            // object
            uint32_t Object[2];
            int      ObjectMark[2];
            int      AlphaObjectMark[2];
            int      AnimatedObjectMark[2];

            // object grid
            std::vector<int> ObjectGridTypeV[2];

            _EditCellDesc()
                : Tile(0)
                , TileMark(0)
                , Ground {0, 0, 0, 0}
                , GroundMark {0, 0, 0, 0}
                , GroundSelectMark {0, 0, 0, 0}
                , Light(0)
                , LightMark(0)
                , Object {0, 0}
                , ObjectMark {0, 0}
                , AlphaObjectMark {0, 0}
                , AnimatedObjectMark {0, 0}
                , ObjectGridTypeV()
            {}
        }EditCellDesc;

    private:
        int             m_W;
        int             m_H;
        bool            m_Valid;
        uint32_t        m_AniSaveTime[8];
        uint8_t         m_AniTileFrame[8][16];

    private:
        Mir2Map        *m_OldMir2Map;
        Mir2xMap       *m_Mir2xMap;

    private:
        std::vector<std::tuple<int, int>> m_SelectPointV;
        std::vector<std::vector<EditCellDesc>> m_BufEditCellDescV2D;

    public:
        EditorMap();
        ~EditorMap();

    public:
        bool LoadMir2Map(const char *);
        bool LoadMir2xMap(const char *);

    public:
        // fast api
        // user's responsability to maintain parameters and states
        bool Valid()
        {
            return m_Valid;
        }

        bool ValidC(int nX, int nY)
        {
            return nX >= 0 && nX < W() && nY >= 0 && nY < H();
        }

        bool ValidP(int nX, int nY)
        {
            return nX >= 0 && nX < 48 * W() && nY >= 0 && nY < 32 * H();
        }

        int W()
        {
            return m_W;
        }

        int H()
        {
            return m_H;
        }

        int TileValid(int nX, int nY)
        {
            return m_BufEditCellDescV2D[nX / 2][nY / 2].TileMark;
        }

        uint32_t Tile(int nX, int nY)
        {
            return m_BufEditCellDescV2D[nX / 2][nY / 2].Tile;
        }

        int ObjectValid(int nX, int nY, int nIndex)
        {
            return m_BufEditCellDescV2D[nX][nY].ObjectMark[nIndex];
        }

        uint32_t Object(int nX, int nY, int nIndex)
        {
            return m_BufEditCellDescV2D[nX][nY].Obj[nIndex];
        }

        int GroundObjectValid(int nX, int nY,  int nIndex)
        {
            return m_BufEditCellDescV2D.GroundObjectMark[nX][nY][nIndex];
        }

        int AlphaObjectValid(int nX, int nY,  int nIndex)
        {
            return m_BufEditCellDescV2D.AlphaObjectMark[nX][nY][nIndex];
        }

        int GroundSelect(int nX, int nY, int nIndex)
        {
            return m_BufEditCellDescV2D.GroundSelectMark[nX][nY][nIndex];
        }

        bool AniObjectValid(int nX, int nY, int nIndex)
        {
            return m_BufEditCellDescV2D.AniObjectMark[nX][nY][nIndex];
        }

        int LightValid(int nX, int nY)
        {
            return m_BufEditCellDescV2D.LightMark[nX][nY];
        }

        uint16_t Light(int nX, int nY)
        {
            return m_BufEditCellDescV2D.Light[nX][nY];
        }

        int GroundValid(int nX, int nY, int nIndex)
        {
            return m_BufEditCellDescV2D.GroundMark[nX][nY][nIndex];
        }

        uint8_t Ground(int nX, int nY, int nIndex)
        {
            return m_BufEditCellDescV2D.Ground[nX][nY][nIndex];
        }

        uint16_t ObjectOff(int nAniType, int nAniCnt)
        {
            return (uint16_t)(m_bAniTileFrame[nAniType][nAniCnt]);
        }

        void SetGround(int nX, int nY, int nIndex, bool bValid, uint8_t nDesc)
        {
            m_BufEditCellDescV2D.GroundMark[nX][nY][nIndex] = ((bValid) ? 1 : 0);
            m_BufEditCellDescV2D.Ground[nX][nY][nIndex]     = nDesc;
        }

        void SetObject(int nX, int nY, int nIndex, bool bValid, uint32_t nDesc)
        {
            m_BufEditCellDescV2D.ObjectMark[nX][nY][nIndex] = ((bValid) ? 1 : 0);
            m_BufEditCellDescV2D.Obj[nX][nY][nIndex]     = nDesc;
        }

        void SetGroundObject(int nX, int nY, int nIndex, int nGroundObj)
        {
            m_BufEditCellDescV2D.GroundObjectMark[nX][nY][nIndex] = nGroundObj;
        }

    public:
        // for map resource extraction
        void ExtractOneTile(int, int, std::function<void(uint8_t, uint16_t)>);
        void ExtractTile(std::function<void(uint8_t, uint16_t)>);

        void ExtractOneObject(int, int, int, std::function<void(uint8_t, uint16_t, uint32_t)>);
        void ExtractObject(std::function<void(uint8_t, uint16_t, uint32_t)>);

    public:
        // draw map
        // external class will provide handlers for physical draw function
        void DrawTile(int, int, int, int, std::function<void(uint8_t, uint16_t, int, int)>);
        void DrawObject(int, int, int, int, bool, std::function<void(uint8_t, uint16_t, int, int)>, std::function<void(int, int)>);
        void DrawLight(int, int, int, int, std::function<void(int, int)>);
        void DrawSelectGround(int, int, int, int, std::function<void(int, int, int)>);
        void DrawSelectPoint(std::function<void(const std::vector<std::pair<int, int>> &)>);

    public:
        // selection operation
        void AddSelectPoint(int, int);
        void SetGroundSelect(int, int, int, int);
        void ClearGroundSelect();

    public:
        void CompressLight(std::vector<bool> &, std::vector<uint8_t> &);
        void CompressGround(std::vector<bool> &, std::vector<uint8_t> &);
        void CompressTile(std::vector<bool> &, std::vector<uint8_t> &);
        void CompressObject(std::vector<bool> &, std::vector<uint8_t> &, int);

    public:
        void DoCompressGround(int, int, int, std::vector<bool> &, std::vector<uint8_t> &);
        void DoCompressLight(int, int, int, std::vector<bool> &, std::vector<uint8_t> &);
        void DoCompressTile(int, int, int, std::vector<bool> &, std::vector<uint8_t> &);
        void DoCompressObject(int, int, int, int, std::vector<bool> &, std::vector<uint8_t> &);

    public:
        void RecordGround(std::vector<uint8_t> &, int, int, int);
        void RecordLight(std::vector<uint8_t> &, int, int);
        void RecordObject(std::vector<bool> &, std::vector<uint8_t> &, int, int, int);
        void RecordTile(std::vector<uint8_t> &, int, int);

    public:
        void UpdateFrame(int);
        bool Resize(int, int, int, int, int, int, int, int);

    public:
        int ObjectBlockType(int, int, int, int);
        int GroundBlockType(int, int, int, int);
        int LightBlockType(int, int, int);
        int TileBlockType(int, int, int);

    public:
        // save to mir2x compact format
        bool Save(const char *);

    private:
        void PushData(const std::vector<bool> &, const std::vector<uint8_t> &, std::vector<uint8_t> &);
        void PushBit(const std::vector<bool> &, std::vector<uint8_t> &);

    public:
        void Optimize();
        void OptimizeTile(int, int);
        void OptimizeCell(int, int);

    private:
        void ClearBuf();
        void MakeBuf(int, int);
        bool InitBuf();

        void SetBufTile(int, int);
        void SetBufLight(int, int);
        void SetBufObj(int, int, int);
        void SetBufGround(int, int, int);

    public:
        std::string MapInfo();

    public:
        bool LocateObject(int, int, int *, int *, int *, int, const std::function<int(uint32_t)> &);
};
