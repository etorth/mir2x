local merchant = require('npc.include.merchant')

-- 肉店, legacy Market_Def/01Meet_*.txt (8)
--
-- the odd one out in two ways. it is mostly a **buyer**: all 8 take meat off you and only 3 sell
-- anything, so its menu leads with 卖. and it says 卖 / 买 where every other trade says
-- 出售 / 购买 — the short form, 21 times against 5
--
-- 12 of them also answer 询问获取肉的途径, which is where a player finds out that meat comes
-- from butchering an animal corpse with Alt+click, that an animal which ran harder gives better
-- meat, and that anything killed with magic gives quality 0
--
--     local butcher = require('npc.include.merchant.butcher')
--     butcher.setButcher
--     {
--         greet = {'我想买品质好的肉。', '我愿意多付钱。'},
--         meatHelp = true,                   -- offer 询问获取肉的途径
--         goods = {'烤肉'},                  -- only 3 of the 8 sell anything
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
butcher.MEAT_HELP =
{
    '可以通过屠宰鸡，鹿，羊，狼等动物获取肉。',
    '首先抓住那些动物，然后按Alt健，在动物尸体上点击鼠标，即可看到正在切肉的样子。',
    '然后你的包裹里就会出现大块大块的肉。',
    '要记住越是不愿意被抓住而拼命逃跑的动物品质越好。而且，使用魔法抓住的动物品质为0。',
}

function butcher.setButcher(spec)
    assertType(spec, 'table')

    local topics = {}

    if spec.meatHelp then
        table.insert(topics, {id = 'npc_meat_help', label = '询问获取肉的途径', text = spec.meatHelpText or butcher.MEAT_HELP})
    end

    for _, t in ipairs(spec.topics or {}) do
        table.insert(topics, t)
    end

    merchant.setMerchant
    {
        greet   = spec.greet,
        redName = spec.redName,

        label = spec.label or '肉',

        -- the short menu words this trade uses
        buyLabel  = spec.buyLabel or '买',
        sellLabel = spec.sellLabel or '卖',

        goods = spec.goods,
        trade = optList(spec.trade, {'肉'}),
        price = spec.price,

        repair  = nil,
        special = false,

        buyText  = spec.buyText or {'请挑选你想要的商品。'},
        sellText = spec.sellText or {'我想买品质好的肉。', '我愿意多付钱。'},

        today  = spec.today,
        topics = topics,
        extra  = spec.extra,
    }
end

return butcher
