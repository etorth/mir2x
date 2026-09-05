local dialogue = require('npc.include.dialogue')
local invop = require('npc.include.invop')

-- 肉店, legacy Market_Def/01Meet_*.txt (8)
--
-- meat counters lead with 卖 and use 卖 / 买 rather than 出售 / 购买. An unused @buy
-- section is not a stock counter: only supply goods if purchase is reachable in the main menu.
--
-- some also answer 询问获取肉的途径, which is where a player finds out that meat comes
-- from butchering an animal corpse with Alt+click, that an animal which ran harder gives better
-- meat, and that anything killed with magic gives quality 0
--
--     local butcher = require('npc.include.merchant.butcher')
--     butcher.setButcher
--     {
--         greet = {'我想买品质好的肉。', '我愿意多付钱。'},
--         meatHelp = true,                   -- offer 询问获取肉的途径
--         goods = {'烤肉'},                  -- only when the source offers purchases
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local butcher = {}

-- @NPC_MeatHelp, the same answer in every shop that has it
function butcher.MEAT_HELP(uid)
    local name = uidQueryName(uid)
    return
    {
        '可以通过屠宰鸡，鹿，羊，狼等动物获取肉。',
        '首先抓住那些动物，然后按Alt健，在动物尸体上点击鼠标，即可看到正在切肉的' .. name .. '。',
        '然后' .. name .. '的包裹里就会出现大块大块的肉',
        '要记住越是不愿意被抓住而拼命逃跑的动物品质越好。而且，使用魔法抓住的动物品质为0。',
    }
end

function butcher.setButcher(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty butcher greeting')

    local label = spec.label or '肉'
    local trade = optList(spec.trade, {'肉'})
    local price = spec.price or 50
    local back = {dialogue.link(SYS_ENTER, spec.backLabel or '前一步')}
    local menu = {}
    local handler = {}

    if trade then
        table.insert(menu, dialogue.link('npc_sell', spec.sellLabel or '卖', spec.sellSuffix or label))
        handler.npc_sell = function(uid, value)
            dialogue.post(uid, spec.sellText or {'我想买品质好的肉。', '我愿意多付钱。'}, back)
            invop.uidStartTrade(uid, 'npc_sell_query', 'npc_sell_commit', trade)
        end
        handler.npc_sell_query = function(uid, value)
            invop.postQueryTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price)
        end
        handler.npc_sell_commit = function(uid, value)
            invop.postCommitTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price)
        end
    end

    if spec.goods then
        setNPCSell(spec.goods)
        table.insert(menu, dialogue.link('npc_buy', spec.buyLabel or '买', spec.buySuffix or label))
        handler.npc_buy = function(uid, value)
            dialogue.post(uid, spec.buyText or {'请挑选你想要的商品。'}, back)
            uidPostSell(uid)
        end
    end

    if spec.meatHelp then
        table.insert(menu, dialogue.link('npc_meat_help', '询问获取肉的途径'))
        handler.npc_meat_help = function(uid, value)
            dialogue.post(uid, spec.meatHelpText or butcher.MEAT_HELP, back)
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

return butcher
