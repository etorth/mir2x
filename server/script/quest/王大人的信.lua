uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
[[
    return setQuestHandler('%s', {
        [SYS_CHECKACTIVE] = function(uid)
            return true
        end,

        ['npc_goto_1'] = function(uid, value)
        end
    })
]], getQuestName())
