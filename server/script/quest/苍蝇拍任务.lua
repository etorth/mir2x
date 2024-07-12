_G.minQuestLevel = 5

setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, '按照比奇城肉店金氏的嘱托，去找杂货商人吧。'}
        setupNPCQuestBehavior('比奇县_0', '金氏_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>苍蝇拍还没做好吗？</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '杂货商_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>最近天气异常的炎热，苍蝇拍的库存货都全部卖光了！...你能帮我找些做苍蝇拍的材料来吗？</par>
                            <par><event id="npc_accept">好的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_accept = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>找到苍蝇拍的材料的话我就会帮你做苍蝇拍！苍蝇拍所需的材料是牛毛和竹棍。牛毛可以从牛身上弄到，竹棍或许能从钉耙猫那儿弄到！</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    qstapi.setState(questUID, {uid=uid, state='quest_start_collection'})
                end,
            }
        ]])

        setupNPCQuestBehavior('道馆_1', '万事通_1', uid,
        [[
            return
            {
                [SYS_HIDE] = true,
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid,
                    [=[
                        <layout>
                            <par>按照金氏的吩咐，去杂货商那儿看看吧！杂货商(450:413)就在金氏店铺右边摆小摊呢！</par>
                            <par><event id="%s">返回</event></par>
                        </layout>
                    ]=], SYS_ENTER)
                end,
            }
        ]])
    end,

    quest_start_collection = function(uid, args)
        setQuestDesp{uid=uid, '杂货商希望你帮他找到制作苍蝇拍的材料竹棍和牛毛，这些东西可以从钉耙猫和牛身上找到。'}
        uidRemoteCall(uid, uid, getUID(),
        [[
            local playerUID, questUID = ...
            addTrigger(SYS_ON_GAINITEM, function(itemID, seqID)
                local has_cowHair    = hasItem(getItemID('牛毛'), 0, 1)
                local has_bambooPole = hasItem(getItemID('竹棍'), 0, 1)

                if has_cowHair and has_bambooPole then
                    postString('已经收集到牛毛和竹棍了，快回去找杂货商吧！')
                    uidRemoteCall(questUID, playerUID,
                    [=[
                        local playerUID = ...
                        setQuestDesp{uid=playerUID, '已经收集到制作苍蝇拍所需要的竹棍和牛毛，把他们交给杂货商吧。'}
                        setQuestState{uid=playerUID, state='quest_complete_collection'}
                    ]=])
                    return true
                end

                if has_cowHair then
                    postString('已经收集到牛毛了，快去猎杀钉耙猫获得竹棍吧！')
                    uidRemoteCall(questUID, playerUID,
                    [=[
                        local playerUID = ...
                        setQuestDesp{uid=playerUID, '获得了牛毛，再去找竹棍，一起送给杂货商。'}
                    ]=])
                end

                if has_bambooPole then
                    postString('已经收集到竹棍了，快去猎杀牛获得牛毛吧！')
                    uidRemoteCall(questUID, playerUID,
                    [=[
                        local playerUID = ...
                        setQuestDesp{uid=playerUID, '获得了竹棍，再去找牛毛，一起送给杂货商。'}
                    ]=])
                end
            end)
        ]])
    end,

    quest_complete_collection = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '杂货商_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦！材料全部找到了啊！请稍等一下...</par>
                        </layout>
                    ]=])

                    pause(500)

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦！材料全部找到了啊！请稍等一下...</par>
                            <par>给你！</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    plyapi.addItem(uid, getItemID('苍蝇拍'), 1)
                    qstapi.setState(questUID, {uid=uid, state='quest_get_fly_swatter'})
                end,
            }
        ]])
    end,

    quest_get_fly_swatter = function(uid, args)
        setQuestDesp{uid=uid, '把材料转给杂货商后，获得了苍蝇拍，把苍蝇拍给肉店金氏送去吧。'}
        setupNPCQuestBehavior('比奇县_0', '金氏_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    if plyapi.hasItem(uid, getItemID('苍蝇拍'), 0, 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>噢...真是太感谢了，现在可以对付这些该死的苍蝇了！</par>
                                <par>这是一些对你帮助的奖励，请你不要客气。</par>
                                <par><event id="%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                        plyapi.removeItem(uid, getItemID('苍蝇拍'), 0, 1)
                        plyapi.   addItem(uid, getItemID('蝉翼刀'), 1)

                        qstapi.setState(questUID, {uid=uid, state=SYS_DONE})
                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你给我带的苍蝇拍呢？</par>
                                <par><event id="%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '金氏_1'), getUID(), getQuestName(), minQuestLevel,
[[
    local questUID, questName, minQuestLevel = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            if plyapi.getLevel(uid) < minQuestLevel then
                return false
            end

            return qstapi.getState(questUID, {uid=uid, fsm=SYS_QSTFSM}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>哎！这些该死的苍蝇，害得我没法儿做生意。要去重新买一个苍蝇拍吧，偏偏这个时候苍蝇拍材料没有了，啧...</par>
                    <par>啊！正好，你去杂货商那儿帮他找些苍蝇拍的材料，然后把做好的苍蝇拍带过来行吗？</par>
                    <par><event id="npc_accept">您是说去杂货店吗？我去一趟吧！</event></par>
                    <par><event id="npc_refuse">我有点忙...</event></par>
                </layout>
            ]=])
        end,

        npc_accept = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那就拜托你了！杂货店就是在右边能看到的那个地方。准确位置是<t color="red">450,413</t>。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            qstapi.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,

        npc_refuse = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>啊，这样啊...一小会儿就行的...唉，真是没辙了！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,
    })
]])
