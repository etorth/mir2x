/*
 * =====================================================================================
 *
 *       Filename: modalstringboard.hpp
 *        Created: 07/18/2021 23:06:52
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
#include <mutex>
#include <functional>
#include <condition_variable>
#include "widget.hpp"
#include "inputline.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"

class ModalStringBoard: public Widget
{
    private:
        mutable std::mutex m_lock;
        mutable std::condition_variable m_cond;

    private:
        ImageBoard m_image;

    private:
        bool m_done = false;
        std::u8string m_xmlString;

    public:
        ModalStringBoard();

    public:
        void loadXML(std::u8string s)
        {
            {
                std::lock_guard<std::mutex> lockGuard(m_lock);
                m_xmlString = std::move(s);
            }
            m_cond.notify_one();
        }

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        void waitDone();

    public:
        void setDone()
        {
            {
                std::lock_guard<std::mutex> lockGuard(m_lock);
                m_done = true;
            }
            m_cond.notify_one();
        }
};
