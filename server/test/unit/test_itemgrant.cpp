#include <climits>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "actormsg.hpp"
#include "chatdb.hpp"
#include "dbpod.hpp"
#include "deliverydb.hpp"
#include "golddb.hpp"
#include "inventorydb.hpp"
#include "jobf.hpp"
#include "uidf.hpp"

class ServerArgParser;
class PeerConfig;
class Log;
class ActorPool;
class MapBinDB;
class ScriptWindow;
class ProfilerWindow;
class MainWindow;
class Server;
class ServerPasswordWindow;
class ServerConfigureWindow;
class PodMonitorWindow;
class ActorMonitorWindow;

ServerArgParser       *g_serverArgParser = nullptr;
PeerConfig            *g_peerConfig = nullptr;
Log                   *g_mir2xLog = nullptr;
ActorPool             *g_actorPool = nullptr;
DBPod                 *g_dbPod = nullptr;
MapBinDB              *g_mapBinDB = nullptr;
ScriptWindow          *g_scriptWindow = nullptr;
ProfilerWindow        *g_profilerWindow = nullptr;
MainWindow            *g_mainWindow = nullptr;
Server                *g_server = nullptr;
ServerPasswordWindow  *g_serverPasswordWindow = nullptr;
ServerConfigureWindow *g_serverConfigureWindow = nullptr;
PodMonitorWindow      *g_podMonitorWindow = nullptr;
ActorMonitorWindow    *g_actorMonitorWindow = nullptr;

namespace
{
    void require(bool condition, const char *message)
    {
        if(!condition){
            throw std::runtime_error(message);
        }
    }

    bool sameItem(const SDItem &lhs, const SDItem &rhs)
    {
        return lhs.itemID == rhs.itemID
            && lhs.seqID == rhs.seqID
            && lhs.count == rhs.count
            && lhs.duration[0] == rhs.duration[0]
            && lhs.duration[1] == rhs.duration[1]
            && lhs.extAttrList == rhs.extAttrList;
    }

    void checkInventory(const SDInventory &actual, const SDInventory &expected)
    {
        require(actual.getItemList().size() == expected.getItemList().size(), "inventory size changed");
        for(const auto &item: expected.getItemList()){
            const auto found = actual.find(item.itemID, item.seqID);
            require(found && sameItem(*found, item), "inventory item changed");
        }
    }

    int countRows(const char *table)
    {
        auto query = g_dbPod->createQuery("select count(*) from %s", table);
        require(query.executeStep(), "missing row count");
        return query.getColumn(0).getInt();
    }

    void setupDatabase()
    {
        g_dbPod->launch(":memory:");
        g_dbPod->exec("pragma foreign_keys = on");
        g_dbPod->createFunction("jobValid", 1, true, +[](sqlite3_context *context, int, sqlite3_value **argv)
        {
            const auto job = sqlite3_value_int64(argv[0]);
            sqlite3_result_int(context, job >= 0 && job <= INT_MAX && jobf::jobValid(to_d(job)));
        });

        static constexpr unsigned char schema[]
        {
            #embed "../../src/server.sql" suffix(,)
            '\0'
        };
        g_dbPod->exec("%s", to_rawcstr(schema));
        g_dbPod->exec("insert into tbl_account(fld_dbid, fld_account, fld_password) values(2, 'offline', 'test')");
        for(const int dbid: {1, 2}){
            g_dbPod->exec(
                "insert into tbl_char(fld_dbid, fld_name, fld_gender, fld_job, fld_map, fld_mapx, fld_mapy, fld_gold)"
                " values(%d, 'test%d', 0, %d, 1, 0, 0, 100)", dbid, dbid, JOB_WARRIOR);
        }
    }

    void runTests()
    {
        const auto findItemID = [](const auto &predicate) -> uint32_t
        {
            for(uint32_t itemID = 1; itemID < DBCOM_ITEMENDID(); ++itemID){
                if(const auto &ir = DBCOM_ITEMRECORD(itemID); ir && predicate(ir)){
                    return itemID;
                }
            }
            throw std::runtime_error("missing item fixture");
        };
        const auto stackID = findItemID([](const auto &ir){ return ir.packable() && !ir.isGold(); });
        const auto weaponID = findItemID([](const auto &ir){ return ir.isWeapon(); });
        const auto goldID = findItemID([](const auto &ir){ return ir.isGold(); });

        SDItem weapon
        {
            .itemID = weaponID,
            .duration{7, 19},
            .extAttrList{SDItem::build_EA_BIND(true), SDItem::build_EA_DC(3)},
        };
        const std::vector<SDItem> items
        {
            weapon,
            SDItem{.itemID = stackID, .count = 5},
            SDItem{.itemID = goldID, .count = 37},
        };

        const auto grant = cerealf::deserialize<SDGrantItemList>(cerealf::serialize(SDGrantItemList
        {
            .playerUID = uidf::getPlayerUID(1),
            .itemList = items,
        }));
        require(grant.playerUID == uidf::getPlayerUID(1), "grant recipient lost in serialization");
        require(grant.itemList.size() == items.size(), "grant items lost in serialization");
        for(size_t i = 0; i < items.size(); ++i){
            require(sameItem(grant.itemList[i], items[i]), "grant attributes lost in serialization");
        }
        require(std::string(mpkName(AM_GRANTITEMLIST)) == "AM_GRANTITEMLIST", "missing actor message name");

        dbUpdateInventoryItem(1, SDItem{.itemID = stackID, .seqID = 1, .count = 3});
        dbUpdateInventoryItem(1, SDItem{.itemID = weaponID, .seqID = 1});
        const auto result = dbGrantItemList(1, grant.itemList);
        require(result.has_value(), "inventory grant was rejected");
        require(result->gold == 137 && dbLoadGold(1) == 137, "gold grant was not committed");
        checkInventory(dbLoadInventory(1), result->inventory);
        require(result->inventory.has(stackID, 0) == 8, "stackable reward was not merged");
        require(result->inventory.has(weaponID, 0) == 2, "weapon reward replaced an existing item");
        weapon.seqID = 2;
        const auto grantedWeapon = result->inventory.find(weaponID, 2);
        require(grantedWeapon && sameItem(*grantedWeapon, weapon), "weapon durability or attributes changed");
        require(countRows("tbl_delivery") == 0 && countRows("tbl_chatmessage") == 0, "inventory success created fallback mail");
        require(!dbGrantItemList(3, items).has_value(), "missing recipient accepted a grant");

        // Overflow occurs after item writes, so rejection must roll back the entire batch.
        dbUpdateGold(1, to_uz(INT64_MAX) - 2);
        const auto beforeReject = dbLoadInventory(1);
        require(!dbGrantItemList(1, items).has_value(), "overflowing grant was accepted");
        require(dbLoadGold(1) == to_uz(INT64_MAX) - 2, "rejected grant changed gold");
        checkInventory(dbLoadInventory(1), beforeReject);
        const auto limitGrant = dbGrantItemList(1, {SDItem{.itemID = goldID, .count = 2}});
        require(limitGrant && limitGrant->gold == to_uz(INT64_MAX) && dbLoadGold(1) == to_uz(INT64_MAX), "grant reaching the gold limit was rejected");
        checkInventory(dbLoadInventory(1), beforeReject);

        // Simulate a database error at the last write and ensure no earlier reward survived it.
        dbUpdateGold(1, 137);
        g_dbPod->exec(
            "create temp trigger fail_grant before update of fld_gold on tbl_char"
            " begin select raise(abort, 'injected grant failure'); end");
        bool failed = false;
        try{
            dbGrantItemList(1, items);
        }
        catch(const SQLite::Exception &){
            failed = true;
        }
        g_dbPod->exec("drop trigger fail_grant");
        require(failed, "database failure was not propagated");
        require(dbLoadGold(1) == 137, "failed grant changed gold");
        checkInventory(dbLoadInventory(1), beforeReject);

        // The offline path needs no Player actor and stores the full attachment under the recipient's DBID.
        const auto delivery = dbCreateDelivery(2, items);
        require(delivery.message.seq.has_value(), "fallback message has no database sequence");
        require(countRows("tbl_delivery") == 1 && countRows("tbl_chatmessage") == 1, "fallback mail was not saved");
        require(dbLoadGold(2) == 100 && dbLoadInventory(2).getItemList().empty(), "mail fallback changed inventory");
        auto query = g_dbPod->createQuery("select fld_dbid, fld_payload, fld_messageid, fld_claimed from tbl_delivery where fld_record = ?");
        query.bind(1, delivery.record);
        require(query.executeStep(), "missing delivery record");
        require(query.getColumn("fld_dbid").getInt() == 2, "wrong attachment recipient");
        require(query.getColumn("fld_claimed").getInt() == 0, "fallback attachment was already claimed");
        require(to_u64(query.getColumn("fld_messageid").getInt64()) == delivery.message.seq->id, "attachment lost its chat message");
        const auto attachedItems = cerealf::deserialize<std::vector<SDItem>>(query.getColumn("fld_payload").getString());
        require(attachedItems.size() == items.size(), "fallback attachment lost items");
        for(size_t i = 0; i < items.size(); ++i){
            require(sameItem(attachedItems[i], items[i]), "fallback attachment changed item data");
        }
        require(!query.executeStep(), "duplicate delivery record");
        const auto message = dbQueryChatMessage(delivery.message.seq->id);
        require(message && message->to.id() == 2 && message->to.player(), "fallback message was not addressed to the player");
        require(cerealf::deserialize<std::string>(message->message).find(delivery.record) != std::string::npos, "fallback message has no claim link");

        g_dbPod->exec(
            "create temp trigger fail_delivery before insert on tbl_chatmessage"
            " begin select raise(abort, 'injected delivery failure'); end");
        failed = false;
        try{
            dbCreateDelivery(2, items);
        }
        catch(const SQLite::Exception &){
            failed = true;
        }
        g_dbPod->exec("drop trigger fail_delivery");
        require(failed, "mail persistence failure was not propagated");
        require(countRows("tbl_delivery") == 1 && countRows("tbl_chatmessage") == 1, "failed mail left an orphaned attachment");

        std::puts("Item grants passed: serialization, inventory, attributes, atomic rejection, database failures, and offline attachments.");
    }
}

int main()
{
    try{
        DBPod database;
        g_dbPod = &database;
        setupDatabase();
        runTests();
        g_dbPod = nullptr;
        return 0;
    }
    catch(const std::exception &e){
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
