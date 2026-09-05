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

function invop.postStartInput(uid, title, commitTag, show)
    uidPostStartInput(uid, title, commitTag, show)
end

function invop.parseItemString(itemString)
    local itemID, seqID = string.match(itemString, '^(%d+):(%d+)$')
    return tonumber(itemID, 10), tonumber(seqID, 10)
end

-- Some legacy categories share a broader mir2x inventory type (notably "道具").
function invop.itemNameFilter(names)
    local accepted = {}
    for _, name in ipairs(names) do
        if getItemID(name) == 0 then
            fatalPrintf('Unknown trade item: %s', name)
        end
        accepted[name] = true
    end
    return function(itemID)
        return accepted[getItemName(itemID)] == true
    end
end

-- 修理     : restores the current durability, may permanently lose part of the max durability
-- 特殊修理 : restores the current durability without wearing the item out, costs more
--
-- an NPC which offers repair only needs to forward its own invop args here:
--
--     ["npc_goto_query_repair" ] = function(uid, value) invop.postQueryRepair (uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", {'武器'}) end,
--     ["npc_goto_commit_repair"] = function(uid, value) invop.postCommitRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", {'武器'}) end,
--
-- the args are needed because these handlers post xml, which rolls the xml layout seqID the tags
-- client holds are encoded with, so the tags have to be issued again before client clicks any further

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

local function queryRepair(uid, value, queryTag, commitTag, typeList, special)
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
    else
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

    invop.uidStartRepair(uid, queryTag, commitTag, typeList)
end

local function commitRepair(uid, value, queryTag, commitTag, typeList, special, sayDone, backLabel)
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

    elseif uidQueryGold(uid) < cost then
        uidPostXML(uid,
        [[
            <layout>
                <par>修理%s需要%d金币，你带的钱不够。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), cost, SYS_ENTER)

    elseif not server.player.removeGold(uid, cost) then
        -- can not pay for it, don't repair

    elseif not uidRepairItem(uid, itemID, seqID, special) then
        uidGrantGold(uid, cost)

    else
        -- sayDone is the shopkeeper's own line, legacy @NPC_Repair_Complete. it goes above the
        -- receipt so the NPC speaks first
        local said = sayDone and string.format('<par>%s</par>', sayDone) or ''
        local back = backLabel == false and '' or
            string.format('<par><event id="%s">%s</event></par>', SYS_ENTER, backLabel or '前一步')

        uidPostXML(uid,
        [[
            <layout>
                %s<par>你的%s已经修理完毕，花费%d金币。</par>
                <par></par>

                %s
            </layout>
        ]], said, getItemName(itemID), cost, back)
    end

    invop.uidStartRepair(uid, queryTag, commitTag, typeList)
end

function invop.postQueryRepair(uid, value, queryTag, commitTag, typeList)
    queryRepair(uid, value, queryTag, commitTag, typeList, false)
end

function invop.postQuerySpecialRepair(uid, value, queryTag, commitTag, typeList)
    queryRepair(uid, value, queryTag, commitTag, typeList, true)
end

function invop.postCommitRepair(uid, value, queryTag, commitTag, typeList, sayDone, backLabel)
    commitRepair(uid, value, queryTag, commitTag, typeList, false, sayDone, backLabel)
end

function invop.postCommitSpecialRepair(uid, value, queryTag, commitTag, typeList, sayDone, backLabel)
    commitRepair(uid, value, queryTag, commitTag, typeList, true, sayDone, backLabel)
end

-- 出售 : NPC buys an item off player at a flat price
--
-- there is no item price in the item record yet so the caller passes one in, keep it under what
-- NPChar::getCostItemList() charges, otherwise player buys from the NPC and sells straight back
--
--     ["npc_goto_query_trade" ] = function(uid, value) invop.postQueryTrade (uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", typeList, price) end,
--     ["npc_goto_commit_trade"] = function(uid, value) invop.postCommitTrade(uid, value, "npc_goto_query_trade", "npc_goto_commit_trade", typeList, price) end,

local function rejectTrade(uid, itemID, queryTag, commitTag, typeList, acceptItem)
    if acceptItem == nil or acceptItem(itemID) then
        return false
    end
    uidPostXML(uid,
    [[
        <layout>
            <par>这里不收购%s。</par>
            <par></par>
            <par><event id="%s">前一步</event></par>
        </layout>
    ]], getItemName(itemID), SYS_ENTER)
    invop.uidStartTrade(uid, queryTag, commitTag, typeList)
    return true
end

function invop.postQueryTrade(uid, value, queryTag, commitTag, typeList, price, acceptItem)
    local itemID, seqID = invop.parseItemString(value)
    if rejectTrade(uid, itemID, queryTag, commitTag, typeList, acceptItem) then
        return
    end

    uidPostXML(uid,
    [[
        <layout>
            <par>你的%s我看过了，出价%d金币。</par>
            <par>你要卖吗？</par>
            <par></par>

            <par><event id="%s">前一步</event></par>
        </layout>
    ]], getItemName(itemID), price, SYS_ENTER)

    invop.postTradePrice(uid, itemID, seqID, price)
    invop.uidStartTrade(uid, queryTag, commitTag, typeList)
end

function invop.postCommitTrade(uid, value, queryTag, commitTag, typeList, price, acceptItem)
    local itemID, seqID = invop.parseItemString(value)
    if rejectTrade(uid, itemID, queryTag, commitTag, typeList, acceptItem) then
        return
    end

    if uidRemove(uid, {itemID = itemID, seqID = seqID}) then
        uidGrantGold(uid, price)
        uidPostXML(uid,
        [[
            <layout>
                <par>成交，这是%d金币，收好了。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], price, SYS_ENTER)
    else
        uidPostXML(uid,
        [[
            <layout>
                <par>你的%s已经不在身上了。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], getItemName(itemID), SYS_ENTER)
    end

    invop.uidStartTrade(uid, queryTag, commitTag, typeList)
end

return invop
