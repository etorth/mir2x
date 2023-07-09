function main()
    uidRemoteCall(getNPCharUID('比奇县_0', '图书管理员_1'), getUID(), getQuestName(),
    [[
        local questUID, questName = ...
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_CHECKACTIVE] = function(uid)
                return uidRemoteCall(uid, [=[ return getQuestState('比奇商会') ]=]) == 'quest_persuade_pharmacist_and_librarian'
            end,

            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>早以前这里就是武林人士聚集的地方，呵呵。</par>
                        <par>你看起来也像个习武之人，来这里有什么事情吗？</par>
                        <par></par>
                        <par><event id="npc_discuss_1">我为了劝您加入比奇商会而来此的！</event></par>
                    </layout>
                ]=])
            end,

            npc_discuss_1 = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>比奇商会？</par>
                        <par>啊！啊！知道了！是那个叫做王大人的创办的商人联合会吧！可是我已经加入了崔大夫创办的传奇商会，还是去别的地方试试吧！</par>
                        <par></par>
                        <par><event id="npc_discuss_2">也就是说无论如何都不行吗？</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_discuss_2 = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>无论如何？那倒也不是。</par>
                        <par>如果你能为我办点事情的话，我也不是不能考虑加入比奇商会的。</par>
                        <par></par>
                        <par><event id="npc_discuss_3">要我帮你做什么事儿才行呢？</event></par>
                    </layout>
                ]=])
            end,

            npc_discuss_3 = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>其实最近我正在编撰记录比奇省地理和历史的书籍。如果想要写好这本书的话必然要从各种各样的人那里收集关于比奇省的资料和信息，可是唯独比奇省的卫士们那里不与我合作啊！</par>
                        <par>不管怎么样你也是武林人士，可能和他们能够有通融的地方，所以这就是我要拜托你的事情！值班卫士反正也不能和别人说话，所以希望你能替我去那儿找那些休班卫士从他们那里收集关于比奇省历史的故事。如果你能做到的话，我会听你的劝告加入比奇商会的。</par>
                        <par></par>
                        <par><event id="npc_accept">也许我可以试试？</event></par>
                    </layout>
                ]=])
            end,

            npc_accept = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>哦？你答应我的请求了？</par>
                        <par>那太好了，我等着你的好消息！</par>
                        <par></par>
                        <par><event id="%s">好的！</event></par>
                    </layout>
                ]=], SYS_EXIT)
                uidRemoteCall(questUID, uid, [=[ setUIDQuestState(..., SYS_ENTER) ]=])
            end,
        })
    ]])

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
            setupNPCQuestBehavior('比奇县_0', '图书管理员_1', uid,
            [[
                return getQuestName()
            ]],
            [[
                local questName = ...
                local questPath = {SYS_EPUID, questName}
                return
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你看起来还没有听到休班卫士的全部故事啊？可以在比奇省内转转就可以找到休班卫士！</par>
                                <par></par>
                                <par><event id="%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                }
            ]])
        end,
    })
end
