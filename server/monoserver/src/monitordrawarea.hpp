/*
 * =====================================================================================
 *
 *       Filename: monitordrawarea.hpp
 *        Created: 04/14/2016 04:27:57 AM
 *  Last Modified: 04/14/2016 18:38:48
 *
 *    Description: To handle GUI interaction for server, to monitor objects
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
#include <FL/Fl_Box.H>
#include <cstdint>
#include <FL/Fl_Shared_Image.H>
#include <functional>

#include "sysconst.hpp"

enum RCoverColorType: uint8_t{
    COLOR_MONSTER,
    COLOR_PLAYER,
    COLOR_NPC,
    COLOR_MAX,
};

class DrawArea: public Fl_Box
{
    private:
        int m_MouseX;
        int m_MouseY;

        int m_OffsetX; // location of (0, 0) on DrawArea
        int m_OffsetY;

    private:
        Fl_Image *m_RC[SYS_MAXR + 1][];

    private:
        Fl_Image *m_TUC[2][4]; // triangle unit cover
        Fl_Image *m_TextBoxBG;     // backgound for living text
        Fl_Image *m_LightUC;

    public:
        DrawArea(int, int, int, int);
        ~DrawArea();

    public:
        // required overriding function
        void draw();
        int  handle(int);

    public:
        void SetOffset(int, bool, int, bool);


    private:
        void DrawAttributeGrid();
        void DrawGrid();
        void DrawTile();
        void DrawLight();
        void DrawObject(bool);
        void DrawGround();

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
        void DrawTrySelect();
        void DrawTextBox();

    private:
        void RhombusCoverOperation(int, int, int, std::function<void(int, int, int)>);
        void RectangleCoverOperation(int, int, int, std::function<void(int, int, int)>);
        void AttributeCoverOperation(int, int, int, std::function<void(int, int, int)>);

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
        void GetTriangleOnMap(int, int, int, int &, int &, int &, int &, int &, int &);
        bool LocateLineSegment(int &, int &, int &, int &);
        bool LocateGroundSubCell(int, int, int &, int &, int &);

    public:
        void SetScrollBar();

    public:
        void DrawTUC(int, int, int, bool);

    public:
        // helper function
        Fl_Image *CreateTUC(int, bool);
        Fl_Image *RetrievePNG(uint8_t, uint16_t);
};
