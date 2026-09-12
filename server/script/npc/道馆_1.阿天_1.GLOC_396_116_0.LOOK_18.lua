local dialog = require('include.dialog')
local invop = require('npc.include.invop')

local secureGold = 20
local secureTypeList = {'恢复药水', '武器', '药粉', '传送卷轴', '技能书', '护身符', '头盔', '戒指', '手镯', '项链', '衣服', '鞋'}

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        dialog.post(uid,
        {
            string.format('这里是沙巴克城<t color="red">%s</t>行会的领地。', getSubukGuildName()),
            '贫道是仓库管理员，需要的话，可以把东西暂时寄存在我这里。',
        },
        {
            dialog.link('npc_goto_secure', '寄存', {suffix = '物品'}),
            dialog.link('npc_goto_get_back', '取回', {suffix = '物品'}),
            dialog.link('npc_goto_set_password', '设置', {suffix = '仓库密码'}),
            dialog.link('npc_goto_daily_quest', '对今日的任务进行了解'),
            dialog.link(SYS_EXIT, '关闭'),
        })
    end,

    ["npc_goto_secure"] = function(uid, value)
        dialog.post(uid, '你想寄存什么东西？',
        dialog.link(SYS_ENTER, '前一步'))
        invop.uidStartSecure(uid, "npc_goto_secure_query", "npc_goto_secure_commit", secureTypeList)
    end,

    ["npc_goto_secure_query"] = function(uid, value)
        local itemID, seqID = invop.parseItemString(value)

        dialog.post(uid, string.format('这个我可以帮你保存，但要收你<t color="red">%d金币</t>保管费。', secureGold),
        dialog.link(SYS_ENTER, '前一步'))

        invop.postSecureCost(uid, itemID, seqID, secureGold)
        invop.uidStartSecure(uid, "npc_goto_secure_query", "npc_goto_secure_commit", secureTypeList)
    end,

    ["npc_goto_secure_commit"] = function(uid, value)
        local itemID, seqID = invop.parseItemString(value)

        if uidQueryGold(uid) < secureGold then
            dialog.post(uid, string.format('保管费要%d金币，你带的钱不够。', secureGold),
            dialog.link(SYS_ENTER, '前一步'))

        elseif uidRemoveGold(uid, secureGold) then
            uidSecureItem(uid, itemID, seqID)
            dialog.post(uid, '已经放好了。',
            dialog.link(SYS_ENTER, '前一步'))
        end

        invop.uidStartSecure(uid, "npc_goto_secure_query", "npc_goto_secure_commit", secureTypeList)
    end,

    ["npc_goto_get_back"] = function(uid, value)
        dialog.post(uid, '你想找什么，看了目录以后再决定吧。',
        dialog.link(SYS_ENTER, '前一步'))
        uidShowSecuredItemList(uid)
    end,

    ["npc_goto_set_password"] = function(uid, value)
        dialog.post(uid, '请设置你的仓库密码。',
        dialog.link(SYS_ENTER, '前一步'))
        invop.postStartInput(uid, '<layout><par>请输入密码</par></layout>', 'npc_goto_get_set_password_1', true)
    end,

    ["npc_goto_get_set_password_1"] = function(uid, value)
        uidRemoteCall(uid, getNPCFullName(), value,
        [[
            local npcName, password = ...
            if not _G.RSVD_NAME_firstPasswordInput then
                _G.RSVD_NAME_firstPasswordInput = {}
            end
            _G.RSVD_NAME_firstPasswordInput[npcName] = password
        ]])

        dialog.post(uid, '请再次输入密码确认。',
        dialog.link(value, '关闭', {close = true}))
        invop.postStartInput(uid, '<layout><par>请确认密码</par></layout>', 'npc_goto_get_set_password_2', true)
    end,

    ["npc_goto_get_set_password_2"] = function(uid, value)
        local firstInput = uidRemoteCall(uid, getNPCFullName(),
        [[
            local npcName = ...
            return _G.RSVD_NAME_firstPasswordInput[npcName]
        ]])

        if firstInput == value then
            dialog.post(uid, '设置密码成功！',
            {
                dialog.link(SYS_ENTER, '前一步'),
                dialog.link(SYS_EXIT, '关闭'),
            })
        else
            dialog.post(uid, '两次密码输入不一致，设置密码失败。',
            {
                dialog.link('npc_goto_set_password', '设置密码'),
                dialog.link(SYS_EXIT, '关闭'),
            })
        end

        uidRemoteCall(uid, getNPCFullName(),
        [[
            local npcName = ...
            _G.RSVD_NAME_firstPasswordInput[npcName] = nil
        ]])
    end,

    ["npc_goto_daily_quest"] = function(uid, value)
        dialog.post(uid, '今天没事情可拜托你了。',
        dialog.link(SYS_EXIT, '关闭'))
    end,
})
