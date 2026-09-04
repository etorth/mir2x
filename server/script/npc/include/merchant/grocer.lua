local merchant = require('npc.include.merchant')

-- 杂货店, legacy Market_Def/07Grocery_*.txt (16) and 10ChestnutMarket_*.txt (4)
--
-- the general store: candles, torches, 地牢逃脱卷. buys and sells, and one of the 16 also
-- repairs, so repair is off by default and that one turns it on
--
--     local grocer = require('npc.include.merchant.grocer')
--     grocer.setGrocer
--     {
--         greet = {'你要买什么？'},
--         goods = {'蜡烛', '火把', '地牢逃脱卷'},
--         repair = {'衣服', '武器'},         -- only 07Grocery_Bichon-0 does this
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local grocer = {}

function grocer.setGrocer(spec)
    assertType(spec, 'table')

    merchant.setMerchant
    {
        greet   = spec.greet,
        redName = spec.redName,

        label = spec.label or '物品',
        goods = spec.goods,
        trade = optList(spec.trade, {'杂物'}),
        price = spec.price,

        -- off unless this one is the shop that mends things
        repair  = optList(spec.repair, nil),
        special = false,

        buyText    = spec.buyText or {'你要买什么？'},
        sellText   = spec.sellText or {'请把不用的东西卖给我'},
        repairText = spec.repairText or {'这里可以修理衣服和武器之类的东西。'},

        today  = spec.today,
        topics = spec.topics,
        extra  = spec.extra,
    }
end

return grocer
