#pragma once
#include "widget.hpp"
#include "menuboard.hpp"
#include "menubutton.hpp"
#include "imageboard.hpp"
#include "itemflex.hpp"
#include "labelboard.hpp"
#include "trigfxbutton.hpp"
#include "gfxcropboard.hpp"
#include "texinputbackground.hpp"

class PullMenu: public Widget
{
    private:
        struct LabelCropArgs final
        {
            const char8_t *text = nullptr;

            Widget::VarSizeOpt w = std::nullopt;
            Widget::VarSizeOpt h = std::nullopt;
        };

        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            PullMenu::LabelCropArgs label {};
            PullMenu::LabelCropArgs title {};

            std::vector<MenuBoard::AddItemArgs> itemList {};
            MenuItem::ClickCBFunc onClick = nullptr;

            Widget::WADPair parent {};
        };

    private:
        LabelBoard   m_tips;
        GfxCropBoard m_tipsCrop;

    private:
        LabelBoard         m_title; // crop by MenuButton
        TexInputBackground m_titleBg;

    private:
        ImageBoard m_imgOff;
        ImageBoard m_imgOn;
        ImageBoard m_imgDown;

    private:
        TrigfxButton m_button;

    private:
        ItemFlex m_flex; // hold label/bg/button

    private:
        MenuBoard  m_menuBoard;
        MenuButton m_menuButton;

    public:
        PullMenu(PullMenu::InitArgs);

    public:
        auto getTips (this auto && self) { return std::addressof(self.m_tips ); }
        auto getTitle(this auto && self) { return std::addressof(self.m_title); }

    public:
        void setFocus(bool argFocus) override
        {
            if(!argFocus){
                m_menuBoard.setShow(false);
            }
            return Widget::setFocus(argFocus);
        }
};
