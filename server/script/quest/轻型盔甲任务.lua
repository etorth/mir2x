_G.minQuestLevel = 11

setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        uidRemoteCall(uid, uid, getUID(),
        [[
            local playerUID, questUID = ...
            addTrigger(SYS_ON_GAINITEM, function(itemID, seqID)
                if hasItem(getItemID('铁矿'), 0, 5) then
                    postString('已经收集到5块铁矿了，快回去找怡美吧！')
                    server.quest.setState(questUID, {uid=playerUID, state='quest_got_iron'})
                    return true
                end
            end)
        ]])

        setupNPCQuestBehavior('比奇县_0', '怡美_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, args)
                    dialog.post(uid, questPath, '还没有带来我要的铁矿啊！',
                    {
                        dialog.link('npc_where_to_get_iron', '去哪儿才能找到铁矿呢？'),
                        dialog.link('npc_patience', '请再给我一些时间。'),
                    })
                end,

                npc_where_to_get_iron = function(uid, args)
                    dialog.post(uid, questPath,
                    {
                        '不知道就说不知道嘛！嗨...',
                        '先去武器店或铁匠铺买把鹤嘴锄，再去矿山就可以挖到各种矿石。',
                        '从中挑出5个纯度在13以上的铁矿带给我就行。',
                        '离这里最近的矿山是比奇矿区，可能去764:206附近就能找到入口。',
                    },
                    dialog.link(SYS_EXIT, '结束'))
                end,

                npc_patience = function(uid, args)
                    dialog.post(uid, questPath, '那我等你的好消息。',
                    dialog.link(SYS_EXIT, '结束'))
                end,
            }
        ]])
    end,

    quest_got_iron = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '怡美_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}
            local dialog = require('include.dialog')

            return
            {
                [SYS_ENTER] = function(uid, args)
                    dialog.post(uid, questPath,
                    {
                        '拿来铁矿了啊！嘻嘻，现在不用担心原料不足，可以稳定的供应顾客的需求啦！',
                        '太谢谢你啦！这是我说好为你定做的特制轻型盔甲。但愿它够带给你很大帮助，呵呵呵！',
                        '这可是我特地为您精心制作的哦，所以要像我一样珍惜爱护它啊，呵呵呵！',
                    },
                    dialog.link(SYS_EXIT, '结束'))

                    server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '怡美_1'), getUID(), getQuestName(), minQuestLevel,
[[
    local questUID, questName, minQuestLevel = ...
    local questPath = {SYS_EPQST, questName}
    local dialog = require('include.dialog')

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            return server.quest.getState(questUID, {uid=uid}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            if server.player.getLevel(uid) < minQuestLevel then
                dialog.post(uid, questPath,
                {
                    string.format('您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！', server.player.getName(uid)),
                    string.format('尽管你和王大人的关系十分熟，但是我跟你这种等级没有达到%d的毛头小子没什么话可说。想让我完全信任你的话还是再去好好修炼一下再来找我吧！', minQuestLevel),
                },
                dialog.link(SYS_EXIT, '结束'))

            else
                local dressName = getItemName((server.player.getWLItem(uid, WLG_DRESS) or {}).itemID)
                if not dressName then
                    dialog.post(uid, questPath,
                    {
                        string.format('您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！', server.player.getName(uid)),
                        '不过...你这样的侠客，却不穿衣服在这里招摇过市真的好吗？这种打扮实在是让我难以置信啊！',
                        '本店特色商品轻型盔甲，一直备受各路闯荡江湖的侠客青睐，你感兴趣吗？',
                    },
                    {
                        dialog.link('npc_query', '嗯？有这种东西？很感兴趣！'),
                        dialog.link('npc_ask_when_not_interested', '不感兴趣。'),
                    })

                elseif string.match(dressName, '布衣.+') then
                    dialog.post(uid, questPath,
                    {
                        string.format('您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！', server.player.getName(uid)),
                        '不过...看起来你今天的穿着很是稀松平常，说实话这种打扮实在难以让我信任啊！',
                        '不知你是否知道我们店里所卖的轻型盔甲呢？',
                    },
                    {
                        dialog.link('npc_query', '嗯？有这种东西？很感兴趣！'),
                        dialog.link('npc_ask_when_not_interested', '不感兴趣。'),
                    })

                elseif string.match(dressName, '轻型盔甲.+') then
                    dialog.post(uid, questPath,
                    {
                        string.format('您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！', server.player.getName(uid)),
                        '噢！已经穿上轻型盔甲了！的确与众不同啊！不过这衣服好像还是有点太平常了！',
                        '实不相瞒，我有一件关于这种盔甲的事情要拜托你，如果你能够帮我我一把话，我就会为你做一套更漂亮的轻型盔甲。',
                    },
                    dialog.link('npc_introduce_quest', '有什么要拜托的事情请您尽管说。'))

                else
                    dialog.post(uid, questPath,
                    {
                        string.format('您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！', server.player.getName(uid)),
                        '您穿上这身衣服果然是器宇不凡！',
                        '本店特色商品轻型盔甲，一直备受各路闯荡江湖的侠客青睐，你感兴趣吗？',
                    },
                    {
                        dialog.link('npc_query', '嗯？有这种东西？很感兴趣！'),
                        dialog.link('npc_ask_when_not_interested', '不感兴趣。'),
                    })
                end
            end
        end,

        npc_query = function(uid, args)
            dialog.post(uid, questPath,
            {
                '<t color="red">轻型盔甲</t>是等级达到11级之后才可以穿上的防御服。',
                '主要部分都是用钢铁打造的，所以比起布衣要重的多，但是防御力也特别的好。',
                '实不相瞒，我有一件关于这种盔甲的事情要拜托你...',
            },
            dialog.link('npc_introduce_quest', '有什么要拜托的事情请您尽管说。'))
        end,

        npc_ask_when_not_interested = function(uid, args)
            local _ = server.player.getName(uid)
            dialog.post(uid, questPath,
            {
                '既然您不感兴趣，那我就不再介绍了...',
                '其实我并非想向您推销轻型盔，而是有一件关于这种盔甲的事情要拜托你...',
            },
            dialog.link('npc_introduce_quest', '有什么要拜托的事情请您尽管说。'))
        end,

        npc_introduce_quest = function(uid, args)
            dialog.post(uid, questPath,
            {
                '那就太谢谢了！是这样的，最近来买轻型盔甲的顾客非常多，可是用来做辅强剂的铁矿不够用了。',
                '所以您要是能够给我找来5个纯度13以上的铁矿的话，我就会为你特别制作一套轻型盔甲。',
            },
            {
                dialog.link('npc_where_to_get_iron', '到哪儿去找铁矿呢？'),
                dialog.link('npc_accept_quest', '知道了。'),
            })
        end,

        npc_where_to_get_iron = function(uid, args)
            dialog.post(uid, questPath,
            {
                dialog.link('npc_nearest_iron', '矿山', {prefix = '先去武器店或铁匠铺买把鹤嘴锄，再去', suffix = '就可以挖到各种矿石！'}),
                '挑出我所需要的纯度在13以上的铁矿之后，剩下的还可以卖给武器店赚到很多钱。',
                '如果你觉得麻烦不想去矿山挖矿的话也可以去武器店购买铁矿，但是那样的话就有点亏本哦！',
            },
            dialog.link('npc_accept_quest', '知道了。', {close = true}))
        end,

        npc_nearest_iron = function(uid, args)
            dialog.post(uid, questPath, '离这里最近的矿山是比奇矿区，可能去764:206附近就能找到入口。',
            dialog.link('npc_accept_quest', '知道了。', {close = true}))
        end,

        npc_accept_quest = function(uid, args)
            server.quest.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,
    })
]])
