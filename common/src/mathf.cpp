#include <random>
#include <chrono>
#include "mathf.hpp"

static std::mutex g_randMutex;
static std::mt19937_64 g_randGenerator(std::random_device{}());
static std::string g_defaultTokens = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

uint64_t mathf::rand()
{
    const std::lock_guard<std::mutex> lock(g_randMutex);
    return g_randGenerator();
}

std::string mathf::randstr(size_t size, const std::string &tok)
{
    std::string s;
    s.reserve(size);

    const auto &tokens = tok.empty() ? g_defaultTokens : tok;
    for(size_t i = 0; i < size; ++i){
        s.push_back(tokens[mathf::rand<size_t>(0, tokens.size() - 1)]);
    }
    return s;
}
