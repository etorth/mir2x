#pragma once
#include <cstdint>
#include <climits>
#include "maprecord.hpp"
#include "itemrecord.hpp"
#include "magicrecord.hpp"
#include "buffrecord.hpp"
#include "monsterrecord.hpp"

namespace
{
    // constexpr string hash
    // code from: http://www.cse.yorku.ca/~oz/hash.html

    // return hash code as size_t
    // don't do careless cast to uint32_t, cast with modulo can give incorrect hash table slot

    constexpr size_t u8StrViewHashHelper(const char8_t *s) noexcept
    {
        size_t h = 0;
        int c;

        for(auto p = s; (c = *p); ++p){
            h = c + (h << 6U) + (h << 16U) - h;
        }
        return h;
    }

    template<typename T, uint32_t TL> constexpr auto recordNameOffsetHelper(const T (&itemList)[TL])
    {
        static_assert(TL >= 1);
        std::array<uint32_t, 3 * TL + 127> result {};

        // 1. itemOffList is much longer than itemList
        // 2. skip itemList[0], off 0 is reserved
        // 3, skip itemList[i], if str_haschar(itemList[i].name) is false

        for(uint32_t i = 1; i < TL; ++i){
            if(itemList[i].name && itemList[i].name[0]){
                for(size_t j = 0, h = u8StrViewHashHelper(itemList[i].name); j < result.size(); ++j){
                    if(const size_t c = (h + j) % result.size(); !result[c]){
                        result[c] = i;
                        break;
                    }
                }
            }
        }
        return result;
    }
}

namespace
{
    // to support use following statement in switch/case
    //
    //      switch(nMonsterID){
    //          case DBCOM_MONSTERID("鹿"):
    //          case DBCOM_MONSTERID("鸡"):
    //          case DBCOM_MONSTERID("狼"):
    //          default:
    //      }
    //
    // this is very useful since
    // 1. don't need to think about all monster english name
    // 2. don't need to assign monster ID to each type, just automatically generate one by .inc file

    constexpr ItemRecord _inn_ItemRecordList []
    {
        #include "itemrecord.inc"
    };

    constexpr BuffActRecord _inn_BuffActRecordList []
    {
        #include "buffactrecord.inc"
    };

    constexpr BuffRecord _inn_BuffRecordList []
    {
        #include "buffrecord.inc"
    };

    constexpr AttackModifierRecord _inn_AttackModifierRecordList []
    {
        #include "attackmodifierrecord.inc"
    };

    constexpr SpellModifierRecord _inn_SpellModifierRecordList []
    {
        #include "spellmodifierrecord.inc"
    };

    constexpr MonsterRecord _inn_MonsterRecordList []
    {
        #include "monsterrecord.inc"
    };

    constexpr MagicRecord _inn_MagicRecordList []
    {
        #include "magicrecord.inc"
    };

    constexpr MapRecord _inn_MapRecordList []
    {
        #include "maprecord.inc"
    };
}

namespace
{
    constexpr auto _inn_ItemRecordOffList           = recordNameOffsetHelper(_inn_ItemRecordList          );
    constexpr auto _inn_BuffActRecordOffList        = recordNameOffsetHelper(_inn_BuffActRecordList       );
    constexpr auto _inn_BuffRecordOffList           = recordNameOffsetHelper(_inn_BuffRecordList          );
    constexpr auto _inn_AttackModifierRecordOffList = recordNameOffsetHelper(_inn_AttackModifierRecordList);
    constexpr auto _inn_SpellModifierRecordOffList  = recordNameOffsetHelper(_inn_SpellModifierRecordList );
    constexpr auto _inn_MonsterRecordOffList        = recordNameOffsetHelper(_inn_MonsterRecordList       );
    constexpr auto _inn_MagicRecordOffList          = recordNameOffsetHelper(_inn_MagicRecordList         );
    constexpr auto _inn_MapRecordOffList            = recordNameOffsetHelper(_inn_MapRecordList           );
}

namespace
{
    template<typename T, uint32_t TL, size_t HL> constexpr uint32_t DBCOM_IDHELPER(const T (&itemList)[TL], const std::array<uint32_t, HL> &itemOffList, const char8_t *name)
    {
        static_assert(TL > 0);
        static_assert(HL > 0);

        if(name && name[0]){
            const size_t h = u8StrViewHashHelper(name);
            for(size_t j = 0; j < HL; ++j){
                const size_t c = (h + j) % HL;
                const uint32_t off = itemOffList[c];

                if(!off){
                    return 0;
                }

                if(itemList[off].name && (std::u8string_view(itemList[off].name) == name)){
                    return off;
                }

                // current slot has non-zero off
                // but doesn't point to the item with given name, try next slot
            }
        }
        return 0;
    }
}

constexpr uint32_t DBCOM_ITEMID          (const char8_t *name) { return DBCOM_IDHELPER(_inn_ItemRecordList,           _inn_ItemRecordOffList,           name); }
constexpr uint32_t DBCOM_MONSTERID       (const char8_t *name) { return DBCOM_IDHELPER(_inn_MonsterRecordList,        _inn_MonsterRecordOffList,        name); }
constexpr uint32_t DBCOM_MAGICID         (const char8_t *name) { return DBCOM_IDHELPER(_inn_MagicRecordList,          _inn_MagicRecordOffList,          name); }
constexpr uint32_t DBCOM_MAPID           (const char8_t *name) { return DBCOM_IDHELPER(_inn_MapRecordList,            _inn_MapRecordOffList,            name); }
constexpr uint32_t DBCOM_BUFFID          (const char8_t *name) { return DBCOM_IDHELPER(_inn_BuffRecordList,           _inn_BuffRecordOffList,           name); }
constexpr uint32_t DBCOM_BUFFACTID       (const char8_t *name) { return DBCOM_IDHELPER(_inn_BuffActRecordList,        _inn_BuffActRecordOffList,        name); }
constexpr uint32_t DBCOM_ATTACKMODIFIERID(const char8_t *name) { return DBCOM_IDHELPER(_inn_AttackModifierRecordList, _inn_AttackModifierRecordOffList, name); }
constexpr uint32_t DBCOM_SPELLMODIFIERID (const char8_t *name) { return DBCOM_IDHELPER(_inn_SpellModifierRecordList,  _inn_SpellModifierRecordOffList,  name); }

constexpr uint32_t DBCOM_ITEMENDID          () { return std::extent_v<decltype(_inn_ItemRecordList          )>; }
constexpr uint32_t DBCOM_MONSTERENDID       () { return std::extent_v<decltype(_inn_MonsterRecordList       )>; }
constexpr uint32_t DBCOM_MAGICENDID         () { return std::extent_v<decltype(_inn_MagicRecordList         )>; }
constexpr uint32_t DBCOM_MAPENDID           () { return std::extent_v<decltype(_inn_MapRecordList           )>; }
constexpr uint32_t DBCOM_BUFFENDID          () { return std::extent_v<decltype(_inn_BuffRecordList          )>; }
constexpr uint32_t DBCOM_BUFFACTENDID       () { return std::extent_v<decltype(_inn_BuffActRecordList       )>; }
constexpr uint32_t DBCOM_ATTACKMODIFIERENDID() { return std::extent_v<decltype(_inn_AttackModifierRecordList)>; }
constexpr uint32_t DBCOM_SPELLMODIFIERENDID () { return std::extent_v<decltype(_inn_SpellModifierRecordList )>; }

namespace
{
    template<typename T, uint32_t TL> constexpr const T &DBCOM_REFHELPER(const T (&itemList)[TL], uint32_t id)
    {
        return (id < TL) ? itemList[id] : itemList[0];
    }
}

constexpr const ItemRecord           &DBCOM_ITEMRECORD          (uint32_t id) { return DBCOM_REFHELPER(_inn_ItemRecordList,           id); }
constexpr const MagicRecord          &DBCOM_MAGICRECORD         (uint32_t id) { return DBCOM_REFHELPER(_inn_MagicRecordList,          id); }
constexpr const MonsterRecord        &DBCOM_MONSTERRECORD       (uint32_t id) { return DBCOM_REFHELPER(_inn_MonsterRecordList,        id); }
constexpr const MapRecord            &DBCOM_MAPRECORD           (uint32_t id) { return DBCOM_REFHELPER(_inn_MapRecordList,            id); }
constexpr const BuffRecord           &DBCOM_BUFFRECORD          (uint32_t id) { return DBCOM_REFHELPER(_inn_BuffRecordList,           id); }
constexpr const BuffActRecord        &DBCOM_BUFFACTRECORD       (uint32_t id) { return DBCOM_REFHELPER(_inn_BuffActRecordList,        id); }
constexpr const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(uint32_t id) { return DBCOM_REFHELPER(_inn_AttackModifierRecordList, id); }
constexpr const SpellModifierRecord  &DBCOM_SPELLMODIFIERRECORD (uint32_t id) { return DBCOM_REFHELPER(_inn_SpellModifierRecordList,  id); }

constexpr const ItemRecord           &DBCOM_ITEMRECORD          (const char8_t *name) { return DBCOM_ITEMRECORD          (DBCOM_ITEMID          (name)); }
constexpr const MagicRecord          &DBCOM_MAGICRECORD         (const char8_t *name) { return DBCOM_MAGICRECORD         (DBCOM_MAGICID         (name)); }
constexpr const MonsterRecord        &DBCOM_MONSTERRECORD       (const char8_t *name) { return DBCOM_MONSTERRECORD       (DBCOM_MONSTERID       (name)); }
constexpr const MapRecord            &DBCOM_MAPRECORD           (const char8_t *name) { return DBCOM_MAPRECORD           (DBCOM_MAPID           (name)); }
constexpr const BuffRecord           &DBCOM_BUFFRECORD          (const char8_t *name) { return DBCOM_BUFFRECORD          (DBCOM_BUFFID          (name)); }
constexpr const BuffActRecord        &DBCOM_BUFFACTRECORD       (const char8_t *name) { return DBCOM_BUFFACTRECORD       (DBCOM_BUFFACTID       (name)); }
constexpr const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(const char8_t *name) { return DBCOM_ATTACKMODIFIERRECORD(DBCOM_ATTACKMODIFIERID(name)); }
constexpr const SpellModifierRecord  &DBCOM_SPELLMODIFIERRECORD (const char8_t *name) { return DBCOM_SPELLMODIFIERRECORD (DBCOM_SPELLMODIFIERID (name)); }

constexpr const ItemRecord           &DBCOM_ITEMRECORD          (const std::u8string_view &name) { return DBCOM_ITEMRECORD          (DBCOM_ITEMID          (name.data())); }
constexpr const MagicRecord          &DBCOM_MAGICRECORD         (const std::u8string_view &name) { return DBCOM_MAGICRECORD         (DBCOM_MAGICID         (name.data())); }
constexpr const MonsterRecord        &DBCOM_MONSTERRECORD       (const std::u8string_view &name) { return DBCOM_MONSTERRECORD       (DBCOM_MONSTERID       (name.data())); }
constexpr const MapRecord            &DBCOM_MAPRECORD           (const std::u8string_view &name) { return DBCOM_MAPRECORD           (DBCOM_MAPID           (name.data())); }
constexpr const BuffRecord           &DBCOM_BUFFRECORD          (const std::u8string_view &name) { return DBCOM_BUFFRECORD          (DBCOM_BUFFID          (name.data())); }
constexpr const BuffActRecord        &DBCOM_BUFFACTRECORD       (const std::u8string_view &name) { return DBCOM_BUFFACTRECORD       (DBCOM_BUFFACTID       (name.data())); }
constexpr const AttackModifierRecord &DBCOM_ATTACKMODIFIERRECORD(const std::u8string_view &name) { return DBCOM_ATTACKMODIFIERRECORD(DBCOM_ATTACKMODIFIERID(name.data())); }
constexpr const SpellModifierRecord  &DBCOM_SPELLMODIFIERRECORD (const std::u8string_view &name) { return DBCOM_SPELLMODIFIERRECORD (DBCOM_SPELLMODIFIERID (name.data())); }

constexpr const std::pair<const MagicGfxEntry *, const MagicGfxEntryRef *> DBCOM_MAGICGFXENTRY(const std::u8string_view &name, const std::u8string_view &stage)
{
    if(name.empty()){
        return {nullptr, nullptr};
    }

    if(stage.empty()){
        return {nullptr, nullptr};
    }

    if(const auto stageVal = magicStageID(stage.data()); (stageVal >= MST_BEGIN) && (stageVal < MST_END)){
        if(const auto &mr = DBCOM_MAGICRECORD(name)){
            for(const auto &entry: mr.gfxList){
                if(entry.ref && entry.ref.checkStage(stage.data())){
                    return {DBCOM_MAGICGFXENTRY(std::u8string_view(entry.ref.name), stage).first, &entry.ref};
                }

                if(entry && entry.checkStage(stage.data())){
                    return {&entry, nullptr};
                }
            }
        }
    }

    return {nullptr, nullptr};
}

constexpr const std::pair<const MagicGfxEntry *, const MagicGfxEntryRef *> DBCOM_MAGICGFXENTRY(uint32_t magicID, const std::u8string_view &stage)
{
    return DBCOM_MAGICGFXENTRY(DBCOM_MAGICRECORD(magicID).name, stage);
}

constexpr const std::optional<uint32_t> DBCOM_MAGICGFXSEFFID(const std::u8string_view &name, const std::u8string_view &stage)
{
    if(name.empty()){
        return {};
    }

    if(stage.empty()){
        return {};
    }

    if(const auto stageVal = magicStageID(stage.data()); (stageVal >= MST_BEGIN) && (stageVal < MST_END)){
        if(const auto [gfxEntry, gfxEntryRef] = DBCOM_MAGICGFXENTRY(name, stage); gfxEntry){
            if(gfxEntry->seff.ref){
                return DBCOM_MAGICGFXSEFFID(gfxEntry->seff.ref.name, gfxEntry->seff.ref.stage.empty() ? stage : gfxEntry->seff.ref.stage);
            }

            if(gfxEntry->seff.absSeffID.has_value()){
                return gfxEntry->seff.absSeffID;
            }

            if(const auto &mr = DBCOM_MAGICRECORD(name)){
                if(mr.seffBase.has_value()){
                    return 0X04000000 + mr.seffBase.value() * 16 + (magicStageID(stage.data()) - MST_BEGIN);
                }
            }
        }
    }
    return {};
}

constexpr const std::optional<uint32_t> DBCOM_MAGICGFXSEFFID(uint32_t magicID, const std::u8string_view &stage)
{
    return DBCOM_MAGICGFXSEFFID(DBCOM_MAGICRECORD(magicID).name, stage);
}
