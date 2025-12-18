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
        void drawTrySelectByObject();
        void drawTrySelectByAttribute();

    private:
        void addSelect();
        void clearSelect();

    private:
        bool LocateAnimation(int, int);

    public:
        void drawFloatObject(int, int, int, int, int);

    protected:
        void drawDoneSelectByTile();
        void drawDoneSelectByObject();
        void drawDoneSelectByAttribute();

    public:
        std::optional<std::tuple<size_t, size_t>> getROISize() const override;
};
