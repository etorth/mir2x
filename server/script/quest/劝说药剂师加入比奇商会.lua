function main()
    uidExecute(getNPCharUID('比奇县_0', '药剂师_1'),
    [[
        local questUID  = %d
        local questName = %s
        local questPath = {SYS_EPQST, questName}

        return setQuestHandler(questName,
        {
            [SYS_CHECKACTIVE] = function(uid)
                return uidExecute(uid, [=[ return getQuestState('比奇商会') ]=]) == 'quest_persuade_pharmacist_and_librarian'
            end,

            [SYS_ENTER] = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我现在特别忙，你有什么事儿吗？</par>
                        <par></par>
                        <par><event id="npc_discuss_1">我来劝说你加入比奇商会！</event></par>
                    </layout>
                ]=])
            end,

            npc_discuss_1 = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>让我加入王大人的比奇商会？</par>
                        <par>你不知道我已经加入传奇商会了吗？呵呵，不过听说王大人那个人也不错，而且比起传奇商会来说条件也要更好。</par>
                        <par>但是不管怎么说都要讲点道义啊，怎么能像手心手背那样说翻就翻呢？</par>
                        <par></par>
                        <par><event id="npc_discuss_2">那就没有别的办法了吗？</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,

            npc_discuss_2 = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>嗯！？既然你都这么说了，我倒是有一个建议。</par>
                        <par>最近比奇省里流行传染病，配制治疗这种病的药所需的原料毒蛇牙齿非常的紧缺。这种毒蛇牙齿在毒蛇山谷村就有卖的，但是我现在马上要给源源不断而来的病人治病，没有去买药材的时间。</par>
                        <par>传奇商会那帮人唯利是图，人命关天的事却无人愿意搭把手帮助我。如果你能够买来足够我们所需的毒蛇牙齿，我就会抛开商人的身份来以医生的角度听从您的劝说。</par>
                        <par></par>
                        <par><event id="npc_accept">没问题！</event></par>
                        <par><event id="npc_refuse">请给我点儿考虑的时间。</event></par>
                    </layout>
                ]=])
            end,

            npc_accept = function(uid, value)
                uidPostXML(uid,
                [=[
                    <layout>
                        <par>从这儿向东北部去就能到达毒蛇山谷，可能去(643,15)附近就能够找得到。</par>
                        <par>穿过毒蛇山谷一直向东走就会达到那个村庄。在那儿找药商<t color="red">金中医</t>(334,224)向他购买<t color="red">毒蛇牙齿</t>。</par>
                        <par>现在患者数量仍然呈增加的趋势，所以还不能推测出以后具体需要多少药材。不管怎么样你都要快去快回。</par>
                        <par></par>
                        <par><event id="%%s">好的</event></par>
                    </layout>
                ]=], SYS_EXIT)
                uidExecute(questUID, [=[ setUIDQuestState(%%d, SYS_ENTER) ]=], uid)
            end,

            npc_refuse = function(uid, value)
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>明白吗，年轻人？！</par>
                        <par>千万不要太拖延而忘了一切啊！人命关天啊！</par>
                        <par>我们所有人啊！</par>
                        <par></par>
                        <par><event id="%%s">退出</event></par>
                    </layout>
                ]=], SYS_EXIT)
            end,
        })
    ]], getUID(), asInitString(getQuestName()))

    setQuestFSMTable(
    {
        [SYS_ENTER] = function(uid, value)
            uidExecute(getNPCharUID('比奇县_0', '药剂师_1'),
            [[
                local playerUID = %d
                local questName = %s
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>患者越来越多，快去<event id="npc_path_details">毒蛇山谷</event>买<t color="red">毒蛇牙齿</t>吧！</par>
                                <par></par>
                                <par><event id="%%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,

                    npc_path_details = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>从这儿向东北部去就能到达毒蛇山谷，去(643,15)附近就能够找得到。穿过毒蛇山谷一直向东走就会达到那个村庄，在那儿找药商<t color="red">金中医</t>(334,224)向他购买<t color="red">毒蛇牙齿</t>。</par>
                                <par></par>
                                <par><event id="%%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, asInitString(getQuestName()))

            uidExecute(getNPCharUID('毒蛇山谷_2', '金中医_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = %s
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>噢！是来买毒蛇牙齿的啊！</par>
                                <par></par>
                                <par><event id="npc_buy_tooth">是的！</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_buy_tooth = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>唔？看起来你不是要药材商或者从医的人吧！</par>
                                <par>这倒无所谓！不过作为药用的毒蛇牙齿的产量是固定的，每天充其量能供给几包而已。难道你不知道吗？</par>
                                <par></par>
                                <par><event id="npc_seller_call_price">我是受比奇省药剂师之托而来的</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_seller_call_price = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>比奇省发生传染病? 你说的是真的吗？那我现在就卖给你一包吧！</par>
                                <par>现在只有这些，如果需要的话再来吧！价格是100钱一颗，给我1000钱就行。</par>
                                <par><event id="npc_pay_full_price">全额付款</event></par>
                                <par><event id="npc_ask_for_discount">讨价还价</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_pay_full_price = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>很着急的样子啊！</par>
                                <par>给你，快去比奇省看看吧！</par>
                                <par></par>
                                <par><event id="%%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_purchased_tooth') ]=], uid)
                    end,

                    npc_ask_for_discount = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>城内的情况这么紧急的话我就赔本卖给你吧！</par>
                                <par>每颗10钱，只付100钱就行。连加工费都去掉就按成本给你啦！</par>
                                <par></par>
                                <par><event id="npc_setup_purchase_quest">好的</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_setup_purchase_quest = function(uid, value)
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_purchase_with_agreed_price', 100) ]=], uid)
                    end,
                })
            ]], uid, getUID(), asInitString(getQuestName()))
        end,

        quest_purchase_with_agreed_price = function(uid, value)

            -- purchase with agreed price
            -- this state won't pop up dialog if player has enough money

            assertType(value, 'integer')
            assert(value > 0)

            uidExecute(getNPCharUID('毒蛇山谷_2', '金中医_1'),
            [[
                local playerUID = %d
                local askedGold = %d
                local questUID  = %d
                local questName = %s

                local questPath = {SYS_EPUID, questName}
                local currGold  = uidExecute(playerUID, [=[ return getGold() ]=])

                if currGold >= askedGold then
                    uidPostXML(playerUID,
                    [=[
                        <layout>
                            <par>东西都在这儿快快拿去，赶紧返回<t color="red">比奇省</t>吧！</par>
                            <par><event id="%%s">好的</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    uidExecute(playerUID,
                    [=[
                        removeItem(getItemID(SYS_GOLDNAME), 0, %%d)
                        addItem(getItemID('毒蛇牙齿'), 10)
                    ]=], askedGold)

                    uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_purchased_tooth') ]=], playerUID)

                else
                    local rand = math.random(0, 100)
                    if rand <= 0 then
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_purchase_with_free_price') ]=], playerUID)
                        -- TODO interesting part here
                        -- needs to wait for quest state change which reset the handler
                        -- otherwise next runEventHandler() triggers old handlr
                        pause(500)
                        uidExecute(getUID(), [=[ runEventHandler(%%d, %%s, SYS_ENTER) ]=], playerUID, asInitString(questPath))

                    elseif rand <= 50 then
                        uidPostXML(playerUID,
                        [=[
                            <layout>
                                <par>你是在开玩笑吗？你没有<t color="red">%%d</t>金币啊？！</par>
                                <par>你这个不老实的家伙，不要再浪费我的时间了！先凑够<t color="red">%%d</t>金币再来找我吧！</par>
                                <par><event id="%%s">退出</event></par>
                            </layout>
                        ]=], askedGold, askedGold, SYS_EXIT)
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_wait_purchase', %%d) ]=], playerUID, askedGold)

                    else
                        local newAskedGold = math.ceil(askedGold * 1.5)
                        uidPostXML(playerUID,
                        [=[
                            <layout>
                                <par>你是在开玩笑吗？你没有<t color="red">%%d</t>金币啊？！</par>
                                <par>你这个家伙实在浪费我的一片好心，不要再说了！我决定涨价<t color="red">50%%%%</t>，先凑够<t color="red">%%d</t>金币再来找我吧！</par>
                                <par><event id="%%s">退出</event></par>
                            </layout>
                        ]=], askedGold, newAskedGold, SYS_EXIT)
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_wait_purchase', %%d) ]=], playerUID, newAskedGold)
                    end
                end
            ]], uid, value, getUID(), asInitString(getQuestName()))
        end,

        quest_wait_purchase = function(uid, value)
            uidExecute(getNPCharUID('毒蛇山谷_2', '金中医_1'),
            [[
                local playerUID = %d
                local askedGold = %d
                local questUID  = %d
                local questName = %s
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你带来<t color="red">%%d</t>金币了吗？</par>
                                <par></par>
                                <par><event id="npc_purchase">带来了！</event></par>
                                <par><event id="%%s">我还没凑齐！</event></par>
                            </layout>
                        ]=], askedGold, SYS_EXIT)
                    end,

                    npc_purchase = function(uid, value)
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_purchase_with_agreed_price', %%d) ]=], uid, askedGold)
                    end,
                })
            ]], uid, value, getUID(), asInitString(getQuestName()))
        end,

        quest_purchase_with_free_price = function(uid, value)
            uidExecute(getNPCharUID('毒蛇山谷_2', '金中医_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = %s
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>真是个难缠的家伙！拿你没办法，先免费给你快拿去给病人们治病用吧！</par>
                                <par></par>
                                <par><event id="npc_say_thanks">谢谢你的慷慨！</event></par>
                            </layout>
                        ]=])
                    end,

                    npc_say_thanks = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>我是看药剂师的面子才免费的！东西都在这儿快快拿去，赶紧返回<t color="red">比奇省</t>吧！</par>
                                <par><event id="%%s">好的</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                        uidExecute(uid, [=[ addItem(getItemID('毒蛇牙齿'), 10) ]=])
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, 'quest_purchased_tooth') ]=], uid)
                    end,
                })
            ]], uid, getUID(), asInitString(getQuestName()))
        end,

        quest_purchased_tooth = function(uid, value)
            uidExecute(getNPCharUID('毒蛇山谷_2', '金中医_1'),
            [[
                local playerUID = %d
                local questName = %s
                local questPath = {SYS_EPUID, questName}

                setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>干嘛呢？还不快把药材带给<t color="yellow">比奇省</t><t color="red">药剂师</t>。</par>
                                <par></par>
                                <par><event id="%%s">退出</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end,
                })
            ]], uid, asInitString(getQuestName()))

            uidExecute(getNPCharUID('比奇县_0', '药剂师_1'),
            [[
                local playerUID = %d
                local questUID  = %d
                local questName = %s
                local questPath = {SYS_EPQST, questName}

                return setUIDQuestHandler(playerUID, questName,
                {
                    [SYS_ENTER] = function(uid, value)
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>您为病人们做了一件大好事！所以我会听从你的劝说加入王大人的比奇商会的，只好对不起崔大夫了！</par>
                                <par>啊！对了，这是金创药，收下这个吧！急匆匆地走了这么远的路累坏了吧！喝了这个可以补充一下元气。</par>
                                <par></par>
                                <par><event id="%%s">谢谢！</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                        uidExecute(uid, [=[ addItem(getItemID('金创药（特）'), 8) ]=])
                        uidExecute(questUID, [=[ setUIDQuestState(%%d, SYS_DONE) ]=], uid)
                    end,
                })
            ]], uid, getUID(), asInitString(getQuestName()))
        end,
    })
end
