#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include "widget.hpp"
#include "buttonbase.hpp"
#include "labelboard.hpp"

class ProcessRun;
class ACButton: public ButtonBase
{
    private:
        ProcessRun *m_proc;
        const std::unordered_map<std::string, uint32_t> m_texMap;

    private:
        size_t m_currButtonName;
        const std::vector<std::string> m_buttonNameList;

    private:
        LabelBoard m_labelBoard;

    public:
        ACButton(dir8_t, int, int, ProcessRun *, const std::vector<std::string> &, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    private:
        void setLabel();
};
