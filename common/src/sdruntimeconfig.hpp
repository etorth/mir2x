#pragma once
#include <tuple>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cerealf.hpp>

class SDRuntimeConfig
{
    private:
        std::unordered_map<int, std::string> m_config;

    public:
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(m_config);
        }

    public:
        template<int            > friend auto SDRuntimeConfig_getConfig(const SDRuntimeConfig &           );
        template<int, typename T> friend bool SDRuntimeConfig_setConfig(      SDRuntimeConfig &, const T &);

    public:
        bool setConfig(int key, const void *data, size_t length)
        {
            if(auto p = m_config.find(key); p == m_config.end()){
                m_config.emplace(key, std::string((char *)(data), length));
                return true;
            }
            else if(p->second != std::string_view((char *)(data), length)){
                p->second = std::string((char *)(data), length);
                return true;
            }
            else{
                return false;
            }
        }

        bool setConfig(int key, const std::string_view &s)
        {
            return setConfig(key, s.data(), s.size());
        }

        bool setConfig(int key, const std::string &s)
        {
            return setConfig(key, s.data(), s.size());
        }

    public:
        const std::optional<std::string> getConfig(int key) const
        {
            if(auto p = m_config.find(key); p != m_config.end()){
                return p->second;
            }
            return std::nullopt;
        }
};

constexpr int RTCFG_NONE  = 0;
constexpr int RTCFG_BEGIN = 1;

constexpr int _RSVD_rtcfg_add_type_counter_begin = __COUNTER__;
template<int            > auto SDRuntimeConfig_getConfig(const SDRuntimeConfig &           ) = delete;
template<int, typename T> bool SDRuntimeConfig_setConfig(      SDRuntimeConfig &, const T &) = delete;

#define _MACRO_ADD_RTCFG_TYPE(rtCfgKeyType, rtCfgValueType, rtCfgDefaultValue) \
    constexpr int rtCfgKeyType = __COUNTER__ - _RSVD_rtcfg_add_type_counter_begin; \
    \
    template<> inline auto SDRuntimeConfig_getConfig<rtCfgKeyType>(const SDRuntimeConfig &rtCfg) \
    { \
        if(auto p = rtCfg.m_config.find(rtCfgKeyType); p != rtCfg.m_config.end()){ \
            return cerealf::deserialize<rtCfgValueType>(p->second); \
        } \
        else{ \
            return rtCfgDefaultValue; \
        } \
    } \
    \
    template<> inline bool SDRuntimeConfig_setConfig<rtCfgKeyType, rtCfgValueType>(SDRuntimeConfig &rtCfg, const rtCfgValueType &value) \
    { \
        return rtCfg.setConfig(rtCfgKeyType, cerealf::serialize<rtCfgValueType>(value)); \
    } \

    /**/ // begin of runtime config types
    /**/ // each line provides two definitions, take RTCFG_IME as example:
    /**/ //
    /**/ //     RTCFG_IME           : an integer used as key
    /**/ //     RTCFG_IME_t         : an type contains: RTCFG_IME_t::value and RTCFG_IME_t::type
    /**/ //
    /**/ // don't put any other code except the macro defines and type aligns
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_IME, bool, true)

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_BGM, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_BGMVALUE, float, 0.50f)

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SEFF, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SEFFVALUE, float, 0.50f)

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SHOWFPS   , bool, false)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_FULLSCREEN, bool, false)

    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_ATTACKMODE, int, 0)
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许私聊        , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许白字聊天    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许地图聊天    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许行会聊天    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许全服聊天    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许加入队伍    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许加入行会    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许回生术      , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许天地合一    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许交易        , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许添加好友    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许行会召唤    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许行会杀人提示, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许拜师        , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_允许好友上线提示, bool, true)
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_强制攻击    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示体力变化, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_满血不显血  , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示血条    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_数字显血    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_综合数字显示, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_标记攻击目标, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_单击解除锁定, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示BUFF图标, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示BUFF计时, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示角色名字, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_关闭组队血条, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_队友染色    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_显示队友位置, bool, true)
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续盾      , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续移花接木, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续金刚    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续破血    , bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_持续铁布衫  , bool, true)
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_自动喝红, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_保持满血, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_自动喝蓝, bool, true)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_保持满蓝, bool, true)
    /**/
    /**/ using _RSVD_helper_type_RTCFG_WINDOWSIZE_t = std::pair<int, int>;
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_WINDOWSIZE, _RSVD_helper_type_RTCFG_WINDOWSIZE_t, std::make_pair(800, 600))
    /**/
    /**/ // end of runtime config types
    /**/ // any config types should be put inside above region

#undef _MACRO_ADD_RTCFG_TYPE
constexpr int RTCFG_END = __COUNTER__ - _RSVD_rtcfg_add_type_counter_begin;
