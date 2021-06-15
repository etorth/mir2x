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
        enum
        {
            FOBJ_TILE,
            FOBJ_OBJ0,
            FOBJ_OBJ1,
        };

    public:
        DrawArea(int argX, int argY, int argW, int argH)
            : BaseArea(argX, argY, argW, argH)
        {}

    public:
        ~DrawArea() = default;

    public:
        void draw();
        int  handle(int);

    public:
        std::tuple<int, int> offset() const override;

    private:
        void drawGrid();
        void drawTile();
        void drawLight();
        void drawGround();
        void drawObject(int);
        void drawAttributeGrid();

    private:
        void drawTextBox();
        void drawTrySelect();
        void drawDoneSelect();

    private:
        void drawTrySelectByTile();
        void drawSelectByObject(int);
        void drawSelectByObjectIndex(int);

    private:
        void drawTrySelectByObject();
        void drawTrySelectByAttribute();

    private:
        void addSelect();
        void clearSelect();

    private:
        void addSelectByObject(int);
        void addSelectByObjectIndex(int);

    private:
        bool LocateAnimation(int, int);

    public:
        void drawFloatObject(int, int, int, int, int);

    public:
        Fl_Image *CreateRoundImage(int, uint32_t);

    protected:
        void drawDoneSelectByTile();
        void drawDoneSelectByObject();
        void drawDoneSelectByAttribute();

    public:
        std::optional<std::tuple<size_t, size_t>> getROISize() const override;
};
