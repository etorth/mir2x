/*
 * =====================================================================================
 *
 *       Filename: drawarea.hpp
 *        Created: 07/26/2017 04:27:57
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
#include <vector>
#include <cstdint>
#include <functional>
#include "basearea.hpp"

class DrawArea: public BaseArea
{
    private:
        enum FloatObjectType: int
        {
            FOTYPE_NONE = 0,
            FOTYPE_TILE,
            FOTYPE_OBJ0,
            FOTYPE_OBJ1,
            FOTYPE_MAX,
        };

    private:
        int m_mouseX;
        int m_mouseY;

    private:
        int m_offsetX;
        int m_offsetY;

    private:
        std::shared_ptr<Fl_Image> m_lightImge;

    public:
        DrawArea(int, int, int, int);

    public:
        ~DrawArea() = default;

    public:
        void draw();
        int  handle(int);

    public:
        void SetOffset(int, bool, int, bool);

    public:
        int OffsetX() const
        {
            return m_offsetX;
        }

        int OffsetY() const
        {
            return m_offsetY;
        }

    private:
        void DrawGrid();
        void DrawTile();
        void DrawLight();
        void DrawGround();
        void DrawObject(bool);
        void DrawAttributeGrid();

    private:
        void DrawTextBox();
        void DrawTrySelect();
        void DrawDoneSelect();

    private:
        void RhombusCoverOperation  (int, int, int, std::function<void(int, int)>);
        void RectangleCoverOperation(int, int, int, std::function<void(int, int)>);
        void AttributeCoverOperation(int, int, int, std::function<void(int, int)>);

    private:
        void DrawTrySelectByTile();
        void DrawSelectByObjectIndex(int);
        void DrawSelectByObjectGround(bool);

    private:
        void DrawTrySelectBySingle();
        void DrawTrySelectByRhombus();
        void DrawTrySelectByRectangle();
        void DrawTrySelectByAttribute();

    private:
        void AddSelect();
        void ClearGroundSelect();

    private:
        void AddSelectByTile();
        void AddSelectBySingle();
        void AddSelectByRhombus();
        void AddSelectByRectangle();
        void AddSelectByAttribute();
        void AddSelectByObject(bool);

    private:
        bool LocateAnimation(int, int);

    public:
        void SetScrollBar();

    public:
        void DrawFloatObject(int, int, int, int, int);

    public:
        Fl_Image *RetrievePNG(uint8_t, uint16_t);
        Fl_Image *CreateRoundImage(int, uint32_t);

    protected:
        void DrawDoneSelectByTile();
        void DrawDoneSelectByObject(bool);
        void DrawDoneSelectByAttribute();

    protected:
        void FillMapGrid(int, int, int, int, uint32_t);
};
