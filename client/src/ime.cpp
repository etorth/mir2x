#include <vector>
#include <atomic>
#include <thread>
#include <cstdint>
#include <optional>
#include <condition_variable>

#include "pinyin.h"
#include "ime.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

// for better implementation, check
// https://github.com/libpinyin/ibus-libpinyin.git

struct _IME_Instance final
{
    std::string input;
    std::string prefix;

    std::optional<size_t> selection;
    std::vector<std::string> candidateList;
    std::vector<std::pair<std::string, int>> stk; // (sentence, start)

    bool done;
    std::thread th;

    mutable std::mutex mtx;
    mutable std::condition_variable cond;

    pinyin_context_t *context;
    pinyin_instance_t *instance;

    _IME_Instance()
    {
        context = pinyin_init("libpinyin/data", "libpinyin/conf");
        fflassert(context);

        pinyin_set_options(context,
                (guint32)PINYIN_INCOMPLETE  |
                (guint32)PINYIN_CORRECT_ALL |
                (guint32)USE_DIVIDED_TABLE  |
                (guint32)USE_RESPLIT_TABLE  |
                (guint32)DYNAMIC_ADJUST);

        instance = pinyin_alloc_instance(context);
        fflassert(instance);

        done = false;
        th = std::thread([this]()
        {
            while(!done){
                std::unique_lock<std::mutex> lock(mtx);
                cond.wait(lock);

                if(done){
                    return;
                }

                if(input.empty()){
                    prefix.clear();
                    selection.reset();
                    stk.clear();
                    candidateList.clear();
                    pinyin_reset(instance);
                    continue;
                }

                if(selection.has_value()){
                    guint num = 0;
                    pinyin_get_n_candidate(instance, &num);

                    if(selection.value() >= num){
                        selection.reset();
                        continue;
                    }

                    const auto choice = selection.value();
                    selection.reset();

                    lookup_candidate_t *candidate = nullptr;
                    pinyin_get_candidate(instance, choice, &candidate);

                    const char *word = nullptr;
                    pinyin_get_candidate_string(instance, candidate, &word);

                    lookup_candidate_type_t type;
                    pinyin_get_candidate_type(instance, candidate, &type);

                    const auto [sentence, offset] = [type, word, this]() -> std::pair<std::string, size_t>
                    {
                        if((type == NBEST_MATCH_CANDIDATE) || stk.empty()){
                            return {word, 0};
                        }
                        else{
                            return {stk.back().first + word, to_uz(stk.back().second)};
                        }
                    }();

                    stk.emplace_back(sentence, pinyin_choose_candidate(instance, offset, candidate));
                }
                else if(stk.empty()){
                    pinyin_parse_more_full_pinyins(instance, input.c_str());
                    pinyin_guess_sentence_with_prefix(instance, prefix.c_str());
                }

                candidateList.clear();
                pinyin_guess_candidates(instance, stk.empty() ? 0 : stk.back().second, SORT_BY_PHRASE_LENGTH_AND_PINYIN_LENGTH_AND_FREQUENCY);

                guint num = 0;
                pinyin_get_n_candidate(instance, &num);

                for(guint i = 0; i < num; ++i){
                    lookup_candidate_t *candidate = nullptr;
                    pinyin_get_candidate(instance, i, &candidate);

                    const char *word = nullptr;
                    pinyin_get_candidate_string(instance, candidate, &word);

                    candidateList.emplace_back(word);
                }
            }
        });
    }

    ~_IME_Instance()
    {
        {
            const std::lock_guard<std::mutex> lock(mtx);
            done = true;
        }
        cond.notify_one();

        th.join();

        pinyin_free_instance(instance);
        pinyin_mask_out(context, 0x0, 0x0);

        pinyin_save(context);
        pinyin_fini(context);
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

void IME::clear()
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        imePtr->input.clear();
    }
    imePtr->cond.notify_one();
}

void IME::feed(char ch)
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        imePtr->input.push_back(ch);
    }
    imePtr->cond.notify_one();
}

void IME::backspace()
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        if(imePtr->input.empty()){
            return;
        }

        if(imePtr->stk.empty()){
            imePtr->input.pop_back();
        }
        else{
            imePtr->stk.pop_back();
        }
    }
    imePtr->cond.notify_one();
}

void IME::assign(std::string argPrefix, std::string argInput)
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        imePtr->stk.clear();
        imePtr->selection.reset();

        imePtr->prefix = std::move(argPrefix);
        imePtr->input  = std::move(argInput);
    }
    imePtr->cond.notify_one();
}

void IME::select(size_t index)
{
    const auto imePtr = asIMEPtr(m_instance);
    {
        const std::lock_guard<std::mutex> lock(imePtr->mtx);
        imePtr->selection = index;
    }
    imePtr->cond.notify_one();
}

bool IME::done() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    return !imePtr->stk.empty() && to_uz(imePtr->stk.back().second) >= imePtr->input.size();
}

bool IME::empty() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    return imePtr->input.empty();
}

std::string IME::input() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    return imePtr->input;
}

std::string IME::result() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    if(imePtr->stk.empty()){
        return imePtr->input;
    }
    return imePtr->stk.back().first + imePtr->input.substr(imePtr->stk.back().second);
}

std::string IME::sentence() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    if(imePtr->stk.empty()){
        return {};
    }
    return imePtr->stk.back().first;
}

std::vector<std::string> IME::candidateList() const
{
    const auto imePtr = asIMEPtr(m_instance);
    const std::lock_guard<std::mutex> lock(imePtr->mtx);
    return imePtr->candidateList;
}
