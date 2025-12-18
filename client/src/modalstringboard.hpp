#pragma once
#include <memory>
#include <string>

class Widget;
class ModalStringBoard
{
    private:
        std::u8string m_xmlString;

    private:
        std::unique_ptr<Widget> m_boardImpl;

    public:
        ModalStringBoard();

    public:
        void loadXML(std::u8string);

    public:
        void drawScreen(bool) const;
};
