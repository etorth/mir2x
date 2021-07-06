local invop = {}

function invop.uidStartSell(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_SELL, queryTag, commitTag, typeList)
end

function invop.uidStartLock(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_LOCK, queryTag, commitTag, typeList)
end

function invop.uidStartRepair(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_REPAIR, queryTag, commitTag, typeList)
end

function invop.postSellPrice(uid, itemID, seqID, price)
    uidPostInvOpCost(uid, INVOP_SELL, itemID, seqID, price)
end

function invop.postLockCost(uid, itemID, seqID, cost)
    uidPostInvOpCost(uid, INVOP_LOCK, itemID, seqID, cost)
end

function invop.postRepairCost(uid, itemID, seqID, cost)
    uidPostInvOpCost(uid, INVOP_REPAIR, itemID, seqID, cost)
end

function invop.uidStartLock(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_LOCK, queryTag, commitTag, typeList)
end

function invop.uidStartRepair(uid, queryTag, commitTag, typeList)
    uidPostStartInvOp(uid, INVOP_REPAIR, queryTag, commitTag, typeList)
end

function invop.parseItemString(itemString)
    local seqLoc = string.find(itemString, ':')
    if seqLoc == nil then
        fatalPrintf('Invalid item string: %s', tostring(itemString))
    end

    itemIDString = string.sub(itemString, 1, seqLoc - 1)
    seqIDString  = string.sub(itemString,    seqLoc + 1)
    return {tonumber(itemIDString, 10), tonumber(seqIDString, 10)}
end

return invop
