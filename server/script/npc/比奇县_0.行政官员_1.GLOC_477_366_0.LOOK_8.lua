-- converted from Envir/QuestDiary/CastleWar/Execution.txt, called by Market_Def/18Execution_Bichon-0.txt
--
-- only the guild creation part for now
-- 申请行会战争 (@guildwar), 询问行会战争 (@guildwarexp) and 变更自己的行会旗帜 (@UserChangeFlag) wait for guild war
-- the admin menu (@admin_menu, @Bichon_Day, @Bichon_Dark) is not converted
--
-- legacy names the cost 沃玛号角, which is 沃玛号角_0 in the item list, 沃玛号角_1 is another item
-- legacy @buildguildexp calls it 沃玛教主号角, kept as is
-- legacy @buildguildexp also says 禹魔神殿, the other transliteration of 우마, written as 沃玛神殿 here

local dialog = require('include.dialog')
local invop = require('npc.include.invop')

local buildGold = 1000000
local buildItem = '沃玛号角_0'

local function postInGuild(uid)
    dialog.post(uid, '你已经加入了行会所以不可以成立行会。',
    dialog.link(SYS_ENTER, '前一步'))
end

local function postNoItem(uid)
    dialog.post(uid, '你没有沃玛号角所以不能成立行会。',
    dialog.link(SYS_EXIT, '结束'))
end

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        dialog.post(uid, '这里是比奇城城堡，我是行政官员。',
        {
            dialog.link('npc_goto_make_guild', '申请创建行会'),
            dialog.link('npc_goto_build_guild_exp', '询问行会创建方法'),
        })
    end,

    ['npc_goto_make_guild'] = function(uid, value)
        if server.player.getGuildName(uid) ~= nil then
            postInGuild(uid)

        elseif server.player.getGold(uid) < buildGold then
            dialog.post(uid, '你的钱不够成立行会。',
            dialog.link(SYS_ENTER, '前一步'))

        elseif server.player.hasItem(uid, buildItem, 1) then
            dialog.post(uid, string.format('你确定要成立行会吗？点击确定后你将会失去<t color="red">%d万金币</t>和<t color="red">沃玛号角</t>。', buildGold // 10000),
            {
                dialog.link('npc_goto_build_guild', '确定'),
                dialog.link(SYS_EXIT, '取消'),
            })

        else
            postNoItem(uid)
        end
    end,

    ['npc_goto_build_guild'] = function(uid, value)
        dialog.post(uid, '请输入行会的名字。',
        dialog.link(SYS_ENTER, '前一步'))
        invop.postStartInput(uid, '<layout><par>请输入行会的名字</par></layout>', 'npc_goto_build_guild_commit', false)
    end,

    -- legacy refunds the cost when BuildGuild fails (@MakeGuildOK~), createGuild never takes it in that case
    -- legacy says 没有沃玛号角 when either the horn or the gold is gone by now
    ['npc_goto_build_guild_commit'] = function(uid, value)
        local result = server.player.createGuild(uid, value, buildGold, buildItem)

        if result == GUILDERR_NONE then
            dialog.post(uid, string.format('%s 您成立了 %s 的行会。', uidQueryName(uid), value),
            dialog.link(SYS_EXIT, '结束'))

        elseif result == GUILDERR_INGUILD then
            postInGuild(uid)

        elseif result == GUILDERR_NOGOLD or result == GUILDERR_NOITEM then
            postNoItem(uid)

        else
            dialog.post(uid, '行会成立失败了。',
            dialog.link(SYS_EXIT, '结束'))
        end
    end,

    ['npc_goto_build_guild_exp'] = function(uid, value)
        dialog.post(uid,
        {
            '要想创建行会，首先要证明你是否具备那种资格。',
            '资格证明需要生活在沃玛丛林里沃玛神殿深处的<t color="red">沃玛教主号角</t>和<t color="red">100万金币</t>的行会设立基金。',
        },
        dialog.link(SYS_ENTER, '前一步'))
    end,
})
