function main()
    uidExecute(getNPCharUID('道馆_1', '士官_1'),
    [[
        local questUID  = %d
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你好？</par>
                        <par>如果你不了解传奇3，就让贫道简单地给你介绍一下吧。</par>
                        <par><event id="npc_query_quests">与士官探讨自己可做的事</event></par>
                        <par><event id="%%s">退出</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,
        })
    ]], getUID(), getQuestName())

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
            uidExecute(getNPCharUID('道馆_1', '士官_1'),
            [[
                uidPostXML(%d,
                [=[
                    <layout>
                        <par>多谢少侠出手相助！最近城外钉耙猫肆虐，请少侠出城斩杀一只<t color="red">钉耙猫</t>，事后我万拍子有礼物奉上！</par>
                        <par></par>
                        <par><event id="%%s">好的</event></par>
                    </layout>
                ]=], SYS_EXIT)

            ]], uid)
        end,
    })
end
