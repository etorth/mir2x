/*
 * =====================================================================================
 *
 *       Filename: layereditarea.hpp
 *        Created: 07/26/2017 04:27:57
 *  Last Modified: 08/24/2017 15:56:29
 *
 *    Description:
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
#include "basearea.hpp"

class LayerEditArea: public BaseArea
{
    private:
        int m_MouseX;
        int m_MouseY;

    private:
        int m_OffsetX;
        int m_OffsetY;

    public:
        LayerEditArea(int, int, int, int);

    public:
        ~LayerEditArea() = default;

    public:
        // required overriding function
        void draw();
        int  handle(int);

    public:
        void SetOffset(int, bool, int, bool);

    public:
        int OffsetX()
        {
            return m_OffsetX;
        }

        int OffsetY()
        {
            return m_OffsetY;
        }

    private:
        void DrawGrid();
        void DrawTile();
        void DrawLight();
        void DrawGround();
        void DrawObject(bool);
        void DrawAttributeGrid();

    private:
        // TODO
        // require drawarea is fully inside of window
        // draw functions with margin cut-off, using *LayerEditArea* coordinates
        // 1. not window coordinates
        // 2. not map coordinates
        //
        // why not for window coordinates is easy
        // why hot for map coordinates, since some line drawing will exceed the boundary
        // if if want to support pre-defined object, the image would also exceeds.
        void DrawImage(Fl_Image *, int, int);
        void DrawImage(Fl_Image *, int, int, int, int, int, int);
        void DrawLoop(int, int, int, int, int, int);
        void DrawLine(int, int, int, int);
        void DrawRectangle(int, int, int, int);
        void DrawCircle(int, int, int);

    private:
        void DrawSelect();
        void DrawTextBox();
        void DrawTrySelect();

    private:
        void RhombusCoverOperation  (int, int, int, std::function<void(int, int)>);
        void RectangleCoverOperation(int, int, int, std::function<void(int, int)>);
        void AttributeCoverOperation(int, int, int, std::function<void(int, int)>);

    private:
        void DrawSelectByTile();
        void DrawSelectByObjectIndex(int);
        void DrawSelectByObjectGround(bool);

    private:
        void DrawSelectBySingle();
        void DrawSelectByRhombus();
        void DrawSelectByRectangle();
        void DrawSelectByAttribute();

    private:
        void AddSelect();
        void ClearGroundSelect();

    private:
        void AddSelectBySingle();
        void AddSelectByRhombus();
        void AddSelectByRectangle();
        void AddSelectByAttribute();
        void AddSelectByObject(bool);

    private:
        bool LocateAnimation(int, int);
        bool LocateLineSegment(int &, int &, int &, int &);
        bool LocateGroundSubCell(int, int, int &, int &, int &);
        void GetTriangleOnMap(int, int, int, int &, int &, int &, int &, int &, int &);

    public:
        void SetScrollBar();

    public:
        void DrawRUC(int, int, bool);

    public:
        void DrawFloatObject(int, int, int, int, int);

    public:
        void DrawImageCover(Fl_Image *, int, int, int, int);

    public:
        Fl_Image *RetrievePNG(uint8_t, uint16_t);
        Fl_Image *CreateRectImage(int, int, uint32_t);
        Fl_Image *CreateRoundImage(int, uint32_t);

    protected:
        void DrawDoneSelectByObject(bool);
};
