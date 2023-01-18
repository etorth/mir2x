#include <atomic>
#include <thread>
#include <condition_variable>

#include "ime.hpp"
#include "pinyin.h"
#include "fflerror.hpp"

struct _IME_Instance final
{
    std::string input;
    std::string prefix;
    std::string search;

    std::vector<std::string> candidateList;

    std::thread th;
    std::atomic<bool> done;

    mutable std::mutex mtx;
    mutable std::condition_variable cond;

    pinyin_context_t *context;
    pinyin_instance_t *instance;

    _IME_Instance()
        : done {false}
    {
        context = pinyin_init("libpinyin/data", "libpinyin/data");
        fflassert(context);

        pinyin_set_options(context,
                (guint32)PINYIN_INCOMPLETE  |
                (guint32)PINYIN_CORRECT_ALL |
                (guint32)USE_DIVIDED_TABLE  |
                (guint32)USE_RESPLIT_TABLE  |
                (guint32)DYNAMIC_ADJUST);

        instance = pinyin_alloc_instance(context);
        fflassert(instance);

        th = std::thread([this]()
        {
            while(!done){
                std::unique_lock<std::mutex> lock(mtx);
                cond.wait(lock, [this]() -> bool
                {
                    return done;
                });

                std::swap(input, search);
                if(search.empty()){
                    continue;
                }

                pinyin_guess_sentence(instance);
            }
        });
    }

    ~_IME_Instance()
    {
        done = true;
        cond.notify_one();

        th.join();

        pinyin_free_instance(instance);
        pinyin_mask_out(context, 0x0, 0x0);

        pinyin_save(context);
        pinyin_fini(context);
    }


    void feed(char ch)
    {
        {
            const std::lock_guard<std::mutex> lock(mtx);
            input.push_back(ch);
        }
        cond.notify_one();
    }

    void clear()
    {
        {
            const std::lock_guard<std::mutex> lock(mtx);
            input.clear();
            candidateList.clear();
        }
        cond.notify_one();
    }

    std::vector<std::string> getCandidateList() const
    {
        const std::lock_guard<std::mutex> lock(mtx);
        return candidateList;
    }

    std::string getInputString() const
    {
        const std::lock_guard<std::mutex> lock(mtx);
        return input;
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
