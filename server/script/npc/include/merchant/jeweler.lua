local merchant = require('npc.include.merchant')

-- 饰品店, legacy Market_Def/08Accessory_*.txt (12) and 08Astrologist_*.txt (2)
--
-- rings, bracelets and necklaces. buys, sells and repairs, no 特殊修理. the shop that tells you
-- to check durability before you commit — 请先看好价钱和持久性再决定
--
--     local jeweler = require('npc.include.merchant.jeweler')
--     jeweler.setJeweler
--     {
--         greet = {'你想买饰品?'},
--         goods = {'铁手镯', '银戒指'},
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local jeweler = {}

function jeweler.setJeweler(spec)
    assertType(spec, 'table')

    merchant.setMerchant
    {
        greet   = spec.greet,
        redName = spec.redName,

        label = spec.label or '饰品',
        goods = spec.goods,
        trade = optList(spec.trade, {'手镯', '戒指', '项链'}),
        price = spec.price,

        repair  = optList(spec.repair, {'手镯', '戒指', '项链'}),
        special = false,

        buyText    = spec.buyText or {'你想买饰品? 想买什么？请先看好价钱和持久性再决定。'},
        sellText   = spec.sellText or {'你想出售饰品？', '请先把东西拿出来给我看看。'},
        repairText = spec.repairText or {'你想修理饰品?'},

        today  = spec.today,
        topics = spec.topics,
        extra  = spec.extra,
    }
end

return jeweler
