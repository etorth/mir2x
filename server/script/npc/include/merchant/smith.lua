local merchant = require('npc.include.merchant')

-- 铁匠 / 武器商, legacy Market_Def/02Weapon_*.txt (22 of them)
--
-- the only trade that offers 特殊修理 — 18 of the 22 do — and the only one with a word to say
-- about where good weapons come from (询问 -> @qweapon). two of them also take a bound weapon
-- off your hand (请求把剑从手分离开 -> @remove_sword), which is how a player gets rid of the
-- 攻杀铁剑 that 攻杀剑术任务 lends out
--
--     local smith = require('npc.include.merchant.smith')
--     smith.setSmith
--     {
--         greet = {'来点什么？我这儿的兵器都是好东西。'},
--         goods = {'青铜斧', '八荒', '凌风'},
--         special = false,                  -- 4 of the 22 do not offer 特殊修理
--         qweapon = true,                   -- offer the 询问 topic with the standard answer
--         removeSword = true,               -- offer 请求把剑从手分离开
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local smith = {}

-- @NPC_QWeapon, the same answer in every shop that has it
smith.QWEAPON =
{
    '商店里出售的武器基本上都差不多，但从怪物那里抢来的武器则根据不同的情况，可能具有超凡的能力。如果你把那类武器拿到商店里来卖，我可以出个好价钱。还有，武器的价格随着种类的不同而不同，但基本上持久性越强，价格就越高。',
}

-- @NPC_Remove_Sword and @NPC_Remove_Sword_Else. legacy checks what is in your hand first and
-- only claims to have unstuck the sword when there is one — three shops carry this and all three
-- name the same two swords, 攻杀铁剑 from 攻杀剑术任务 and 焱火剑 from 大火球任务, which are
-- the items SDItem::EA_BIND is actually used on
--
-- @NPC_Remove_Sword_1 is a byte-for-byte duplicate of @NPC_Remove_Sword in every one of the
-- three, so there is only one line to say
smith.REMOVE_SWORD =
{
    '你是怎么会让手粘在剑上呢 。。。',
    '你看看现在是不是已经摘下来了。。。 这种没用的剑我来替你保管吧。。。',
}

smith.REMOVE_SWORD_ELSE =
{
    '你的手没有粘在剑上。。。',
    '听说<t color="red">攻杀铁剑</t>和 <t color="red">焱火剑</t>一旦到手上就摘不下来。',
}

-- the swords that stick. a smith will only take one of these off, so a player holding an
-- ordinary weapon does not get it confiscated by clicking the wrong menu entry
smith.BOUND_SWORD = {'攻杀铁剑', '焱火剑'}

function smith.setSmith(spec)
    assertType(spec, 'table')

    local topics = {}
    local repair = optList(spec.repair, {'武器'})
    local special = (spec.special ~= false)

    -- 询问, about what weapons are worth
    if spec.qweapon then
        table.insert(topics, {id = 'npc_qweapon', label = '询问', suffix = '关于武器的事', text = spec.qweaponText or smith.QWEAPON})
    end

    -- 请求把剑从手分离开. SDItem::EA_BIND stops the player taking it off, and removeWearItem is
    -- the one path that ignores that
    if spec.removeSword then
        local stuckText = spec.removeSwordText or smith.REMOVE_SWORD
        local elseText  = spec.removeSwordElseText or smith.REMOVE_SWORD_ELSE

        table.insert(topics, {id = 'npc_remove_sword', label = '请求把剑从手分离开',
            handler = function(uid, value)
                local held = server.player.getWLItem(uid, WLG_WEAPON)
                local stuck = false

                if held then
                    for _, name in ipairs(smith.BOUND_SWORD) do
                        if held.itemID == getItemID(name) then
                            stuck = true
                            break
                        end
                    end
                end

                local par = {}
                for _, line in ipairs(stuck and stuckText or elseText) do
                    table.insert(par, string.format('<par>%s</par>', line))
                end

                table.insert(par, '<par></par>')
                table.insert(par, string.format('<par><event id="%s">前一步</event></par>', SYS_ENTER))
                uidPostXML(uid, string.format('<layout>%s</layout>', table.concat(par)))

                if stuck then
                    server.player.removeWearItem(uid, WLG_WEAPON)
                end
            end})
    end

    for _, t in ipairs(spec.topics or {}) do
        table.insert(topics, t)
    end

    merchant.setMerchant
    {
        greet   = spec.greet,
        redName = spec.redName,

        label = spec.label or '武器',
        goods = spec.goods,
        trade = optList(spec.trade, {'武器'}),
        price = spec.price,

        repair        = repair,
        special       = special,
        specialRepair = special and (spec.specialRepair or repair or {'武器'}) or nil,

        -- @NPC_Pre_Repair, the line 6 of them share
        repairText = spec.repairText or {'请把要修理的武器放上去。'},

        buyText     = spec.buyText,
        sellText    = spec.sellText,
        specialText = spec.specialText,
        repairDone  = spec.repairDone,
        today       = spec.today,
        topics      = topics,
        extra       = spec.extra,
    }
end

return smith
