#include "ime.hpp"
#include "pinyin.h"
#include "fflerror.hpp"

struct _IME_Instance
{
    pinyin_context_t *context;
    pinyin_instance_t *instance;

    _IME_Instance()
    {
        context = pinyin_init("libpinyin/data", "libpinyin/data");
        fflassert(context);

        pinyin_set_options(context, (guint32)PINYIN_INCOMPLETE | (guint32)PINYIN_CORRECT_ALL | (guint32)USE_DIVIDED_TABLE | (guint32)USE_RESPLIT_TABLE | (guint32)DYNAMIC_ADJUST);
        instance = pinyin_alloc_instance(context);
        fflassert(instance);
    }
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
