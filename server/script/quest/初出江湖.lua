function main()
    uidExecute(getNPCharUID('道馆_1', '士官_1'),
    [[
        local questUID  = %d
        local questName = '%s'
        local questPath = {SYS_EPQST, questName}

        setQuestHandler(questName,
        {
            [SYS_ENTER] = function(uid, value)
                local level = uidExecute(uid, [=[ return getLevel() ]=])
                if level < 4 then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>你好？</par>
                            <par>如果你不了解传奇3，就让贫道简单地给你介绍一下吧。</par>
                            <par></par>
                            <par><event id="npc_what_is_taoist">什么是道士？</event></par>
                            <par><event id="npc_what_is_dogwan">这道馆是什么地方？</event></par>
                            <par><event id="npc_who_are_you">士官是做什么事的人？</event></par>
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
                            <par><event id="npc_what_is_taoist">什么是道士？</event></par>
                            <par><event id="npc_what_is_dogwan">这道馆是什么地方？</event></par>
                            <par><event id="npc_who_are_you">士官是做什么事的人？</event></par>
                            <par><event id="npc_what_is_my_quest">什么是我要做的事情？</event></par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end
            end,

            npc_what_is_dogwan = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>这道馆在很久以前建成至今，有无数的道士都已经修道祭天了！</par>
                        <par>虽然我们无法得知曾经有多少得道成仙的道人，但是现任馆主波观昊道长的道力却是非常高深莫测的！</par>
                        <par></par>
                        <par><event id="%%s">返回</event></par>
                    </layout>
                ]=], SYS_ENTER)
            end,

            npc_what_is_taoist = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>所谓道士就是每天努力洗脱罪过，修身养性，救济人间的人。</par>
                        <par>我们遵从上仙药手的教诲，追求的是进入一个陌生之地潜心修炼以达到长生不老，得道成仙的目的。</par>
                        <par>另外，我们还会帮助与怪物战斗的武士，道士的治愈术和防御术对在与怪物战斗中的武士是非常有用的。</par>
                        <par>主动直接与敌人交手违背了我们上仙药手的教诲。因此在战斗中我们主要采取防御保护的方式。</par>
                        <par></par>
                        <par><event id="%%s">返回</event></par>
                    </layout>
                ]=], SYS_ENTER)
            end,

            npc_who_are_you = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>贫道在这里负责为本派门生传道！</par>
                        <par>施主既然了解本派的门道，就不必太费神啦！</par>
                        <par></par>
                        <par><event id="%%s">返回</event></par>
                    </layout>
                ]=], SYS_ENTER)
            end,

            npc_what_is_my_quest = function(uid, value)
                local level = uidExecute(uid, [=[ return getLevel() ]=])
                if level < 6 then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>现在没有什么合适的任务交给施主做呀！</par>
                            <par>请级别高一点，修练到6级以上再来吧！</par>
                            <par>祝你好运噢！</par>
                            <par></par>
                            <par><event id="%%s">退出</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                else
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>这个，嗯，详细的情况请到收罗杂货的<t color="red">大老板</t>道友那儿打听吧。</par>
                            <par>大老板道友就在道馆内。从这往下走，在右侧可以看到杂货店，进去就可以见到他了。杂货店入口的大概位置在<event id="npc_fly_to_loc" args="{'道馆_1',394,169}" close="1">(394,169)</event>，请参考一下吧！</par>
                            <par></par>
                            <par><event id="npc_accept_quest" close="1">好的！</event></par>
                            <par><event id="%%s">还是算了。</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end
            end,

            npc_fly_to_loc = function(uid, value)
                uidExecute(uid, [=[ spaceMove(%%s) ]=], value)
            end,

            npc_accept_quest = function(uid, value)
                uidExecute(questUID, [=[ setUIDQuestState(%%d, SYS_ENTER) ]=], uid)
            end
        })
    ]], getUID(), getQuestName())

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
            setupNPCQuestBehavior('仓库_1_007', '大老板_1', uid,
            [[
                return getUID(), getQuestName()
            ]],
            [[
                local questUID, questName = ...
                local questPath = {SYS_EPUID, questName}
                return
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>是士官派你来的？</par>
                                <par>嗯，那么先吩咐你做件简单的事儿吧！你能去把这个护身符交给武器库的<t color="red">阿潘</t>道友吗？</par>
                                <par></par>
                                <par><event id="npc_accept_quest">好的！</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_accept_quest = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>阿潘道友还在等着呢！尽快把这个护身符给他带过去吧！</par>
                                <par>从这出去再向右上方一直走就是阿潘道友所在的武器库入口。准确位置在<event id="npc_fly_to_loc" args="{'道馆_1',429,120}" close="1">(429,120)</event>。</par>
                                <par></par>
                                <par><event id="%s">好的！</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                        uidGrant(uid, '道力护身符', 1)
                        uidExecute(questUID, [=[ setUIDQuestState(%d, 'quest_setup_apan') ]=], uid)
                    end,

                    npc_fly_to_loc = function(uid, value)
                        uidExecute(uid, [=[ spaceMove(%s) ]=], value)
                    end,
                }
            ]])
        end,

        quest_setup_apan = function(uid, value)
            -- has accepted the quest to apan
            -- change 大老板's dialog

            uidExecute(getNPCharUID('仓库_1_007', '大老板_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = '%s'
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>出去后向右上方一直走就是武器库的入口。位置在<event id="npc_fly_to_loc" args="{'道馆_1',429,120}" close="1">(429,120)</event>，到阿潘道友后把道力护身符交给他。</par>
                                <par></par>
                                <par><event id="%%s">好的！</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,

                    npc_fly_to_loc = function(uid, value)
                        uidExecute(uid, [=[ spaceMove(%%s) ]=], value)
                    end,
                })
            ]], uid, getUID(), getQuestName())

            uidExecute(getNPCharUID('武器仓库_1_001', '阿潘_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = '%s'
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>嗯，这就是以前我要的护身符啊！要是你不送来的话我就要去催大老板道友了，做得不错啊！</par>
                                <par>送你一把我们店里卖的匕首就当是报答你了，希望能好好使用它哦！</par>
                                <par>再去找找大老板道友吧，或许又有什么事情要派施主去做呢！</par>
                                <par></par>
                                <par><event id="%%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                        uidGrant(uid, '匕首', 1)
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_done_apan') ]=], uid)
                    end,
                })
            ]], uid, getUID(), getQuestName())
        end,

        quest_done_apan = function(uid, value)
            uidExecute(getNPCharUID('武器仓库_1_001', '阿潘_1'),
            [[
                local playerUID = %d
                local questName = '%s'
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>大老板道友呆的杂货店在<event id="npc_fly_to_loc" args="{'道馆_1',394,169}" close="1">(394,169)</event>那儿。快回去看看吧！</par>
                                <par></par>
                                <par><event id="%%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,

                    npc_fly_to_loc = function(uid, value)
                        uidExecute(uid, [=[ spaceMove(%%s) ]=], value)
                    end,
                })
            ]], uid, getQuestName())

            uidExecute(getNPCharUID('仓库_1_007', '大老板_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = '%s'
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>把护身符交给啊潘道友了吧? 那么我会信任施主并且再拜托施主办另外的事儿的！倒没什么特别的，只是在道馆北部的灌木林中最近总有怪物出没，跑出来骚扰百姓，所以需要许多护身符。但是我又有其他的急事要办没时间去弄制护身符所需的鸡血，所以希望你替我收集<t color="green">2</t>瓶<t color="red">鸡血</t>来！</par>
                                <par>嗯, 只要去猎到鸡自然就会有鸡血了，所以不用特别担心！</par>
                                <par></par>
                                <par><event id="%%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, getUID(), getQuestName())

            setUIDQuestState(uid, 'quest_find_chicken_blood')
        end,

        quest_find_chicken_blood = function(uid, value)
            uidExecute(uid,
            [[
                local playerUID = %d
                local  questUID = %d

                addTrigger(SYS_ON_GAINITEM, function(itemID, seqID)
                    if hasItem(getItemID('鸡血'), 0, 2) then
                        postString('已经收集到2瓶鸡血了，快回去找大老板吧！')
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_done_chicken_blood') ]=], playerUID)
                        return true
                    end
                end)
            ]], uid, getUID())
        end,

        quest_done_chicken_blood = function(uid, value)
            uidExecute(getNPCharUID('仓库_1_007', '大老板_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = '%s'
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>多亏了你，我才能及时画完所有的护身符啊！这是辛苦费请你收下！</par>
                                <par>再回去找找士官吧，或许还有别的事情要你做呢！</par>
                                <par></par>
                                <par><event id="%%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                       uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_prepare_to_wang') ]=], uid)
                    end,
                })
            ]], uid, getUID(), getQuestName())
        end,

        quest_prepare_to_wang = function(uid, value)
            if uidExecute(uid, [[ return getLevel() ]]) < 6 then
                uidExecute(getNPCharUID('道馆_1', '士官_1'),
                [[
                    local playerUID = %d
                    local questName = '%s'
                    local questPath = {SYS_EPUID, questName}

                    setUIDQuestHandler(playerUID, questName,
                    {
                        [SYS_ENTER] = function(uid, value)
                            uidPostXML(uid, questPath,
                            [=[
                                <layout>
                                    <par>施主的实力真是大有所增啊！下次交给你做的事可能有点难，等你修炼到了6级再来吧！</par>
                                    <par></par>
                                    <par><event id="%%s">结束</event></par>
                                </layout>
                            ]=], SYS_EXIT)
                        end,
                    })
                ]], uid, getQuestName())

                uidExecute(uid,
                [[
                    local playerUID = %d
                    local  questUID = %d

                    addTrigger(SYS_ON_LEVELUP, function(oldLevel, newLevel)
                        if newLevel >= 6 then
                            postString('你已经升到6级，去找道馆士官咨询新的任务。')
                            uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_prepare_to_wang') ]=], playerUID)
                            return true
                        end
                    end)
                ]], uid, getUID())

            else
                uidExecute(getNPCharUID('道馆_1', '士官_1'),
                [[
                    local playerUID = %d
                    local questUID  = %d
                    local questName = '%s'
                    local questPath = {SYS_EPUID, questName}

                    setUIDQuestHandler(playerUID, questName,
                    {
                        [SYS_ENTER] = function(uid, value)
                            uidPostXML(uid, questPath,
                            [=[
                                <layout>
                                    <par>施主的实力真是大有所增啊！现在你需要摆脱道馆的周围，将视野放到更宽广的地方去才行。从这里通过东南方的通路(516,580)到达比奇县后就可以到达首都比奇省。那个地方是政治、经济、文化的中心地。想修炼成为道士，一定要了解人间苦暖才行，所正好贫道有一样东西要送到比奇省，这件事情就拜托给你吧！</par>
                                    <par></par>
                                    <par><event id="npc_accept_wang">好的</event></par>
                                </layout>
                            ]=])
                        end,

                        npc_accept_wang = function(uid, value)
                            uidPostXML(uid, questPath,
                            [=[
                                <layout>
                                    <par>往比奇省西南方走，就能找到王大人了。详细的位置在比奇县(389,396)。找到他，然后把这本书转交给他，他自然会支付给你报酬。通过比奇省的东南部通路(516,580)到达比奇县后，就可以找到比奇省了。</par>
                                    <par>对了，别忘了把这本武功秘笈给道士高手清明子。这位高手能给像施主这样的道士入门者传授一些基本的魔法，施主一定会有所收获的。清明子就在本馆内。从本馆左边往上走就可以找到了。准确位置在(429,96)。</par>
                                    <par></par>
                                    <par><event id="%%s">结束</event></par>
                                </layout>
                            ]=], SYS_EXIT)

                            uidGrant(uid, '古籍'  , 1)
                            uidGrant(uid, '治愈术', 1)
                            uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_accept_wang_book') ]=], uid)
                        end,
                    })
                ]], uid, getUID(), getQuestName())
            end
        end,

        quest_accept_wang_book = function(uid, value)
            uidExecute(getNPCharUID('道馆_1', '士官_1'),
            [[
                local playerUID = %d
                local questName = '%s'
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>去见完比奇省的王大人，还要请您去拜访本馆的清明子！王大人在比奇省的西南部就可以找到.准确位置是比奇县(389,396)。通过比奇省的东南通路(516,580)到达比奇县后，便可以找到比奇省了。清明子就在本馆内。从本馆左边往上走就可以找到了。准确位置在(429,96)。</par>
                                <par></par>
                                <par><event id="%%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, getQuestName())

            uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = '%s'
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>来此有何贵干啊？</par>
                                <par></par>
                                <par><event id="npc_ask_who_are_you">请问阁下是王大人吗？</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_ask_who_are_you = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>难道你是受道馆的士官之托而来的人？</par>
                                <par></par>
                                <par><event id="npc_grant_bonus">正是在下！</event></par>
                                <par><event id="npc_deny">认错人了。</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_grant_bonus = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>我就是你要找的王某人，咳嗯。</par>
                                <par>哦？带来了道馆的士官送给我的东西吗？啊哈，这就是我以前想要的古书。远道而来，辛苦你啦！</par>
                                <par>这是给你的辛苦费，请收下吧！对了，或许以后还需要你的帮助呢，下次再来吧！</par>
                                <par></par>
                                <par><event id="%%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                        uidGrantGold(uid, 1000)
                        uidGrant(uid, '青铜头盔', 1)
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_done_wang_book') ]=], uid)
                    end,

                    npc_deny = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>喔...</par>
                                <par>出发有一会儿了，也该到了啊？</par>
                                <par>不知是不是在路上哪儿遇到了什么麻烦？</par>
                                <par></par>
                                <par><event id="%%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, getUID(), getQuestName())
        end,

        quest_done_wang_book = function(uid, value)
            uidExecute(getNPCharUID('比奇县_0', '王大人_1'),
            [[
                local playerUID = %d
                local questName = '%s'
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>暂时没有什么需要你帮忙的。</par>
                                <par></par>
                                <par><event id="%%s">退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, getQuestName())
            setUIDQuestState(uid, SYS_DONE)
        end,
    })
end
