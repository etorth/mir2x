/*
 * =====================================================================================
 *
 *       Filename: texaniboard.hpp
 *        Created: 07/20/2017 00:34:13
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *
 * =====================================================================================
 */

#pragma once
#include <vector>
#include "widget.hpp"

class TexAniBoard: public Widget
{
    private:
        size_t m_fps;
        double m_accuTime;

    private:
        bool m_loop;
        bool m_fadeInout;

    private:
        std::vector<uint32_t> m_texSeq;

    public:
        TexAniBoard(int, int, uint32_t, size_t, size_t, bool, bool loop = true, Widget * pwidget = nullptr, bool autoDelete = false);

    public:
        void update(double) override;
        void drawEx(int, int, int, int, int, int) override;

    private:
        std::tuple<int, uint8_t> getDrawFrame() const;
};
