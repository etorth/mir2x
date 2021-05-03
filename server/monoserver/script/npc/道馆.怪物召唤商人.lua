-- =====================================================================================
--
--       Filename: 道馆.物品展示商人.lua
--        Created: 11/22/2020 08:52:57 PM
--    Description: lua 5.3
--
--        Version: 1.0
--       Revision: none
--       Compiler: lua
--
--         Author: ANHONG
--          Email: anhonghe@gmail.com
--   Organization: USTC
--
-- =====================================================================================

-- NPC script
-- provides the table: processNPCEvent for event processing

addLog(LOGTYPE_INFO, 'NPC %s sources %s', getNPCFullName(), getFileName())
setNPCLook(5)
setNPCGLoc(406, 126)

processNPCEvent = {}
do
    local monsterID = 1
    local monsterNameEventString = ''

    while(true) do
        local monsterName = getMonsterName(monsterID)
        if monsterName == '' then
            break
        end

        if monsterName ~= '未知' then
            local tagName = string.format('event_goto_%d', monsterID)
            monsterNameEventString = monsterNameEventString .. string.format([[<event id="%s" wrap="false">%s</event>，]], tagName, monsterName)
            processNPCEvent[tagName] = function(uid, value)
                addLog(LOGTYPE_INFO, '召唤%s', monsterName)
            end
        end
        monsterID = monsterID + 1
    end

    processNPCEvent[SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>客官%s你好我是%s，我可以帮你召唤所有的怪物哦！<emoji id="0"/></par>
                <par></par>
                <par>%s</par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), monsterNameEventString, SYS_NPCDONE)
    end
end
