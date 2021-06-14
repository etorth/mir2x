/*
 * =====================================================================================
 *
 *       Filename: layerviewarea.hpp
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
#include <functional>
#include "basearea.hpp"
#include "mir2xmapdata.hpp"

class LayerViewArea: public BaseArea
{
    private:
        int m_mouseX = 0;
        int m_mouseY = 0;

    public:
        LayerViewArea(int argX, int argY, int argW, int argH)
            : BaseArea(argX, argY, argW, argH)
        {}

    public:
        void draw();
        int  handle(int);

    public:
        std::tuple<int, int> offset() const;

    private:
        void drawGrid();
        void drawTile();
        void drawLight();
        void drawGround();
        void drawObject(int);
        void drawAttributeGrid();

    protected:
        const Mir2xMapData *getLayer() const;

    public:
        std::optional<std::tuple<size_t, size_t>> getROISize() const override;
};
