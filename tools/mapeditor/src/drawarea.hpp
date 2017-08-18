/*
 * =====================================================================================
 *
 *       Filename: drawarea.hpp
 *        Created: 07/26/2015 04:27:57 AM
 *  Last Modified: 08/17/2017 17:48:34
 *
 *    Description: To handle or GUI interaction
 *                 Provide handlers to EditorMap
 *                 EditorMap will draw scene with assistance of ImageDB
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
#include <FL/Fl_Box.H>
#include <FL/Fl_Shared_Image.H>

class DrawArea: public Fl_Box
{
    private:
        int m_MouseX;
        int m_MouseY;

        int m_OffsetX; // location of (0, 0) on DrawArea
        int m_OffsetY;

    private:
        Fl_Image *m_RUC[2];     // rectangle unit cover
        Fl_Image *m_TextBoxBG;  // backgound for living text
        Fl_Image *m_LightRUC;

    public:
        DrawArea(int, int, int, int);
       ~DrawArea();

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
        // draw functions with margin cut-off, using *DrawArea* coordinates
        // 1. not window coordinates
        // 2. not map coordinates
        //
        // why not for window coordinates is easy
        // why hot for map coordinates, since some line drawing will exceed the boundary
        // if if want to support pre-defined object, the image would also exceeds.
        void DrawImage(Fl_Image *, int, int);
        void DrawLoop(int, int, int, int, int, int);
        void DrawLine(int, int, int, int);
        void DrawRectangle(int, int, int, int);

    private:
        void DrawSelect();
        void DrawTextBox();
        void DrawTrySelect();

    private:
        void RhombusCoverOperation  (int, int, int, std::function<void(int, int)>);
        void RectangleCoverOperation(int, int, int, std::function<void(int, int)>);
        void AttributeCoverOperation(int, int, int, std::function<void(int, int)>);

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
        Fl_Image *RetrievePNG(uint8_t, uint16_t);
        Fl_Image *CreateRectImage(int, int, uint32_t);
};
