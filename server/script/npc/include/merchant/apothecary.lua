local merchant = require('npc.include.merchant')

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

function apothecary.setApothecary(spec)
    assertType(spec, 'table')

    merchant.setMerchant
    {
        greet   = spec.greet,
        redName = spec.redName,

        label = spec.label or '药品',
        goods = spec.goods,
        trade = optList(spec.trade, {'恢复药水'}),
        price = spec.price,

        -- deliberately absent, see the note above
        repair  = nil,
        special = false,

        buyText  = spec.buyText or {'出门在外时，多带上些药品心里才踏实。'},
        sellText = spec.sellText or {'请把你想出售的药放在这里。'},

        today  = spec.today,
        topics = spec.topics,
        extra  = spec.extra,
    }
end

return apothecary
