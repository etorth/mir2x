#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "itempair.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "trigfxbutton.hpp"

class ProcessRun;
class ACButton: public TrigfxButton
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            ProcessRun *proc = nullptr;
            std::vector<std::string> names {};

            Widget::WADPair parent {};
        };

    private:
        ProcessRun *m_proc;
        const std::unordered_map<std::string, uint32_t> m_texMap;

    private:
        size_t m_buttonIndex = 0;
        const std::vector<std::string> m_buttonNameList;

    private:
        ImageBoard m_img;
        TextBoard  m_text;
        ItemPair   m_gfxCanvas;

    public:
        ACButton(ACButton::InitArgs);

    private:
        const std::string &buttonName() const
        {
            return m_buttonNameList.at(m_buttonIndex);
        }
};
