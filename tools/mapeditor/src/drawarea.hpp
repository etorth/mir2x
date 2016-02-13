#pragma once
#include <vector>
#include <FL/Fl_Box.H>
#include <cstdint>
#include <FL/Fl_Shared_Image.H>

class DrawArea: public Fl_Box
{
    private:
        int m_MouseX;
        int m_MouseY;

        int m_OffsetX; // location of (0, 0) on DrawArea
        int m_OffsetY;

    private:
        Fl_Image *m_TriangleUC[4]; // triangle unit cover
        Fl_Image *m_TextBoxBG;     // backgound for living text

    public:
        DrawArea(int, int, int, int);
        ~DrawArea();

    public:
        // required overriding function
        void draw();
        int  handle(int);

    public:
        void    SetXOffset(int);
        void    SetYOffset(int);

    private:
        void    DrawBaseTile();
        void    DrawGroundObject();
        void    DrawOverGroundObject();
        void    DrawFunction(Fl_Image *, int, int);
        void    DrawFunction(uint32_t, uint32_t, int, int);
        void    DrawGroundInfo();

    private:
        void    DrawSelect();
        void    DrawTextBox();

    private:
        void DrawSelectBySingle();
        void DrawSelectByRegion();
        void DrawSelectByRhombus();
        void DrawSelectByRectangle();

    private:
        void GetTriangleOnMap(int, int, int, int &, int &, int &, int &, int &, int &);
        bool LocateGroundSubCell(int, int, int &, int &, int &);
        void SetGroundSubCellUnderPoint(int, int);

    public:
        Fl_Image *CreateTriangleUC(int);

    public:
        void DrawTriangleUC(int, int, int);
};
