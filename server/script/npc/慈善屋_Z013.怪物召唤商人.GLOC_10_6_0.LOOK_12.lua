local dialog = require('include.dialog')

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
            monsterNameList[#monsterNameList + 1] = dialog.link(tagName, monsterName .. '，', {wrap = false})

            eventHandlerTable[tagName] = function(uid, value)
                addMonster(monsterName)
            end
        end
    end
    monsterID = monsterID + 1
end

eventHandlerTable[SYS_ENTER] = function(uid, value)
    dialog.post(uid, string.format('客官%s你好我是%s，我可以召唤所有的怪物哦！<emoji id="0"/>', uidQueryName(uid), getNPCName()),
    {
        {table.concat(monsterNameList), align = 'justify'},
        '',
        dialog.link(SYS_EXIT, '关闭'),
    })
end

setEventHandler(eventHandlerTable)
