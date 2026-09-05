local dialogue = require('npc.include.dialog')
local invop = require('npc.include.invop')

-- 药店 / 中医, legacy Market_Def/04Potion_*.txt (26)
--
-- the largest merchant type and the clearest cut: it buys and sells and **never repairs**. not
-- one of the 26 has a @repair label, which is why this is its own template rather than an
-- outfitter with a flag turned off — a potion has no durability to mend
--
--     local apothecary = require('npc.include.merchant.apothecary')
--     apothecary.setApothecary
--     {
--         greet = {'出门在外时，多带上些药品心里才踏实。'},
--         goods = {'金创药（小）', '魔法药（小）'},
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local apothecary = {}

-- StdMode 0 and 3 from King_StdItems.csv. In particular, +3 is not every "道具".
apothecary.TRADE_ITEMS =
{
    '金创药（小）', '魔法药（小）', '地牢逃脱卷', '金创药（中）', '魔法药（中）',
    '战神油', '回城卷', '祝福油', '太阳水', '随机传送卷',
    '行会回城卷', '修复油', '金创药（大）', '魔法药（大）', '强效太阳水',
    '彩卷', '鹿茸', '米糕', '鹿血', '神水',
    '人参', '馒头', '红苹果', '特殊药水', '万年雪霜',
    '水饺', '攻击神水（小）', '自然神水（小）', '灵魂神水（小）', '疾风神水（小）',
    '体力强效神水（小）', '魔法强效神水（小）', '攻击神水（中）', '自然神水（中）', '灵魂神水（中）',
    '疾风神水（中）', '体力强效神水（中）', '魔力强效神水（中）', '攻击神水（大）', '自然神水（大）',
    '灵魂神水（大）', '体力强效神水（大）', '魔力强效神水（大）', '攻击神水（特）', '自然神水（特）',
    '灵魂神水（特）', '体力强效神水（特）', '魔力强效神水（特）', '疾风神水（特）', '疾风神水（大）',
    '苹果', '鸡血', '烧酒', '无名药', '胆汁',
    '战酒', '活动专用褐色栗子', '活动专用铜色栗子', '活动专用银色栗子', '活动专用金色栗子',
    '回生神水', '武器强化油', '诅咒之药水', '亡灵之药水', '金创药（特）',
    '魔法药（特）', '饺子（自然）', '饺子（灵魂）', '饺子（攻击）', '饺子（疾风）',
    '饺子（体力）', '饺子（魔力）', '汤圆（自然）', '汤圆（灵魂）', '汤圆（攻击）',
    '汤圆（疾风）', '汤圆（体力）', '汤圆（魔力）', '爱情礼盒', '魔法糖果',
    '灵魂糖果', '疾风糖果', '攻击糖果', '体力糖果', '时空之门',
    '随身NPC', '时空传送',
}

-- QuestDiary/Event/PotionEvent/PEvent.txt: @Event_Wjwn and @Event_Ghltod.
apothecary.SPECIAL_POTIONS =
{
    {
        flag = 'cursedPotion',
        tag = 'npc_wjwn',
        item = '诅咒之药水',
        price = 5000000,
        offer = '在找诅咒之药水吗? 你真幸运,我刚刚拿到了好东西想看看吗? 每一瓶价格是500万金币.',
        done = '呵呵,真是有福气的年轻人.随时欢迎你再来.能有像你这样有福气的老顾客,对我来说也不是好事吗?',
        decline = '如果是这样,就只能作罢.想清楚后再来吧.',
    },
    {
        flag = 'rebirthPotion',
        tag = 'npc_ghltod',
        item = '回生神水',
        price = 1500000,
        offer = '回生神水可是要150万金币哦..., 年轻人你怎么想?',
        done = '好.没有比合理价格来购买好东西更愉快的事情.我这里还有很多好东西.如果需要随时欢迎你再来.',
        decline = '真可惜.购买人不想要我也没有办法.如果需要药水请不要犹豫.欢迎你再来.',
    },
}

function apothecary.setApothecary(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty apothecary greeting')

    local label = spec.label or '药品'
    -- Legacy +0,+3: recovery potions, scrolls, special potions and lottery/event items.
    local trade = optList(spec.trade, {'恢复药水', '传送卷轴', '功能药水', '强效药水', '道具'})
    local price = spec.price or 50
    local back = {dialogue.link(SYS_ENTER, spec.backLabel or '前一步')}
    local menu = {}
    local handler = {}
    local acceptItem
    if trade and spec.tradeItems ~= false then
        acceptItem = invop.itemNameFilter(spec.tradeItems or apothecary.TRADE_ITEMS)
    end

    if spec.goods then
        setNPCSell(spec.goods)
        table.insert(menu, dialogue.link('npc_buy', spec.buyLabel or '购买', {suffix = spec.buySuffix or label}))
        handler.npc_buy = function(uid, value)
            dialogue.post(uid, spec.buyText or {'出门在外时，多带上些药品心里才踏实。'}, back)
            uidPostSell(uid)
        end
    end

    if trade then
        table.insert(menu, dialogue.link('npc_sell', spec.sellLabel or '出售', {suffix = spec.sellSuffix or label}))
        handler.npc_sell = function(uid, value)
            dialogue.post(uid, spec.sellText or {'请把你想出售的药放在这里。'}, back)
            invop.uidStartTrade(uid, 'npc_sell_query', 'npc_sell_commit', trade)
        end
        handler.npc_sell_query = function(uid, value)
            invop.postQueryTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price, acceptItem)
        end
        handler.npc_sell_commit = function(uid, value)
            invop.postCommitTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price, acceptItem)
        end
    end

    for _, potion in ipairs(apothecary.SPECIAL_POTIONS) do
        if spec[potion.flag] then
            local itemID = getItemID(potion.item)
            assert(itemID > 0, 'unknown special potion: ' .. potion.item)
            local buyTag = potion.tag .. '_buy'
            local declineTag = potion.tag .. '_decline'
            local close = {dialogue.link(SYS_EXIT, '关  闭')}
            table.insert(menu, dialogue.link(potion.tag, '购买', {prefix = potion.item}))

            handler[potion.tag] = function(uid, value)
                dialogue.post(uid, {potion.offer},
                {
                    dialogue.link(buyTag, '购买.'),
                    dialogue.link(declineTag, '再想一想.'),
                })
            end
            handler[buyTag] = function(uid, value)
                local bought = uidRemoteCall(uid, itemID, potion.price,
                [[
                    local itemID, price = ...
                    if not removeGold(price) then
                        return false
                    end
                    addItem(itemID, 1)
                    return true
                ]])
                assertType(bought, 'boolean')
                dialogue.post(uid, {bought and potion.done or '什么? 没钱你还想购买药水? 等你有了钱再来吧.'}, close)
            end
            handler[declineTag] = function(uid, value)
                dialogue.post(uid, {potion.decline}, close)
            end
        end
    end

    for _, topic in ipairs(spec.topics or {}) do
        table.insert(menu, dialogue.link(topic.id, topic.label, {prefix = topic.prefix, suffix = topic.suffix}))
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

return apothecary
