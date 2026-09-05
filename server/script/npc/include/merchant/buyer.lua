local dialogue = require('npc.include.dialogue')
local invop = require('npc.include.invop')

-- 材料商, legacy Market_Def/10Material_*.txt (13)
--
-- stocks nothing at all. not one of the 13 has a [Goods] section or a @buy label — it only ever
-- takes things off the player, which is why it gets its own template instead of a merchant with
-- goods set to nil: a 购买 entry on this menu would be wrong, not merely empty
--
--     local buyer = require('npc.include.merchant.buyer')
--     buyer.setBuyer
--     {
--         greet = {'你要出售什么？'},
--         trade = {'道具'},
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local buyer = {}

-- StdMode 52 from King_StdItems.csv; other quest props also use the "道具" inventory type.
buyer.TRADE_ITEMS =
{
    '食人树叶', '毒蜘蛛牙齿', '食人树的果实', '蝎子的尾巴', '蛆卵',
    '宝玉', '蚂蚁卵', '毒蛇胆汁', '震天魔印', '思念珍珠',
    '骷髅骨', '潘夜珠', '潘夜之泪', '夜明珠', '牙齿',
    '蜘蛛线', '皮', '灵魂', '指甲', '神灵雕像',
    '僵尸骨头', '跳蚤皮', '号角', '霸群雕像', '遗物',
    '生存游戏场地地图1', '生存游戏场地地图2', '生存游戏场地地图3', '生存游戏场地地图4', '游戏点卷',
    '嫁祸卡', '飞龙剑碎片（火）', '飞龙剑碎片（冰）', '飞龙剑碎片（雷）', '飞龙剑碎片（风）',
    '飞龙剑碎片（神圣）', '飞龙剑碎片（暗黑）', '飞龙剑碎片（幻影）', '金牛', '石钥匙',
    '死亡木偶', '彩色钱币', '冰针', '檀木扇', '皇家玉石',
    '亡灵雕象', '王者圣杯', '骷髅碎骨', '金色魔瓶', '蓝色魔瓶',
    '金钥匙', '梳子', '古币碎片', '古币', '古琵琶',
    '银铃', '礼物盒', '蓝星浩月', '宝箱', '积分彩票',
    '英雄遗物（英）', '英雄遗物（雄）', '英雄遗物（遗）', '英雄遗物（物）', '普通饲料',
    '高级饲料',
}

function buyer.setBuyer(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty buyer greeting')

    local trade = optList(spec.trade, {'道具'})
    local price = spec.price or 50
    local back = {dialogue.link(SYS_ENTER, spec.backLabel or '前一步')}
    local menu = {}
    local handler = {}
    local acceptItem
    if trade and spec.tradeItems ~= false then
        acceptItem = invop.itemNameFilter(spec.tradeItems or buyer.TRADE_ITEMS)
    end

    if trade then
        table.insert(menu, dialogue.link('npc_sell', spec.sellLabel or '出售', spec.sellSuffix or spec.label or '材料'))
        handler.npc_sell = function(uid, value)
            dialogue.post(uid, spec.sellText or {'你要出售什么？'}, back)
            invop.uidStartTrade(uid, 'npc_sell_query', 'npc_sell_commit', trade)
        end
        handler.npc_sell_query = function(uid, value)
            invop.postQueryTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price, acceptItem)
        end
        handler.npc_sell_commit = function(uid, value)
            invop.postCommitTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price, acceptItem)
        end
    end

    for _, topic in ipairs(spec.topics or {}) do
        table.insert(menu, dialogue.link(topic.id, topic.label, topic.suffix, topic.prefix))
        handler[topic.id] = topic.handler or function(uid, value)
            dialogue.post(uid, topic.text, {dialogue.link(SYS_ENTER, topic.back or spec.backLabel or '前一步')})
        end
    end

    if spec.today then
        table.insert(menu, dialogue.link('npc_today', '对今日的任务进行了解'))
        handler.npc_today = function(uid, value)
            dialogue.post(uid, {spec.today}, {dialogue.link(SYS_EXIT, spec.todayExit or '结束')})
        end
    end

    table.insert(menu, dialogue.link(SYS_EXIT, spec.exitLabel or '结束'))
    handler[SYS_ENTER] = function(uid, value)
        dialogue.post(uid, spec.greet, menu)
    end

    for tag, callback in pairs(spec.extra or {}) do
        handler[tag] = callback
    end
    for tag, callback in pairs(handler) do
        if type(callback) == 'function' and tag ~= SYS_LABEL and tag ~= SYS_HIDE and tag ~= SYS_CHECKACTIVE and tag ~= SYS_ALLOWREDNAME then
            handler[tag] = dialogue.guardRedName(callback, spec.redName, spec.redNameExit or '结束')
        end
    end
    handler[SYS_ALLOWREDNAME] = true
    setEventHandler(handler)
    return handler
end

return buyer
