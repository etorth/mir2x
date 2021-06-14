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
#include <utility>
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
        int m_mouseX = 0;
        int m_mouseY = 0;

    private:
        std::unique_ptr<Fl_Image> m_lightImge;

    public:
        DrawArea(int argX, int argY, int argW, int argH)
            : BaseArea(argX, argY, argW, argH)
            , m_lightImge(CreateRoundImage(200, 0X001286FF))
        {}

    public:
        ~DrawArea() = default;

    public:
        void draw();
        int  handle(int);

    public:
        std::tuple<int, int> offset() const override;

    private:
        void DrawGrid();
        void drawTile();
        void DrawLight();
        void DrawGround();
        void drawObject(int);
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
        void DrawSelectByObject(int);
        void DrawSelectByObjectIndex(int);

    private:
        void DrawTrySelectBySingle();
        void DrawTrySelectByRhombus();
        void DrawTrySelectByRectangle();
        void DrawTrySelectByAttribute();

    private:
        void AddSelect();
        void clearGroundSelect();

    private:
        void AddSelectByTile();
        void AddSelectBySingle();
        void AddSelectByRhombus();
        void AddSelectByRectangle();
        void AddSelectByAttribute();
        void AddSelectByObject(int);
        void AddSelectByObjectIndex(int);

    private:
        bool LocateAnimation(int, int);

    public:
        void DrawFloatObject(int, int, int, int, int);

    public:
        Fl_Image *CreateRoundImage(int, uint32_t);

    protected:
        void DrawDoneSelectByTile();
        void DrawDoneSelectByObject(int);
        void DrawDoneSelectByObjectIndex(int);
        void DrawDoneSelectByAttribute();

    protected:
        void FillMapGrid(int, int, int, int, uint32_t);

    public:
        std::optional<std::tuple<size_t, size_t>> getROISize() const override;
};
