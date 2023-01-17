#pragma once
#include <vector>
#include <string>

class IME
{
    private:
        void *m_instance;

    public:
        IME();

    public:
        virtual ~IME();

    public:
        std::vector<std::string> getCandidateStringList() const;
};
