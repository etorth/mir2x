local merchant = require('npc.include.merchant')

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
--         trade = {'材料'},
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local buyer = {}

function buyer.setBuyer(spec)
    assertType(spec, 'table')

    merchant.setMerchant
    {
        greet   = spec.greet,
        redName = spec.redName,

        label = spec.label or '材料',

        -- nothing to buy, by definition
        goods = nil,

        trade = optList(spec.trade, {'材料'}),
        price = spec.price,

        repair  = nil,
        special = false,

        sellText = spec.sellText or {'你要出售什么？'},

        today  = spec.today,
        topics = spec.topics,
        extra  = spec.extra,
    }
end

return buyer
