local merchant = require('npc.include.merchant')

-- 布店 / 服装店 / 鞋店, legacy Market_Def/03Armor_*.txt (15) and 03Shoes_*.txt (2)
--
-- buys, sells and repairs, and unlike a smith never offers 特殊修理 — none of the 17 do
--
--     local outfitter = require('npc.include.merchant.outfitter')
--     outfitter.setOutfitter
--     {
--         greet = {'欢迎光临，需要什么？'},
--         goods = {'布衣（男）', '布衣（女）'},
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local outfitter = {}

function outfitter.setOutfitter(spec)
    assertType(spec, 'table')

    merchant.setMerchant
    {
        greet   = spec.greet,
        redName = spec.redName,

        label = spec.label or '防御工具',
        goods = spec.goods,
        trade = optList(spec.trade, {'衣服', '头盔'}),
        price = spec.price,

        -- 特殊修理 is a smith's trade, an outfitter never has it
        repair  = optList(spec.repair, {'衣服', '头盔'}),
        special = false,

        buyText    = spec.buyText or {'你要买什么？'},
        sellText   = spec.sellText or {'请把要卖的衣服(头盔)放到上面。'},
        repairText = spec.repairText or {'防御工具，头盔和帽子都可以修理。'},

        today  = spec.today,
        topics = spec.topics,
        extra  = spec.extra,
    }
end

return outfitter
