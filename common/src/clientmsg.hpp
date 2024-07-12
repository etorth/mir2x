#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include "msgf.hpp"
#include "conceptf.hpp"
#include "staticbuffer.hpp"
#include "actionnode.hpp"
#include "staticvector.hpp"

enum CMType: uint8_t
{
    // CM_NONE already used on windows
    // outside of this file use zero directly for invalid value
    CM_NONE_0 = 0,
    CM_BEGIN  = 1,
    CM_PING   = 1,
    CM_LOGIN,
    CM_QUERYCHAR,
    CM_CREATECHAR,
    CM_DELETECHAR,
    CM_CHANGEPASSWORD,
    CM_ONLINE,
    CM_ACTION,
    CM_SETMAGICKEY,
    CM_SETRUNTIMECONFIG,
    CM_QUERYCORECORD,
    CM_REQUESTADDEXP,
    CM_REQUESTKILLPETS,
    CM_REQUESTRETRIEVESECUREDITEM,
    CM_REQUESTSPACEMOVE,
    CM_REQUESTMAGICDAMAGE,
    CM_PICKUP,
    CM_QUERYGOLD,
    CM_QUERYUIDBUFF,
    CM_QUERYPLAYERNAME,
    CM_QUERYPLAYERWLDESP,
    CM_QUERYCHATPEERLIST,
    CM_CREATECHATGROUP,
    CM_CREATEACCOUNT,
    CM_NPCEVENT,
    CM_QUERYSELLITEMLIST,
    CM_DROPITEM,
    CM_CONSUMEITEM,
    CM_MAKEITEM,
    CM_BUY,
    CM_ADDFRIEND,
    CM_CHATMESSAGE,
    CM_REQUESTEQUIPWEAR,
    CM_REQUESTGRABWEAR,
    CM_REQUESTEQUIPBELT,
    CM_REQUESTGRABBELT,
    CM_REQUESTJOINTEAM,
    CM_REQUESTLEAVETEAM,
    CM_REQUESTLATESTCHATMESSAGE,
    CM_END,
};

static_assert(CM_END < 128);
#pragma pack(push, 1)

struct CMPing
{
    uint32_t Tick;
};

struct CMLogin
{
    StaticBuffer<SYS_IDSIZE> id;
    StaticBuffer<SYS_PWDSIZE> password;
};

struct CMSetMagicKey
{
    uint32_t magicID;
    uint8_t key;
};

struct CMCreateChar
{
    StaticBuffer<SYS_NAMESIZE> name;
    uint8_t job;
    uint8_t gender;
};

struct CMDeleteChar
{
    StaticBuffer<SYS_PWDSIZE> password;
};

struct CMAction
{
    uint64_t UID;
    uint32_t mapID;
    ActionNode action;
};

struct CMQueryCORecord
{
    uint64_t AimUID;
};

struct CMRequestAddExp
{
    uint64_t addExp;
};

struct CMRequestRetrieveSecuredItem
{
    uint32_t itemID;
    uint32_t seqID;
};

struct CMRequestSpaceMove
{
    uint32_t mapID;
    uint16_t X;
    uint16_t Y;
};

struct CMRequestMagicDamage
{
    uint32_t magicID;
    uint64_t aimUID;
};

struct CMPickUp
{
    uint16_t x;
    uint16_t y;
    uint32_t mapID;
};

struct CMQueryUIDBuff
{
    uint64_t uid;
};

struct CMQueryPlayerName
{
    uint64_t uid;
};

struct CMQueryPlayerWLDesp
{
    uint64_t uid;
};

struct CMQueryChatPeerList
{
    StaticBuffer<128> input;
};

struct CMCreateChatGroup
{
    StaticBuffer<128> name;
    StaticVector<uint32_t, 512> list;
};

struct CMCreateAccount
{
    StaticBuffer<SYS_IDSIZE> id;
    StaticBuffer<SYS_PWDSIZE> password;
};

struct CMChangePassword
{
    StaticBuffer<SYS_IDSIZE> id;
    StaticBuffer<SYS_PWDSIZE> password;
    StaticBuffer<SYS_PWDSIZE> passwordNew;
};

struct CMSetRuntimeConfig
{
    uint16_t type;
    StaticBuffer<256> buf;
};

struct CMNPCEvent
{
    uint64_t uid;

    char path [100];
    char event[100];
    char value[200];

    int16_t valueSize;
};

struct CMQuerySellItemList
{
    uint64_t npcUID;
    uint32_t itemID;
};

struct CMDropItem
{
    uint32_t itemID;
    uint32_t seqID;
    uint16_t count;
};

struct CMConsumeItem
{
    uint32_t itemID;
    uint32_t seqID;
    uint16_t count;
};

struct CMMakeItem
{
    uint32_t itemID;
    uint16_t count;
};

struct CMBuy
{
    uint64_t npcUID;
    uint32_t itemID;
    uint32_t seqID;
    uint32_t count;
};

struct CMAddFriend
{
    uint32_t dbid;
};

struct CMRequestEquipWear
{
    uint32_t itemID;
    uint32_t seqID;
    uint16_t wltype;
};

struct CMRequestGrabWear
{
    uint16_t wltype;
};

struct CMRequestEquipBelt
{
    uint32_t itemID;
    uint32_t seqID;
    uint16_t slot;
};

struct CMRequestGrabBelt
{
    uint16_t slot;
};

struct CMRequestJoinTeam
{
    uint64_t uid;
};

struct CMRequestLeaveTeam
{
    uint64_t uid;
};

struct CMRequestLatestChatMessage
{
    StaticVector<uint64_t, 128> cpidList;

    uint32_t limitCount  : 30;
    uint32_t includeSend :  1;
    uint32_t includeRecv :  1;
};

#pragma pack(pop)

// I was using class name ClientMessage
// unfortunately this name has been taken by Xlib
// and ClientMessage seems like a real message but actually this class only give general information

namespace
{
    inline const auto _RSVD_clientmsg_attribute_list = []
    {
        std::array<std::unique_ptr<const msgf::MsgAttribute>, std::min<size_t>(CM_END, 128)> result;
#define _RSVD_register_clientmsg(type, ...) result.at(type) = std::make_unique<msgf::MsgAttribute>(#type, __VA_ARGS__)

        //  0    :     empty
        //  1    : not empty,     fixed size,     compressed
        //  2    : not empty,     fixed size, not compressed
        //  3    : not empty, not fixed size, not compressed
        _RSVD_register_clientmsg(CM_NONE_0,                     0                                      );
        _RSVD_register_clientmsg(CM_PING,                       2, sizeof(CMPing)                      );
        _RSVD_register_clientmsg(CM_LOGIN,                      1, sizeof(CMLogin)                     );
        _RSVD_register_clientmsg(CM_QUERYCHAR,                  0                                      );
        _RSVD_register_clientmsg(CM_CREATECHAR,                 1, sizeof(CMCreateChar)                );
        _RSVD_register_clientmsg(CM_DELETECHAR,                 1, sizeof(CMDeleteChar)                );
        _RSVD_register_clientmsg(CM_ONLINE,                     0                                      );
        _RSVD_register_clientmsg(CM_ACTION,                     1, sizeof(CMAction)                    );
        _RSVD_register_clientmsg(CM_SETMAGICKEY,                1, sizeof(CMSetMagicKey)               );
        _RSVD_register_clientmsg(CM_QUERYCORECORD,              1, sizeof(CMQueryCORecord)             );
        _RSVD_register_clientmsg(CM_REQUESTADDEXP,              1, sizeof(CMRequestAddExp)             );
        _RSVD_register_clientmsg(CM_REQUESTKILLPETS,            0                                      );
        _RSVD_register_clientmsg(CM_REQUESTRETRIEVESECUREDITEM, 1, sizeof(CMRequestRetrieveSecuredItem));
        _RSVD_register_clientmsg(CM_REQUESTSPACEMOVE,           1, sizeof(CMRequestSpaceMove)          );
        _RSVD_register_clientmsg(CM_REQUESTMAGICDAMAGE,         1, sizeof(CMRequestMagicDamage)        );
        _RSVD_register_clientmsg(CM_PICKUP,                     1, sizeof(CMPickUp)                    );
        _RSVD_register_clientmsg(CM_QUERYGOLD,                  0                                      );
        _RSVD_register_clientmsg(CM_QUERYUIDBUFF,               1, sizeof(CMQueryUIDBuff)              );
        _RSVD_register_clientmsg(CM_QUERYPLAYERNAME,            1, sizeof(CMQueryPlayerName)           );
        _RSVD_register_clientmsg(CM_QUERYPLAYERWLDESP,          1, sizeof(CMQueryPlayerWLDesp)         );
        _RSVD_register_clientmsg(CM_QUERYCHATPEERLIST,          1, sizeof(CMQueryChatPeerList)         );
        _RSVD_register_clientmsg(CM_CREATECHATGROUP,            1, sizeof(CMCreateChatGroup)           );
        _RSVD_register_clientmsg(CM_CREATEACCOUNT,              1, sizeof(CMCreateAccount)             );
        _RSVD_register_clientmsg(CM_CHANGEPASSWORD,             1, sizeof(CMChangePassword)            );
        _RSVD_register_clientmsg(CM_SETRUNTIMECONFIG,           1, sizeof(CMSetRuntimeConfig)          );
        _RSVD_register_clientmsg(CM_NPCEVENT,                   1, sizeof(CMNPCEvent)                  );
        _RSVD_register_clientmsg(CM_QUERYSELLITEMLIST,          1, sizeof(CMQuerySellItemList)         );
        _RSVD_register_clientmsg(CM_DROPITEM,                   1, sizeof(CMDropItem)                  );
        _RSVD_register_clientmsg(CM_CONSUMEITEM,                1, sizeof(CMConsumeItem)               );
        _RSVD_register_clientmsg(CM_MAKEITEM,                   1, sizeof(CMMakeItem)                  );
        _RSVD_register_clientmsg(CM_BUY,                        1, sizeof(CMBuy)                       );
        _RSVD_register_clientmsg(CM_ADDFRIEND,                  1, sizeof(CMAddFriend)                 );
        _RSVD_register_clientmsg(CM_CHATMESSAGE,                3                                      );
        _RSVD_register_clientmsg(CM_REQUESTEQUIPWEAR,           1, sizeof(CMRequestEquipWear)          );
        _RSVD_register_clientmsg(CM_REQUESTGRABWEAR,            1, sizeof(CMRequestGrabWear)           );
        _RSVD_register_clientmsg(CM_REQUESTEQUIPBELT,           1, sizeof(CMRequestEquipBelt)          );
        _RSVD_register_clientmsg(CM_REQUESTGRABBELT,            1, sizeof(CMRequestGrabBelt)           );
        _RSVD_register_clientmsg(CM_REQUESTJOINTEAM,            1, sizeof(CMRequestJoinTeam)           );
        _RSVD_register_clientmsg(CM_REQUESTLEAVETEAM,           1, sizeof(CMRequestLeaveTeam)          );
        _RSVD_register_clientmsg(CM_REQUESTLATESTCHATMESSAGE,   1, sizeof(CMRequestLatestChatMessage)  );

#undef _RSVD_register_clientmsg
        return result;
    }();
}

class ClientMsg final: public msgf::MsgBase
{
    public:
        ClientMsg(uint8_t headCode)
            : MsgBase(headCode)
        {}

    private:
        const msgf::MsgAttribute &getAttribute() const override
        {
            if(auto attPtr = _RSVD_clientmsg_attribute_list[headCode()].get()){
                return *attPtr;
            }
            else{
                throw fflerror("message not registered: %02d", (int)(headCode()));
            }
        }

    public:
        static const msgf::MsgAttribute *headCodeValid(uint8_t argHeadCode)
        {
            return _RSVD_clientmsg_attribute_list[argHeadCode & 0x7f].get();
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

struct ClientMsgBuf final
{
    const uint8_t  headCode;
    const uint8_t *data;
    const  size_t  size;

    ClientMsgBuf(uint8_t argHeadCode, const void *argData, const size_t argSize)
        : headCode(argHeadCode)
        , data(reinterpret_cast<const uint8_t *>(argData))
        , size(argSize)
    {
        if(headCode & 0x80){
            throw fflerror("invalid head code 0x%02x", (int)(headCode));
        }

        if(!ClientMsg(headCode).checkData(data, size)){
            throw fflerror("invalid data size, data %p, size %zu", (const void *)(data), size);
        }
    }

    ClientMsgBuf(uint8_t argHeadCode, const std::string &buf)
        : ClientMsgBuf(argHeadCode, buf.empty() ? nullptr : buf.data(), buf.size())
    {}

    ClientMsgBuf(uint8_t argHeadCode, const std::u8string &buf)
        : ClientMsgBuf(argHeadCode, buf.empty() ? nullptr : buf.data(), buf.size())
    {}

    ClientMsgBuf(uint8_t argHeadCode, const std::string_view &buf)
        : ClientMsgBuf(argHeadCode, buf.empty() ? nullptr : buf.data(), buf.size())
    {}

    ClientMsgBuf(uint8_t argHeadCode, const std::u8string_view &buf)
        : ClientMsgBuf(argHeadCode, buf.empty() ? nullptr : buf.data(), buf.size())
    {}

    ClientMsgBuf(uint8_t argHeadCode)
        : ClientMsgBuf(argHeadCode, nullptr, 0)
    {}

    template<conceptf::TriviallyCopyable T> ClientMsgBuf(uint8_t argHeadCode, const T &t)
        : ClientMsgBuf(argHeadCode, &t, sizeof(t))
    {}

    template<typename T> T conv() const
    {
        if constexpr (!std::is_trivially_copyable_v<T>){
            return cerealf::deserialize<T>(data, size);
        }
        else if(sizeof(T) == size){
            // always use memcpy
            // a trivially copyable type may also defines serialization/deserialization

            T t;
            std::memcpy(&t, data, size);
            return t;
        }
        else{
            throw fflerror("failed to deserialize %zu bytes into %zu buffer", size, sizeof(T));
        }
    }
};
