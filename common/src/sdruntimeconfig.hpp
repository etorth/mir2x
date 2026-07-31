#pragma once
#include <tuple>
#include <string>
#include <cstdint>
#include <utility>
#include <optional>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <cerealf.hpp>

class SDRuntimeConfig
{
    private:
        std::unordered_map<std::string, std::string> m_config;

    public:
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(m_config);
        }

    public:
        template<int> friend struct SDRuntimeConfigAccessor;

    public:
        // empty s means "reset to default": erase the key if present.
        // cerealf::serialize never returns an empty string, so empty s is an unambiguous sentinel.
        bool setConfig(std::string key, std::string s)
        {
            if(s.empty()){
                if(auto p = m_config.find(key); p != m_config.end()){
                    m_config.erase(p);
                    return true;
                }
                return false;
            }

            if(auto p = m_config.find(key); p == m_config.end()){
                m_config.emplace(std::move(key), std::move(s));
                return true;
            }
            else if(p->second != s){
                p->second = std::move(s);
                return true;
            }
            return false;
        }

    public:
        std::optional<std::string> getConfig(const std::string &key) const
        {
            if(auto p = m_config.find(key); p != m_config.end()){
                return p->second;
            }
            return std::nullopt;
        }
};

constexpr int RTCFG_NONE  = 0;
constexpr int RTCFG_BEGIN = 1;

template<int> struct SDRuntimeConfigAccessor;
constexpr int _RSVD_rtcfg_add_type_counter_begin = __COUNTER__;

// KEYS must be a parenthesized list of extra key types, possibly empty:
//   ()               : no extra keys       -> KeyTuple = std::tuple<int>
//   (uint32_t)       : one extra uint32_t  -> KeyTuple = std::tuple<int, uint32_t>
//   (uint32_t, int)  : two extras          -> KeyTuple = std::tuple<int, uint32_t, int>
//
// each accessor gets a unique constexpr int NAME (RTCFG_XXX) via __COUNTER__.
// map key = cerealf::serialize(KeyTuple(NAME, extraKeys...))
// two accessors always differ in the first tuple element (NAME), so serialized keys are always distinct across accessors.

#define _RTCFG_UNPAREN(x)         _RTCFG_UNPAREN_IMPL x
#define _RTCFG_UNPAREN_IMPL(...)  __VA_ARGS__

#define _MACRO_ADD_RTCFG_TYPE(NAME, KEYS, VALUE_TYPE, DEF_VAL) \
    constexpr int NAME = __COUNTER__ - _RSVD_rtcfg_add_type_counter_begin; \
    template<> struct SDRuntimeConfigAccessor<NAME> \
    { \
        using ExtraKeyTuple = std::tuple<_RTCFG_UNPAREN(KEYS)>; \
        using KeyTuple      = decltype(std::tuple_cat(std::declval<std::tuple<int>>(), std::declval<ExtraKeyTuple>())); \
        using ValueType     = VALUE_TYPE; \
        \
        static constexpr int index = NAME; \
        \
        static ValueType defValue() \
        { \
            return ValueType(DEF_VAL); \
        } \
        \
        template<typename... Args> static std::string keyString(Args && ... args) \
        { \
            static_assert(sizeof...(Args) == std::tuple_size_v<ExtraKeyTuple>, "wrong number of extra key args"); \
            return cerealf::serialize(KeyTuple(index, std::forward<Args>(args)...), 0); \
        } \
        \
        template<typename... Args> static ValueType get(const SDRuntimeConfig &rtCfg, Args && ... args) \
        { \
            if(const auto opt = rtCfg.getConfig(keyString(std::forward<Args>(args)...))){ \
                return cerealf::deserialize<ValueType>(*opt); \
            } \
            return defValue(); \
        } \
        \
        template<typename... Args> static bool set(SDRuntimeConfig &rtCfg, Args && ... args) \
        { \
            constexpr size_t N = sizeof...(Args); \
            static_assert(N == std::tuple_size_v<ExtraKeyTuple> + 1, "expected extra keys + value"); \
            \
            auto argRefs = std::forward_as_tuple(std::forward<Args>(args)...); \
            std::string key = [&]<size_t... Is>(std::index_sequence<Is...>){ \
                return keyString(std::get<Is>(std::move(argRefs))...); \
            }(std::make_index_sequence<N - 1>{}); \
            \
            ValueType value(std::get<N - 1>(std::move(argRefs))); \
            if(defValue() == value){ \
                return rtCfg.setConfig(std::move(key), {}); \
            } \
            return rtCfg.setConfig(std::move(key), cerealf::serialize<ValueType>(value)); \
        } \
    };

    /**/ // begin of runtime config types
    /**/ // each _MACRO_ADD_RTCFG_TYPE(NAME, (extraKeyTypes...), valueType, defValue) generates:
    /**/ //     constexpr int NAME;             // unique key index
    /**/ //     struct SDRuntimeConfigAccessor<NAME> { ... };
    /**/ //
    /**/ // don't put any other code except the macro defines and type aligns

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_BGM,      (), bool,  true )
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_BGMVALUE, (), float, 0.50f)

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SEFF,      (), bool,  true )
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SEFFVALUE, (), float, 0.50f)

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_FULLSCREEN, (), bool, false)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SHOWFPS   , (), bool, false)

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_IME       , (), int, static_cast<int>(IME_DISABLE))
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_ATTACKMODE, (), int, 0)

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许私聊        , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许白字聊天    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许地图聊天    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许行会聊天    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许全服聊天    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许加入队伍    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许加入行会    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许回生术      , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许天地合一    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许交易        , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许添加好友    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许行会召唤    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许行会杀人提示, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许拜师        , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许好友上线提示, (), bool, true)
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_强制攻击    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示体力变化, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_满血不显血  , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示血条    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_数字显血    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_综合数字显示, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_标记攻击目标, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_单击解除锁定, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示BUFF图标, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示BUFF计时, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示角色名字, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_关闭组队血条, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_队友染色    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示队友位置, (), bool, true)
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续盾      , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续移花接木, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续金刚    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续破血    , (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续铁布衫  , (), bool, true)
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_自动喝红, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_保持满血, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_自动喝蓝, (), bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_保持满蓝, (), bool, true)
    /**/
    /**/ using _RSVD_helper_type_RTCFG_WINDOWSIZE_t = std::pair<int, int>;
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_WINDOWSIZE, (), _RSVD_helper_type_RTCFG_WINDOWSIZE_t, std::make_pair(800, 600))
    /**/
    /**/ // 0: accept any friend request
    /**/ // 1: reject any friend request
    /**/ // 2: accept or reject manually
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_好友申请, (), int, 2)
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_DROPITEMRULE, (uint32_t), uint32_t, 0u)
    /**/
    /**/ // end of runtime config types
    /**/ // any config types should be put inside above region

#undef _MACRO_ADD_RTCFG_TYPE
#undef _RTCFG_UNPAREN
#undef _RTCFG_UNPAREN_IMPL
constexpr int RTCFG_END = __COUNTER__ - _RSVD_rtcfg_add_type_counter_begin;

template<int INDEX, typename... Args> inline auto SDRuntimeConfig_getConfig(const SDRuntimeConfig &rtCfg, Args && ... args)
{
    return SDRuntimeConfigAccessor<INDEX>::get(rtCfg, std::forward<Args>(args)...);
}

template<int INDEX, typename... Args> inline bool SDRuntimeConfig_setConfig(SDRuntimeConfig &rtCfg, Args && ... args)
{
    return SDRuntimeConfigAccessor<INDEX>::set(rtCfg, std::forward<Args>(args)...);
}
