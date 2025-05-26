local monsterID = 1
local monsterNameList = {}
local eventHandlerTable = {}

while true do
    local monsterName = getMonsterName(monsterID)
    if not hasChar(monsterName) then
        break
    end

    local suffixDigits = string.match(monsterName, '^.-(%d+)$')
    if suffixDigits == nil then
        if monsterName ~= '未知' then
            local tagName = string.format('goto_tag_%d', monsterID)
            monsterNameList[#monsterNameList + 1] = string.format([[<event id="%s" wrap="false">%s，</event>]], tagName, monsterName)

            eventHandlerTable[tagName] = function(uid, value)
                addMonster(monsterName)
            end
        end
    end
    monsterID = monsterID + 1
end

eventHandlerTable[SYS_ENTER] = function(uid, value)
    uidPostXML(uid,
    [[
        <layout>
            <par>客官%s你好我是%s，我可以召唤所有的怪物哦！<emoji id="0"/></par>
            <par></par>
            <par align="justify">%s</par>
            <par></par>
            <par><event id="%s" close="1">关闭</event></par>
        </layout>
    ]], uidQueryName(uid), getNPCName(), table.concat(monsterNameList), SYS_EXIT)
end

setEventHandler(eventHandlerTable)
