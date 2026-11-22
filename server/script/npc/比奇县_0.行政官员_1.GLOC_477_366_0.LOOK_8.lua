-- converted from Envir/QuestDiary/CastleWar/Execution.txt, called by Market_Def/18Execution_Bichon-0.txt
--
-- 变更自己的行会旗帜 (@UserChangeFlag) waits for guild flags
-- the admin menu (@admin_menu, @Bichon_Day, @Bichon_Dark) is not converted
-- legacy @donate (还没有收到捐款) is linked from nowhere, not converted
--
-- legacy names the cost 沃玛号角, which is 沃玛号角_0 in the item list, 沃玛号角_1 is another item
-- legacy @buildguildexp calls it 沃玛教主号角, kept as is
-- legacy @buildguildexp also says 禹魔神殿, the other transliteration of 우마, written as 沃玛神殿 here

local dialog = require('include.dialog')
local invop = require('npc.include.invop')

local buildGold = 1000000
local buildItem = '沃玛号角_0'

local warGold = 30000

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
            dialog.link('npc_goto_guild_war', '申请行会战争'),
            dialog.link('npc_goto_guild_war_exp', '询问行会战争'),
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

    ['npc_goto_guild_war'] = function(uid, value)
        dialog.post(uid,
        {
            '请说出要交战的行会名称。',
            string.format('进行行会战争的合法认证需要%d万金币。', warGold // 10000),
        },
        {
            dialog.link('npc_goto_confirm_guild_war', '进行行会战争'),
            dialog.link(SYS_ENTER, '前一步'),
        })
    end,

    -- legacy checks the leader only when the war gets declared, checked first here
    ['npc_goto_confirm_guild_war'] = function(uid, value)
        if not server.player.isGuildLeader(uid) then
            dialog.post(uid, '只有行会的创建者才能申请行会战争。',
            dialog.link(SYS_ENTER, '前一步'))

        elseif server.player.getGold(uid) < warGold then
            dialog.post(uid, '请确认一下自己所带的金币。',
            dialog.link(SYS_EXIT, '结束'))

        else
            dialog.post(uid, '请输入参战的行会名字。',
            dialog.link(SYS_ENTER, '前一步'))
            invop.postStartInput(uid, '<layout><par>请输入参战的行会名字</par></layout>', 'npc_goto_guild_war_commit', false)
        end
    end,

    -- legacy takes the gold before the name is typed and refunds it on failure (@GuildMakeWarOK~)
    -- declareGuildWar never takes it in that case
    ['npc_goto_guild_war_commit'] = function(uid, value)
        local result = server.player.declareGuildWar(uid, value, warGold)

        if result == GUILDERR_NONE then
            dialog.post(uid, string.format('你的 %s 行会和 敌对 %s 行会申请了行会大战。', server.player.getGuildName(uid), value),
            dialog.link(SYS_EXIT, '结束'))

        elseif result == GUILDERR_NOGUILD or result == GUILDERR_NOTLEADER then
            dialog.post(uid, '只有行会的创建者才能申请行会战争。',
            dialog.link(SYS_EXIT, '结束'))

        elseif result == GUILDERR_NOGOLD then
            dialog.post(uid, '请确认一下自己所带的金币。',
            dialog.link(SYS_EXIT, '结束'))

        else
            dialog.post(uid, '敌对行会输入错误。',
            dialog.link(SYS_EXIT, '结束'))
        end
    end,

    ['npc_goto_guild_war_exp'] = function(uid, value)
        dialog.post(uid,
        {
            dialog.link('npc_goto_guild_war_exp_color', '行会战争') .. '是指进行合法的申请后展开的战争。',
            '现在存在的行会很多，因此行会之间的战争也不断，国家承认' .. dialog.link('npc_goto_guild_war_exp_rule', '合法的') .. '战争。',
            string.format('每%s一次，可以进行1个小时的战争，申请时需要%d万金币的申请资金。', dialog.link('npc_goto_guild_war_exp_leader', '申请'), warGold // 10000),
        },
        dialog.link(SYS_ENTER, '前一步'))
    end,

    ['npc_goto_guild_war_exp_color'] = function(uid, value)
        dialog.post(uid, '申请了行会战争以后，与自己同一行会的成员的ID变为蓝色，对方行会成员的ID变为朱红色。在行会战争进行中登陆时，交谈窗口会出现“正在和某行会进行战争”的字样。即使杀死对方行会的人也不会变红。',
        dialog.link('npc_goto_guild_war_exp', '前一步'))
    end,

    ['npc_goto_guild_war_exp_rule'] = function(uid, value)
        dialog.post(uid, '行会战争不允许在村子里展开，只能在可以对练的场地上(商店或其它的建筑物里)或开离村子一定距离以上的地方进行。战争时间结束后，所有人复归原样。即使在战争中，原本被划上红道的ID仍然显示为红色。',
        dialog.link('npc_goto_guild_war_exp', '前一步'))
    end,

    ['npc_goto_guild_war_exp_leader'] = function(uid, value)
        dialog.post(uid, '只有行会的创建者才能申请行会战争。',
        dialog.link('npc_goto_guild_war_exp', '前一步'))
    end,
})
