#pragma once
#include <array>
#include <string>
#include <numeric>
#include <variant>
#include <utility>
#include <optional>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include "totype.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "dbcomid.hpp"
#include "luaf.hpp"

struct SDItem
{
    enum SDItemXMLLayoutParamType: int
    {
        XML_NONE  = 0,
        XML_BEGIN = 1,
        XML_DC    = 1,
        XML_MC,
        XML_SC,
        XML_LEVEL,
        XML_JOB,

        XML_PRICE,
        XML_PRICECOLOR,
        XML_END,
    };

    constexpr static int EA_NONE  = 0;
    constexpr static int EA_BEGIN = 1;

    constexpr static int _ea_add_type_counter_begin = __COUNTER__;
#define _MACRO_ADD_EA_TYPE(eaType, eaValType) \
    constexpr static int eaType = __COUNTER__ - _ea_add_type_counter_begin; \
    struct eaType##_t \
    { \
        constexpr static int value = eaType; \
        using type = eaValType; \
    }; \
    template<typename ... Args> static std::pair<const int, std::string> build_##eaType(Args && ... args) \
    { \
        return {eaType, cerealf::serialize<eaValType>(eaValType(std::forward<Args>(args)...), -1)}; \
    } \

    /**/ // begin of extra-attributes
    /**/ // each line provides three types, take EA_DC as example:
    /**/ //
    /**/ //     SDItem::EA_DC           : an integer used as key
    /**/ //     SDItem::EA_DC_t         : an type contains: EA_DC_t::value and EA_DC_t::type
    /**/ //     SDItem::build_EA_DC()   : an function returns std::pair<int, std::string> where second is the serialized extra-attributes value
    /**/ //
    /**/ // don't put any other code except the macro defines and type aligns
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_DC , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MC , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_SC , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_AC , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MAC, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_DCHIT, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MCHIT, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_DCDODGE, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MCDODGE, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_SPEED, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_COMFORT, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_LUCKCURSE, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_HPADD, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_HPSTEAL, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_HPRECOVER, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_MPADD, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MPSTEAL, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MPRECOVER, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_DCFIRE   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCICE    , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCLIGHT  , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCWIND   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCHOLY   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCDARK   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCPHANTOM, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_ACFIRE   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACICE    , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACLIGHT  , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACWIND   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACHOLY   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACDARK   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACPHANTOM, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_LOADBODY, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_LOADWEAPON, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_LOADINVENTORY, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_EXTEXP, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_EXTGOLD, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_EXTDROP, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_BUFFID, int)
    /**/
    /**/ using _helper_type_EA_TELEPORT_t = std::tuple<uint32_t, int, int>;
    /**/ _MACRO_ADD_EA_TYPE(EA_TELEPORT, _helper_type_EA_TELEPORT_t)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_COLOR, uint32_t)
    /**/
    /**/ // end of extra-attributes
    /**/ // any extra-attributes should be put inside above region

#undef _MACRO_ADD_EA_TYPE
    constexpr static int EA_END = __COUNTER__ - _ea_add_type_counter_begin;

    uint32_t itemID = 0;
    uint32_t  seqID = 0;

    size_t count = 1;
    size_t duration[2] = {0, 0};
    std::unordered_map<int, std::string> extAttrList = {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(itemID, seqID, count, duration[0], duration[1], extAttrList);
    }

    std::string str() const
    {
        return str_printf("(name, itemID, seqID, count, duration) = (%s, %zu, %zu, %zu, (%zu, %zu))", to_cstr(DBCOM_ITEMRECORD(itemID).name), to_uz(itemID), to_uz(seqID), count, duration[0], duration[1]);
    }

    std::u8string getXMLLayout(const std::unordered_map<int, std::string> & = {}) const;

    bool isGold() const
    {
        return to_u8sv(DBCOM_ITEMRECORD(itemID).type) == u8"金币";
    }

    operator bool () const;
    static std::vector<SDItem> buildGoldItem(size_t);

    template<typename T> std::optional<typename T::type> getExtAttr() const
    {
        static_assert(T::value >= EA_BEGIN);
        static_assert(T::value <  EA_END  );

        if(const auto p = extAttrList.find(T::value); p != extAttrList.end()){
            return cerealf::deserialize<typename T::type>(p->second);
        }
        return {};
    }

    luaf::luaVar asLuaVar() const
    {
        return luaf::buildLuaVar(std::unordered_map<std::string, luaf::luaVar>
        {
            {"itemID", itemID},
            { "seqID",  seqID},
        });
    }
};
