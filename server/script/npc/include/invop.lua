local invop = {}

function invop.uidStartTrade(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_TRADE, queryTag, commitTag, typeList)
end

function invop.uidStartSecure(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_SECURE, queryTag, commitTag, typeList)
end

function invop.uidStartRepair(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_REPAIR, queryTag, commitTag, typeList)
end

function invop.postTradePrice(uid, itemID, seqID, price)
    uidPostInvOpCost(uid, INVOP_TRADE, itemID, seqID, price)
end

function invop.postSecureCost(uid, itemID, seqID, cost)
    uidPostInvOpCost(uid, INVOP_SECURE, itemID, seqID, cost)
end

function invop.postRepairCost(uid, itemID, seqID, cost)
    uidPostInvOpCost(uid, INVOP_REPAIR, itemID, seqID, cost)
end

function invop.uidStartSecure(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_SECURE, queryTag, commitTag, typeList)
end

function invop.uidStartRepair(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_REPAIR, queryTag, commitTag, typeList)
end

function invop.postStartInput(uid, title, commitTag, show)
    uidPostStartInput(uid, title, commitTag, show)
end

function invop.parseItemString(itemString)
    local itemID, seqID = string.match(itemString, '^(%d+):(%d+)$')
    return tonumber(itemID, 10), tonumber(seqID, 10)
end

-- 修理     : restores the current durability, may permanently lose part of the max durability
-- 特殊修理 : restores the current durability without wearing the item out, costs more
--
-- an NPC which offers repair only needs to forward its query/commit tags here:
--
--     ["npc_goto_query_repair" ] = function(uid, value) invop.postQueryRepair (uid, value, false) end,
--     ["npc_goto_commit_repair"] = function(uid, value) invop.postCommitRepair(uid, value, false) end,

local repairGoldPerDuration = 30

-- returns nil when the item can not or needs not to be repaired
-- otherwise returns the price and the current/max durability

local function repairCost(uid, itemID, seqID, special)
    local duration = uidQueryItemDuration(uid, itemID, seqID)
    if duration == nil then
        return nil
    end

    local curr, max = duration[1], duration[2]
    if curr >= max then
        return nil
    end

    local cost = (max - curr) * repairGoldPerDuration
    if special then
        cost = cost * 3
    end

    return cost, curr, max
end

function invop.postQueryRepair(uid, value, special)
    local itemID, seqID = invop.parseItemString(value)
    local cost, curr, max = repairCost(uid, itemID, seqID, special)

    if cost == nil then
        uidPostXML(uid,
        [[
            <layout>
                <par>你的%s还很结实，不需要修理。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), SYS_ENTER)
        return
    end

    uidPostXML(uid,
    [[
        <layout>
            <par>你的%s持久为%d/%d，%s费用是%d金币。</par>
            <par></par>

            <par><event id="%s">前一步</event></par>
        </layout>
    ]], getItemName(itemID), curr, max, special and '特殊修理' or '修理', cost, SYS_ENTER)

    invop.postRepairCost(uid, itemID, seqID, cost)
end

function invop.postCommitRepair(uid, value, special)
    local itemID, seqID = invop.parseItemString(value)
    local cost = repairCost(uid, itemID, seqID, special)

    if cost == nil then
        uidPostXML(uid,
        [[
            <layout>
                <par>你的%s不需要修理。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), SYS_ENTER)
        return
    end

    if uidQueryGold(uid) < cost then
        uidPostXML(uid,
        [[
            <layout>
                <par>修理%s需要%d金币，你带的钱不够。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), cost, SYS_ENTER)
        return
    end

    if not uidRemoveGold(uid, cost) then
        return
    end

    if not uidRepairItem(uid, itemID, seqID, special) then
        uidGrantGold(uid, cost)
        return
    end

    uidPostXML(uid,
    [[
        <layout>
            <par>你的%s已经修理完毕，花费%d金币。</par>
            <par></par>

            <par><event id="%s">前一步</event></par>
        </layout>
    ]], getItemName(itemID), cost, SYS_ENTER)
end

return invop
