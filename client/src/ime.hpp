#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <utility>

class IME
{
    private:
        void *m_instance;

    public:
        IME();

    public:
        virtual ~IME();

    public:
        void clear();

    public:
        void feed(char);
        void backspace();
        void assign(std::string, std::string);

    public:
        void select(size_t);

    public:
        std::string input() const;
        std::string result() const;
        std::string sentence() const;
        std::vector<std::string> candidateList() const;
};
