#pragma once
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>

class Widget;
class ModalStringBoard // thread safe text board
{
    private:
        mutable std::mutex m_lock;
        mutable std::condition_variable m_cond;

    private:
        bool m_done = false;
        std::u8string m_xmlString;

    private:
        std::unique_ptr<Widget> m_boardImpl;

    public:
        ModalStringBoard();
        ~ModalStringBoard() = default;

    public:
        void loadXML(std::u8string);

    public:
        void  setDone();
        void waitDone();
};
