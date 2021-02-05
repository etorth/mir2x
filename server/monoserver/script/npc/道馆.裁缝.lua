-- =====================================================================================
--
--       Filename: 道馆.铁匠.lua
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

addLog(LOGTYPE_INFO, string.format('NPC %s sources %s', getNPCFullName(), getFileName()))
local function randomHeadString()
    if math.random(0, 1) == 0 then
        return '与衣服鞋子有关的都可以找我！'
    else
        return '有什么可以为你效劳的吗？'
    end
end

local function getSell()
    return {'太阳水', '草鞋', '布鞋', '皮靴', '五彩鞋', '鹿皮靴', '钢靴', '天掌靴', '赤飞靴', '黑皮靴', '长皮靴', '月光鞋', '无影靴', '仙云靴', '武神靴', '踏浪靴', '锦靴', '黄金靴', '紫金靴', '乌金靴', '青铜头盔', '道士头盔', '骷髅头盔', '虎面头盔', '破山剑', '青铜剑', '旋风流星刀', '破魂'}
end

processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        sayXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好我是%s，%s<emoji id="0"/></par>
                <par></par>
                <par><event id="event_post_sell">购买</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), randomHeadString(), SYS_NPCDONE))
    end,

    ["event_post_sell"] = function(uid, value)
        uidPostSell(uid, getSell())
    end,
}
