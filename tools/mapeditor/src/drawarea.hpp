#pragma once
#include <FL/Fl_Box.H>
#include <cstdint>
#include <FL/Fl_Shared_Image.H>
// #include <FL/Fl_Gl_Window.H>

class DrawArea: public Fl_Box
// class DrawArea: public Fl_Gl_Window
{
    private:
        int     m_MouseX;
        int     m_MouseY;
    private:
        int     m_OffsetX; // location of (0, 0) on DrawArea
        int     m_OffsetY;
    public:
        DrawArea(int, int, int, int);
        ~DrawArea();
    public:
        void draw();
        int  handle(int);
    public:
        void    SetXOffset(int);
        void    SetYOffset(int);
        void    DrawBaseTile();
        void    DrawGroundObject();
        void    DrawOverGroundObject();
        void    DrawTriangleUnderCover();
        void    DrawFunction(Fl_Shared_Image *, int, int);
        void    DrawFunction(uint32_t, uint32_t, int, int);
        void    DrawGroundInfo();
        void    DrawCover();
        void    DrawSelect();
        void    DrawTextBox();

    public:
        bool LocateGroundSubCell(int, int, int &, int &, int &);
        void SetGroundSubCellUnderPoint(int, int);

    public:
        void DrawTriangleUnit(int, int, int);

    public:
        void GetTriangleOnMap(int, int, int, int &, int &, int &, int &, int &, int &);
};
