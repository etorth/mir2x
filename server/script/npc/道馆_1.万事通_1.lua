setNPCLook(2)
setNPCGLoc(398, 127)

local dq = require('npc.include.dailyquest')
processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>江湖上的朋友都叫我万拍子，不是我吹，你不了解的任务我都可以给你解答。你有什么想问的吗？</par>
                <par></par>
                <par><event id="npc_goto_1">询问一般的任务</event></par>
                <par><event id="npc_goto_2">对今日的任务进行了解</event></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    ["npc_goto_1"] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>来吧，你有什么任务？</par>
                <par><event id="npc_goto_3">乞丐任务</event>，<event id="npc_goto_4">苍蝇拍任务</event>，<event id="npc_goto_5">石罂粟任务</event></par>
                <par></par>

                <par>（等级 9）</par>
                <par><event id="npc_goto_6">王大人任务</event>，<event id="npc_goto_7">比奇省任务</event>，<event id="npc_goto_8">药剂师任务</event></par>
                <par></par>

                <par>（等级 11）</par>
                <par><event id="npc_goto_9">轻型盔甲任务</event>，<event id="npc_goto_10">半兽人任务</event></par>
                <par></par>

                <par>（等级 16）</par>
                <par><event id="npc_goto_11">被盗的灵魂任务</event>，<event id="npc_goto_12">千年毒蛇任务</event>，<event id="npc_goto_13">堕落道士任务</event></par>
                <par></par>

                <par>（等级 20）</par>
                <par><event id="npc_goto_14">沃玛教主任务</event></par>
                <par></par>

                <par><event id="%s">前一步</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], SYS_NPCINIT, SYS_NPCDONE)
    end,

    ["npc_goto_2"] = function(uid, value)
        dq.setQuest(0, uid, value)
    end
}
