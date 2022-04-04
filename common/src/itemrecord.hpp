#pragma once
#include <string>
#include <cstdint>
#include <optional>
#include <string_view>
#include "protocoldef.hpp"

struct ItemRecord
{
    const char8_t * const name   = nullptr;
    const char8_t * const type   = nullptr;
    const char8_t * const rarity = nullptr;

    const int weight = 0;
    const int pkgGfxID = 0;
    const uint32_t shape = 0;

    const struct EquipAttribute
    {
        const int duration = 0;

        const int dc[2] = {0, 0};
        const int mc[2] = {0, 0};
        const int sc[2] = {0, 0};

        const int  ac[2] = {0, 0};
        const int mac[2] = {0, 0};

        const int dcHit = 0;
        const int mcHit = 0;

        const int dcDodge = 0;
        const int mcDodge = 0;

        const int speed = 0;
        const int comfort = 0;
        const int luckCurse = 0;

        struct ExtHealth
        {
            const int add     = 0;
            const int steal   = 0;
            const int recover = 0;
        };

        const ExtHealth hp {};
        const ExtHealth mp {};

        struct AddElem
        {
            const int fire    = 0;
            const int ice     = 0;
            const int light   = 0;
            const int wind    = 0;
            const int holy    = 0;
            const int dark    = 0;
            const int phantom = 0;
        };

        const AddElem dcElem {};
        const AddElem acElem {};

        const struct AddLoad
        {
            const int body      = 0;
            const int weapon    = 0;
            const int inventory = 0;
        }
        load {};

        const struct EquipReq
        {
            const int dc = 0;
            const int mc = 0;
            const int sc = 0;
            const int level = 0;
            const char8_t * const job = nullptr;
        }
        req {};

        const struct WeaponAttr
        {
            const int doubleHand = 0;
            const std::u8string_view type {}; // type of weapon, used for sound effect, possible types:
                                              // 飞镖   ：袖里剑
                                              // 短剑   : 短剑
                                              // 木剑   : 木剑，桃木剑
                                              // 长剑   : 所有剑
                                              // 刀     : 所有刀
                                              // 斧     : 青铜斧，修罗，炼狱
                                              // 梃     : 裁决
                                              // 棍     : 无极棍
                                              // 法杖   ：魔杖，噬魂，天神法杖
        }
        weapon {};
    }
    equip {};

    const struct PotionAttribute
    {
        const int hp   = 0;
        const int mp   = 0;
        const int time = 0;
    }
    potion {};

    const struct DopeAttribute
    {
        const int  hp = 0;
        const int  mp = 0;

        const int dc = 0;
        const int mc = 0;
        const int sc = 0;

        const int speed = 0;
        const int time  = 0;
    }
    dope {};

    const struct PoisonAttribute
    {
        const int dose = 0;
    }
    poison {};

    const struct BookAttribute
    {
        const char8_t * job = nullptr;
        const int level = 0;
    }
    book {};

    const char8_t * const description = nullptr;

    operator bool() const
    {
        return true
            && name && !std::u8string_view(name).empty()
            && type && !std::u8string_view(type).empty();
    }

    constexpr bool packable() const
    {
        return false
            || std::u8string_view(type) == u8"恢复药水"
            || std::u8string_view(type) == u8"传送卷轴"
            || std::u8string_view(type) == u8"护身符"
            || std::u8string_view(type) == u8"药粉";
    }

    constexpr bool beltable() const
    {
        return false
            || std::u8string_view(type) == u8"恢复药水"
            || std::u8string_view(type) == u8"传送卷轴";
    }

    constexpr bool wearable(int wltype) const
    {
        switch(wltype){
            case WLG_DRESS   : return std::u8string_view(u8"衣服") == type;
            case WLG_HELMET  : return std::u8string_view(u8"头盔") == type;
            case WLG_WEAPON  : return std::u8string_view(u8"武器") == type;
            case WLG_SHOES   : return std::u8string_view(u8"鞋"  ) == type;
            case WLG_NECKLACE: return std::u8string_view(u8"项链") == type;
            case WLG_ARMRING0: return std::u8string_view(u8"手镯") == type;
            case WLG_ARMRING1: return std::u8string_view(u8"手镯") == type;
            case WLG_RING0   : return std::u8string_view(u8"戒指") == type;
            case WLG_RING1   : return std::u8string_view(u8"戒指") == type;
            case WLG_TORCH   : return std::u8string_view(u8"火把") == type;
            case WLG_CHARM   : return std::u8string_view(u8"魅力|护身符|药粉").find(type) != std::u8string_view::npos;
            default          : return false;
        }
    }

    constexpr bool isItem(const char8_t *itemName) const
    {
        return name && itemName && std::u8string_view(name) == itemName;
    }

    constexpr bool isGold() const
    {
        return std::u8string_view(type) == u8"金币";
    }

    constexpr bool isDress() const
    {
        return std::u8string_view(type) == u8"衣服";
    }

    constexpr bool isWeapon() const
    {
        return std::u8string_view(type) == u8"武器";
    }

    constexpr bool isRing() const
    {
        return std::u8string_view(type) == u8"戒指";
    }

    constexpr bool isHelmet() const
    {
        return std::u8string_view(type) == u8"头盔";
    }

    constexpr bool isBook() const
    {
        return std::u8string_view(type) == u8"技能书";
    }

    constexpr bool isPotion() const
    {
        return std::u8string_view(type) == u8"恢复药水";
    }

    constexpr bool isDope() const
    {
        return std::u8string_view(type) == u8"强效药水";
    }

    constexpr std::optional<bool> clothGender() const
    {
        if(type && (std::u8string_view(type) == u8"衣服")){
            if(name){
                if(std::u8string_view(name).find(u8"（男）") != std::u8string_view::npos){
                    return true;
                }
                else if(std::u8string_view(name).find(u8"（女）") != std::u8string_view::npos){
                    return false;
                }
            }
        }
        return {};
    }
};
