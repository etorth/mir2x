local invop = require('npc.include.invop')

local weaponTypeList = {'武器'}

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            uidPostXML(uid,
            [[
                <layout>
                    <par>我不想和你这种坏人做生意。</par>
                    <par></par>

                    <par><event id="%s" close="1">关闭</event></par>
                </layout>
            ]], SYS_EXIT)
        else
            uidPostXML(uid,
            [[
                <layout>
                    <par>欢迎光临，有什么事吗？</par>
                    <par></par>

                    <par><event id="npc_goto_1">修理</event>武器</par>
                    <par><event id="npc_goto_2">特殊修理</event>武器</par>
                    <par><event id="npc_goto_3">对今日的任务进行了解</event></par>
                    <par><event id="%s" close="1">关闭</event></par>
                </layout>
            ]], getSubukGuildName(), SYS_EXIT)
        end
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>请把要修理的武器拿来。</par>
                <par>普通修理会有概率损失武器的持久上限。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartRepair(uid, "npc_goto_query_repair", "npc_goto_commit_repair", weaponTypeList)
    end,

    ["npc_goto_2"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>特殊修理不会损失持久上限，价钱贵些。</par>
                <par>请把要修理的武器拿来。</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
            </layout>
        ]], SYS_ENTER)
        invop.uidStartRepair(uid, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_query_repair"] = function(uid, value)
        invop.postQueryRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", weaponTypeList)
    end,

    ["npc_goto_commit_repair"] = function(uid, value)
        invop.postCommitRepair(uid, value, "npc_goto_query_repair", "npc_goto_commit_repair", weaponTypeList)
    end,

    ["npc_goto_query_special_repair"] = function(uid, value)
        invop.postQuerySpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_commit_special_repair"] = function(uid, value)
        invop.postCommitSpecialRepair(uid, value, "npc_goto_query_special_repair", "npc_goto_commit_special_repair", weaponTypeList)
    end,

    ["npc_goto_3"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>今天没事情可拜托你了。</par>
                <par></par>

                <par><event id="%s" close="1">关闭</event></par>
            </layout>
        ]], SYS_EXIT)
    end,
})
