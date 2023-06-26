function main()
    uidExecute(getNPCharUID('道馆_1', '士官_1'),
    [[
        local questUID  = %d
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_ENTER] = function(uid, value)
                if uidExecute(uid, [=[ return getLevel() ]=]) < 4 then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你好？</par>
                            <par>如果你不了解传奇3，就让贫道简单地给你介绍一下吧。</par>
                            <par></par>
                            <par><event id="quest_what_is_taoist">什么是道士？</event></par>
                            <par><event id="quest_what_is_dogwan">这道馆是什么地方？</event></par>
                            <par><event id="quest_who_are_you">士官是做什么事的人？</event></par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                else
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>贫道会教你做一些简单但值得去做的事情，一边做一边慢慢的熟悉一下道馆内的事情！</par>
                            <par>这之前如果有什么不明白的地方，尽管来问吧！</par>
                            <par></par>
                            <par><event id="quest_what_is_taoist">什么是道士？</event></par>
                            <par><event id="quest_what_is_dogwan">这道馆是什么地方？</event></par>
                            <par><event id="quest_who_are_you">士官是做什么事的人？</event></par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end
            end,

            quest_what_is_dogwan = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>这道馆在很久以前建成至今，有无数的道士都已经修道祭天了！</par>
                        <par>虽然我们无法得知曾经有多少得道成仙的道人，但是现任馆主波观昊道长的道力却是非常高深莫测的！</par>
                        <par><event id="%%s">返回</event></par>
                    </layout>
                ]=], SYS_ENTER)
            end,

            quest_what_is_taoist = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>所谓道士就是每天努力洗脱罪过，修身养性，救济人间的人。</par>
                        <par>我们遵从上仙药手的教诲，追求的是进入一个陌生之地潜心修炼以达到长生不老，得道成仙的目的。</par>
                        <par>另外，我们还会帮助与怪物战斗的武士，道士的治愈术和防御术对在与怪物战斗中的武士是非常有用的。</par>
                        <par>主动直接与敌人交手违背了我们上仙药手的教诲。因此在战斗中我们主要采取防御保护的方式。</par>
                        <par><event id="%%s">返回</event></par>
                    </layout>
                ]=], SYS_ENTER)
            end,

            quest_who_are_you = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>贫道在这里负责为本派门生传道！</par>
                        <par>施主既然了解本派的门道，就不必太费神啦！</par>
                        <par><event id="%%s">返回</event></par>
                    </layout>
                ]=], SYS_ENTER)
            end,
        })
    ]], getUID(), getQuestName())

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
        end,
    })
end
