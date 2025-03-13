#pragma once
#include <cstdint>
#include <unordered_map>
#include "msgf.hpp"
#include "staticbuffer.hpp"
#include "actionnode.hpp"

enum SMType: uint8_t
{
    SM_NONE_0 = 0,
    SM_BEGIN  = 1,
    SM_OK     = 1,
    SM_ERROR,
    SM_PING,
    SM_LOGINOK,
    SM_LOGINERROR,
    SM_CREATEACCOUNTOK,
    SM_CREATEACCOUNTERROR,
    SM_CHANGEPASSWORDOK,
    SM_CHANGEPASSWORDERROR,
    SM_QUERYCHAROK,
    SM_QUERYCHARERROR,
    SM_CREATECHAROK,
    SM_CREATECHARERROR,
    SM_CREATECHATGROUP,
    SM_ADDFRIENDACCEPTED,
    SM_ADDFRIENDREJECTED,
    SM_DELETECHAROK,
    SM_DELETECHARERROR,
    SM_ONLINEOK,
    SM_ONLINEERROR,
    SM_STARTGAMESCENE,
    SM_PLAYERCONFIG,
    SM_FRIENDLIST,
    SM_LEARNEDMAGICLIST,
    SM_PLAYERWLDESP,
    SM_ACTION,
    SM_CORECORD,
    SM_HEALTH,
    SM_NEXTSTRIKE,
    SM_NOTIFYDEAD,
    SM_DEADFADEOUT,
    SM_EXP,
    SM_BUFF,
    SM_BUFFIDLIST,
    SM_MISS,
    SM_CASTMAGIC,
    SM_OFFLINE,
    SM_PICKUPERROR,
    SM_REMOVEGROUNDITEM,
    SM_NPCXMLLAYOUT,
    SM_NPCSELL,
    SM_STARTINVOP,
    SM_STARTINPUT,
    SM_GOLD,
    SM_INVOPCOST,
    SM_STRIKEGRID,
    SM_SELLITEMLIST,
    SM_TEXT,
    SM_PLAYERNAME,
    SM_BUILDVERSION,
    SM_INVENTORY,
    SM_BELT,
    SM_UPDATEITEM,
    SM_REMOVEITEM,
    SM_REMOVESECUREDITEM,
    SM_BUYSUCCEED,
    SM_BUYERROR,
    SM_GROUNDITEMIDLIST,
    SM_GROUNDFIREWALLLIST,
    SM_EQUIPWEAR,
    SM_EQUIPWEARERROR,
    SM_GRABWEAR,
    SM_GRABWEARERROR,
    SM_EQUIPBELT,
    SM_EQUIPBELTERROR,
    SM_GRABBELT,
    SM_GRABBELTERROR,
    SM_SHOWSECUREDITEMLIST,
    SM_TEAMMEMBERLIST,
    SM_TEAMCANDIDATE,
    SM_TEAMERROR,
    SM_QUESTDESPUPDATE,
    SM_QUESTDESPLIST,
    SM_CHATMESSAGELIST,
    SM_END,
};

static_assert(SM_END < 128);
#pragma pack(push, 1)

struct SMPing
{
    uint32_t Tick;
};

struct SMLoginError
{
    uint8_t error;
};

struct SMCreateAccountError
{
    uint32_t error;
};

struct SMChangePasswordError
{
    uint32_t error;
};

struct SMQueryCharOK
{
    StaticBuffer<SYS_NAMESIZE> name;
    uint8_t gender;
    uint8_t job;
    uint32_t exp;
};

struct SMQueryCharError
{
    uint8_t error;
};

struct SMCreateCharError
{
    uint8_t error;
};

struct SMDeleteCharError
{
    uint8_t error;
};

struct SMOnlineOK
{
    uint64_t uid;
    StaticBuffer<128> name;
    uint8_t gender : 1;
    uint8_t job    : 3;
    uint32_t mapID;
    ActionNode action;
};

struct SMOnlineError
{
    uint8_t error;
};

struct SMAction
{
    uint64_t UID;
    uint32_t mapID;
    ActionNode action;
};

struct SMCORecord
{
    uint64_t UID;
    uint32_t mapID;
    ActionNode action;

    struct _SMCORecord_Monster
    {
        uint32_t MonsterID;
    };

    struct _SMCORecord_Player
    {
        uint8_t gender : 1;
        uint8_t job    : 3;
        uint32_t Level;
    };

    struct _SMCORecord_NPC
    {
        uint32_t NPCID;
    };

    union
    {
        _SMCORecord_Monster Monster;
        _SMCORecord_Player  Player;
        _SMCORecord_NPC     NPC;
    };
};

struct SMNotifyDead
{
    uint64_t UID;
};

struct SMDeadFadeOut
{
    uint64_t UID;
    uint32_t mapID;

    uint32_t X;
    uint32_t Y;
};

struct SMExp
{
    uint32_t exp;
};

struct SMBuff
{
    uint64_t uid;
    uint32_t type;
    uint32_t state;
};

struct SMMiss
{
    uint64_t UID;
};

struct SMCastMagic
{
    uint64_t UID;
    uint32_t mapID;

    uint8_t Magic;
    uint8_t MagicParam;
    uint8_t Speed;
    uint8_t Direction;

    uint16_t X;
    uint16_t Y;
    uint16_t AimX;
    uint16_t AimY;
    uint64_t AimUID;
};

struct SMOffline
{
    uint64_t UID;
    uint32_t mapID;
};

struct SMPickUpError
{
    uint32_t failedItemID;
};

struct SMRemoveGroundItem
{
    uint16_t X;
    uint16_t Y;
    uint32_t ID;
    uint32_t DBID;
};

struct SMGold
{
    uint32_t gold;
};

struct SMInvOpCost
{
    uint32_t invOp;
    uint32_t itemID;
    uint32_t seqID;
    uint32_t cost;
};

struct SMStrikeGrid
{
    uint32_t x;
    uint32_t y;
};

struct SMBuildVersion
{
    StaticBuffer<128> version;
};

struct SMRemoveItem
{
    uint32_t itemID;
    uint32_t  seqID;
    uint16_t  count;
};

struct SMRemoveSecuredItem
{
    uint32_t itemID;
    uint32_t  seqID;
};

struct SMBuySucceed
{
    uint64_t npcUID;
    uint32_t itemID;
    uint32_t  seqID;
};

struct SMBuyError
{
    uint64_t npcUID;
    uint32_t itemID;
    uint32_t  seqID;
    uint16_t  error;
};

struct SMEquipWearError
{
    uint32_t itemID;
    uint32_t  seqID;
    uint16_t  error;
};

struct SMGrabWearError
{
    uint16_t error;
};

struct SMEquipBeltError
{
    uint32_t itemID;
    uint32_t  seqID;
    uint16_t  error;
};

struct SMGrabBeltError
{
    uint16_t error;
};

struct SMTeamMemberLeft
{
    uint64_t uid;
};

struct SMTeamError
{
    uint8_t error;
};

#pragma pack(pop)

namespace
{
    inline const auto _RSVD_severmsg_attribute_list = []
    {
        std::array<std::unique_ptr<const msgf::MsgAttribute>, std::min<size_t>(SM_END, 128)> result;
#define _RSVD_register_servermsg(type, ...) result.at(type) = std::make_unique<msgf::MsgAttribute>(#type, __VA_ARGS__)

        //  0    :     empty
        //  1    : not empty,     fixed size,     compressed
        //  2    : not empty,     fixed size, not compressed
        //  3    : not empty, not fixed size, not compressed
        _RSVD_register_servermsg(SM_NONE_0,              0                               );
        _RSVD_register_servermsg(SM_OK,                  3                               );
        _RSVD_register_servermsg(SM_ERROR,               3                               );
        _RSVD_register_servermsg(SM_PING,                2, sizeof(SMPing)               );
        _RSVD_register_servermsg(SM_LOGINOK,             0                               );
        _RSVD_register_servermsg(SM_LOGINERROR,          1, sizeof(SMLoginError)         );
        _RSVD_register_servermsg(SM_CREATEACCOUNTOK,     0                               );
        _RSVD_register_servermsg(SM_CREATEACCOUNTERROR,  1, sizeof(SMCreateAccountError) );
        _RSVD_register_servermsg(SM_CHANGEPASSWORDOK,    0                               );
        _RSVD_register_servermsg(SM_CHANGEPASSWORDERROR, 1, sizeof(SMChangePasswordError));
        _RSVD_register_servermsg(SM_CREATECHAROK,        0                               );
        _RSVD_register_servermsg(SM_CREATECHARERROR,     1, sizeof(SMCreateCharError)    );
        _RSVD_register_servermsg(SM_CREATECHATGROUP,     3                               );
        _RSVD_register_servermsg(SM_ADDFRIENDACCEPTED,   3                               );
        _RSVD_register_servermsg(SM_ADDFRIENDREJECTED,   3                               );
        _RSVD_register_servermsg(SM_DELETECHAROK,        0                               );
        _RSVD_register_servermsg(SM_DELETECHARERROR,     1, sizeof(SMDeleteCharError)    );
        _RSVD_register_servermsg(SM_ONLINEOK,            1, sizeof(SMOnlineOK)           );
        _RSVD_register_servermsg(SM_ONLINEERROR,         1, sizeof(SMOnlineError)        );
        _RSVD_register_servermsg(SM_STARTGAMESCENE,      3                               );
        _RSVD_register_servermsg(SM_PLAYERCONFIG,        3                               );
        _RSVD_register_servermsg(SM_FRIENDLIST,          3                               );
        _RSVD_register_servermsg(SM_LEARNEDMAGICLIST,    3                               );
        _RSVD_register_servermsg(SM_QUERYCHAROK,         1, sizeof(SMQueryCharOK)        );
        _RSVD_register_servermsg(SM_QUERYCHARERROR,      1, sizeof(SMQueryCharError)     );
        _RSVD_register_servermsg(SM_PLAYERWLDESP,        3                               );
        _RSVD_register_servermsg(SM_ACTION,              1, sizeof(SMAction)             );
        _RSVD_register_servermsg(SM_CORECORD,            1, sizeof(SMCORecord)           );
        _RSVD_register_servermsg(SM_HEALTH,              3                               );
        _RSVD_register_servermsg(SM_NEXTSTRIKE,          0                               );
        _RSVD_register_servermsg(SM_NOTIFYDEAD,          1, sizeof(SMNotifyDead)         );
        _RSVD_register_servermsg(SM_DEADFADEOUT,         1, sizeof(SMDeadFadeOut)        );
        _RSVD_register_servermsg(SM_EXP,                 1, sizeof(SMExp)                );
        _RSVD_register_servermsg(SM_BUFF,                1, sizeof(SMBuff)               );
        _RSVD_register_servermsg(SM_BUFFIDLIST,          3                               );
        _RSVD_register_servermsg(SM_MISS,                1, sizeof(SMMiss)               );
        _RSVD_register_servermsg(SM_CASTMAGIC,           1, sizeof(SMCastMagic)          );
        _RSVD_register_servermsg(SM_OFFLINE,             1, sizeof(SMOffline)            );
        _RSVD_register_servermsg(SM_PICKUPERROR,         1, sizeof(SMPickUpError)        );
        _RSVD_register_servermsg(SM_REMOVEGROUNDITEM,    1, sizeof(SMRemoveGroundItem)   );
        _RSVD_register_servermsg(SM_NPCXMLLAYOUT,        3                               );
        _RSVD_register_servermsg(SM_NPCSELL,             3                               );
        _RSVD_register_servermsg(SM_STARTINVOP,          3                               );
        _RSVD_register_servermsg(SM_STARTINPUT,          3                               );
        _RSVD_register_servermsg(SM_GOLD,                1, sizeof(SMGold)               );
        _RSVD_register_servermsg(SM_INVOPCOST,           1, sizeof(SMInvOpCost)          );
        _RSVD_register_servermsg(SM_STRIKEGRID,          1, sizeof(SMStrikeGrid)         );
        _RSVD_register_servermsg(SM_SELLITEMLIST,        3                               );
        _RSVD_register_servermsg(SM_TEXT,                3                               );
        _RSVD_register_servermsg(SM_PLAYERNAME,          3                               );
        _RSVD_register_servermsg(SM_BUILDVERSION,        1, sizeof(SMBuildVersion)       );
        _RSVD_register_servermsg(SM_INVENTORY,           3                               );
        _RSVD_register_servermsg(SM_BELT,                3                               );
        _RSVD_register_servermsg(SM_UPDATEITEM,          3                               );
        _RSVD_register_servermsg(SM_REMOVEITEM,          1, sizeof(SMRemoveItem)         );
        _RSVD_register_servermsg(SM_REMOVESECUREDITEM,   1, sizeof(SMRemoveSecuredItem)  );
        _RSVD_register_servermsg(SM_BUYSUCCEED,          1, sizeof(SMBuySucceed)         );
        _RSVD_register_servermsg(SM_BUYERROR,            1, sizeof(SMBuyError)           );
        _RSVD_register_servermsg(SM_GROUNDITEMIDLIST,    3                               );
        _RSVD_register_servermsg(SM_GROUNDFIREWALLLIST,  3                               );
        _RSVD_register_servermsg(SM_EQUIPWEAR,           3                               );
        _RSVD_register_servermsg(SM_EQUIPWEARERROR,      1, sizeof(SMEquipWearError)     );
        _RSVD_register_servermsg(SM_GRABWEAR,            3                               );
        _RSVD_register_servermsg(SM_GRABWEARERROR,       1, sizeof(SMGrabWearError)      );
        _RSVD_register_servermsg(SM_EQUIPBELT,           3                               );
        _RSVD_register_servermsg(SM_EQUIPBELTERROR,      1, sizeof(SMEquipBeltError)     );
        _RSVD_register_servermsg(SM_GRABBELT,            3                               );
        _RSVD_register_servermsg(SM_GRABBELTERROR,       1, sizeof(SMGrabBeltError)      );
        _RSVD_register_servermsg(SM_SHOWSECUREDITEMLIST, 3                               );
        _RSVD_register_servermsg(SM_TEAMMEMBERLIST,      3                               );
        _RSVD_register_servermsg(SM_TEAMCANDIDATE,       3                               );
        _RSVD_register_servermsg(SM_TEAMERROR,           1, sizeof(SMTeamError)          );
        _RSVD_register_servermsg(SM_QUESTDESPUPDATE,     3                               );
        _RSVD_register_servermsg(SM_QUESTDESPLIST,       3                               );
        _RSVD_register_servermsg(SM_CHATMESSAGELIST,     3                               );

#undef _RSVD_register_servermsg
        return result;
    }();
}

class ServerMsg final: public msgf::MsgBase
{
    public:
        ServerMsg(uint8_t argHeadCode)
            : MsgBase(argHeadCode)
        {}

    private:
        const msgf::MsgAttribute &getAttribute() const override
        {
            if(const auto attPtr = _RSVD_severmsg_attribute_list[headCode()].get()){
                return *attPtr;
            }
            else{
                throw fflerror("message not registered: %02d", (int)(headCode()));
            }
        }

    public:
        static const msgf::MsgAttribute *headCodeValid(uint8_t argHeadCode)
        {
            return _RSVD_severmsg_attribute_list[argHeadCode & 0x7f].get();
        }

    public:
        template<typename T> static T conv(const uint8_t *buf, size_t bufLen = 0)
        {
            static_assert(std::is_trivially_copyable_v<T>);

            if(bufLen && bufLen != sizeof(T)){
                throw fflerror("invalid buffer length");
            }

            T t;
            std::memcpy(&t, buf, sizeof(t));
            return t;
        }
};
