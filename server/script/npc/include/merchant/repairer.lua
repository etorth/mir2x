local merchant = require('npc.include.merchant')

-- 修理店, legacy Market_Def/09Repair_*.txt (2)
--
-- mends things and trades in nothing. no [Goods], no @buy, no @sell — the mirror image of
-- npc/include/merchant/buyer.lua
--
--     local repairer = require('npc.include.merchant.repairer')
--     repairer.setRepairer
--     {
--         greet = {'要修理什么？'},
--         repair = {'武器', '衣服', '头盔'},
--         special = true,
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local repairer = {}

function repairer.setRepairer(spec)
    assertType(spec, 'table')

    merchant.setMerchant
    {
        greet   = spec.greet,
        redName = spec.redName,

        label = spec.label or '装备',

        -- trades in nothing, by definition
        goods = nil,
        trade = nil,

        repair  = optList(spec.repair, {'武器', '衣服', '头盔'}),
        special = spec.special or false,

        repairText  = spec.repairText,
        specialText = spec.specialText,

        today  = spec.today,
        topics = spec.topics,
        extra  = spec.extra,
    }
end

return repairer
