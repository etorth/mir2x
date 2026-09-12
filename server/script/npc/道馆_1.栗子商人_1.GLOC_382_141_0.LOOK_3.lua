local dialog = require('include.dialog')
local function onSellChestnut(uid, chestnutName, currTagName, lastTagName)
    local priceTable = {
        ['金色栗子'] = 10000,
        ['银色栗子'] =  5000,
        ['铜色栗子'] =  2000,
        ['褐色栗子'] =  1000,
    }

    if priceTable[chestnutName] == nil then
        fatalPrintf('Invalid chestnut name: %s', tostring(chestnutName))
    end

    if uidRemove(uid, chestnutName, 1) then
        uidGrantGold(uid, priceTable[chestnutName])
        dialog.post(uid, string.format('请收下<t color="red">%d</t>金币', priceTable[chestnutName]),
        {
            dialog.link(currTagName, string.format('还卖其它的%s', chestnutName), {args = chestnutName}),
            dialog.link(lastTagName, '前一步'),
            dialog.link(SYS_EXIT, '关闭'),
        })
    else
        dialog.post(uid, string.format('哼，你没带%s来，快去找！', chestnutName),
        dialog.link(SYS_EXIT, '关闭'))
    end
end

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            dialog.post(uid, '跟你这种人我无话可说。',
            dialog.link(SYS_EXIT, '关闭'))
        else
            dialog.post(uid,
            {
                '欢迎光临，对，我就是买栗子的。 如果你能给我找来那些味道又好，营养又好的栗子，我就送你一份大礼。',
                '你有栗子吗？',
            },
            {
                dialog.link('npc_goto_trade', '带来了金色栗子，你要卖吗？', {args = '金色栗子'}),
                dialog.link('npc_goto_trade', '带来了银色栗子，你要卖吗？', {args = '银色栗子'}),
                dialog.link('npc_goto_trade', '带来了铜色栗子，你要卖吗？', {args = '铜色栗子'}),
                dialog.link('npc_goto_trade', '带来了褐色栗子，你要卖吗？', {args = '褐色栗子'}),
                dialog.link(SYS_EXIT, '马上去给你找'),
            })
        end
    end,

    ["npc_goto_trade"] = function(uid, value)
        onSellChestnut(uid, value, "npc_goto_trade", SYS_ENTER)
    end,
})
