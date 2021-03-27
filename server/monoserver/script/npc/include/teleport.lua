local tp = {}

function tp.uidSpaceMove(uid, mapName, x, y)
    local mapID = getMapID(mapName)
    if mapID == 0 then
        return false
    end

    local value = uidQuery(uid, string.format("SPACEMOVE %d %d %d", mapID, x, y))
    if value == '1' then
        return true
    elseif value == '0' then
        return false
    else
        errorPrintf('invalid value: %s', value)
    end
end

function tp.uidReqSpaceMove(uid, mapName, x, y, gold, level)
    if gold  == nil then gold  = 0 end
    if level == nil then level = 0 end

    if type(gold) ~= 'number' or type(level) ~= 'number' then
        errorPrintf("invalid argument type: gold: %s, level: %s", type(gold), type(level))
    end

    if gold > 0 and gold > uidQueryGold(uid) then
        sayXML(uid,
        [[
            <layout>
                <par>你没有%d金币！</par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], gold, SYS_NPCDONE)
        return
    end

    if level > 0 and level > uidQueryLevel(uid) then
        sayXML(uid,
        [[
            <layout>
                <par>你还没达到%d级！</par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], level, SYS_NPCDONE)
        return
    end

    if tp.uidSpaceMove(uid, mapName, x, y) then
        -- uidUseGold(uid, gold)
    end
end

return tp
