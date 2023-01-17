#include "ime.hpp"
#include "pinyin.h"

struct _IME_Instance
{

};

static _IME_Instance * asIMEPtr(void *ptr)
{
    return reinterpret_cast<_IME_Instance *>(ptr);
}

IME::IME()
    : m_instance(new _IME_Instance())
{}

IME::~IME()
{
    delete asIMEPtr(m_instance);
}
