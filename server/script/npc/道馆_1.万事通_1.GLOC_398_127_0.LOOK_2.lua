_G.questName_basic    = '初级任务'

_G.questName_mujun    = '乞丐任务'
_G.questName_pariche  = '苍蝇拍任务'
_G.questName_dolumi   = '石母任务'

_G.questName_wang     = '王大人任务'
_G.questName_bichun   = '比奇省任务'
_G.questName_yaksa    = '药剂师任务'

_G.questName_kyunggap = '轻型盔甲任务'
_G.questName_oma      = '半兽人任务'

_G.questName_younghon = '被盗灵魂任务'
_G.questName_bamgol   = '千年毒蛇任务'
_G.questName_tarak    = '堕落道士任务'

_G.questName_umyun    = '沃玛教主任务'

local quest_config = {
    [questName_mujun] = {
        level = 3,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始完成乞丐任务呢！</par>
                    <par>这个任务是去帮助在比奇省东海客栈工作的客栈店员遇到的麻烦。</par>
                    <par>可惜你现在还没有能力帮助她啊！先去把<t color="red">等级</t>修炼提高到<t color="red">3</t>以上再说吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始完成乞丐任务呢！</par>
                    <par>在比奇省东海客栈工作的客栈店员最近好像有点棘手的事情，去看看吧！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你辛苦了。</par>
                    <par>像你这样热心帮助别人的好心人，一定有好报的...</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_pariche] = {
        level = 5,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始苍蝇拍任务呢！</par>
                    <par>这是一个为在比奇省经营肉铺店的金氏找苍蝇拍的任务！</par>
                    <par>可惜你现在还没有帮助他的能力。先去把等级提高到<t color="red">5</t>以上吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始苍蝇拍任务呢！</par>
                    <par>比奇省经营肉铺店的金氏正在因为没有苍蝇拍的事儿而苦恼呢！去看看怎么回事吧！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>金氏拿到苍蝇拍以后，对你非常感激。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_dolumi] = {
        level = 7,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始石母任务呢！</par>
                    <par>石母任务是安抚冤魂的任务。可惜你现在还没有能力帮助她，首先去把等级修炼提高到7以上吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始石母任务呢！</par>
                    <par>最近，在比奇省东边的公园常传出隐约的抽泣声。前去调查一番吧！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>哦？原来石母就是那童子像的母亲石啊！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_wang] = {
        level = 9,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始王大人任务呢！</par>
                    <par>要想挑战王大人任务，首先要完成初级任务。</par>
                    <par>王大人任务是去帮助王大人将比奇省商界掌握在比奇商会手中。</par>
                    <par>可惜你现在还没有帮助王大人的能力，首先要将等级提高到9以上才行！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        prequest = questName_basic,
        prequest_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>要想挑战王大人任务，首先要完成初级任务。</par>
                    <par>最起码要跟王大人有点交情才能得到王大人的信任，不是吗？</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始王大人任务呢！</par>
                    <par>王大人任务是去帮助王大人将比奇省商界掌握在比奇商会手中。</par>
                    <par>完成任务后，凭借你与王大人的交情将会对完成以后的任务有很大帮助！</par>
                    <par>那么，先去王大人那儿走走吧。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>很好，现在比奇省商界的势力就要改朝换代了！</par>
                    <par>从王大人的话中得知好像也要得到比奇城城主的信任才行！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_kyunggap] = {
        level = 11,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始轻型盔甲任务呢！</par>
                    <par>轻型盔甲任务就是帮助比奇省的棉布商怡美解决困难。</par>
                    <par>可惜你现在还没有能力帮助怡美。因此，首先要将等级提高到11以上再说吧。</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        prequest = questName_wang,
        prequest_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始轻型盔甲任务呢！</par>
                    <par>轻型盔甲任务就是帮助比奇省的棉布商“怡美”解决困难。</par>
                    <par>要想得到在比奇省做生意的怡美姑娘的信任，可是需要一定声望才行的。</par>
                    <par>要首先完成了王大人任务才行。</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始轻型盔甲任务呢！</par>
                    <par>去找一找在比奇省经营布店的怡美，她会向你提出一个请求。</par>
                    <par>如果完成了她拜托你办的事情的话，你会得到她送给你的礼物哦。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你辛苦了！</par>
                    <par>得到特制的轻型盔甲，辛苦总算没有白费啊！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_oma] = {
        level = 11,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始半兽任务呢！</par>
                    <par>半兽任务是制止以半兽勇士为中心集结起来的半兽人纠合势力使用古代魔法的阴谋。</par>
                    <par>可惜你现在还没有与半兽勇士和半兽人对抗的能力。首先将等级提高到11以上再说吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        prequest = questName_wang,
        prequest_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始半兽任务呢！</par>
                    <par>半兽任务是制止以半兽勇士为中心集结起来的半兽人纠合势力使用古代魔法的阴谋。</par>
                    <par>首先要证明自己的能力，才能让官府启用你。先要完成王大人任务才行。</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始半兽任务呢！</par>
                    <par>半兽任务是制止以半兽勇士为中心集结起来的半兽人纠合势力使用古代魔法的阴谋。</par>
                    <par>前往比奇省内城找比奇城城主看看，他好像要拜托你帮助他阻止半兽人的阴谋。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>太辛苦你了！</par>
                    <par>托你的福，来自半兽人的威胁已经大大减轻了。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_younghon] = {
        level = 11,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始被盗灵魂任务呢！</par>
                    <par>最近比奇省的贩牛商王小二的独生女王丽灵突然被发现一夕之间变成了白痴。好像是最近盗取百姓灵魂的妖怪干的好事！灵魂任务就是揭开这个妖怪的真面目。</par>
                    <par>可惜你现在还没有能力完成灵魂任务。先去将等级提高到16以上吧。</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        prequest = questName_oma,
        prequest_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始被盗灵魂任务呢！</par>
                    <par>这次任务是揭露盗取灵魂导致民心惶惶的妖怪真面目！</par>
                    <par>你要证明你有能力对抗妖怪才行啊！那么，首先要去完成半兽人任务吧。</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始被盗灵魂任务呢！</par>
                    <par>最近比奇省的贩牛商王小二的独生女王丽灵突然被发现一夕之间变成了白痴。好像是最近盗取百姓灵魂的妖怪干的好事！灵魂任务就是揭开这个妖怪的真面目。</par>
                    <par>先去王小二的亲戚王大人家了解一些情况吧。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>再晚到一步，就让邪恶的道士偷到不死牌了！</par>
                    <par>你救出了王丽灵小姐，并揭开妖怪的真面目，真的很不错啊！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_bamgol] = {
        level = 16,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始千年毒蛇任务呢！</par>
                    <par>毒蛇山谷出现了传说中的蛇，寻找这条传说中的蛇，取得解毒剂就是千年毒蛇任务。</par>
                    <par>可惜你现在还没有能力帮助毒蛇山谷的毒蛇山谷老太啊！首先去将等级修炼提高到16以上吧。</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        prequest = questName_younghon,
        prequest_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始千年毒蛇任务呢！</par>
                    <par>听说毒蛇山谷出现了传说中的蛇，寻找这条传说中的蛇，取得解毒剂就是千年毒蛇任务。</par>
                    <par>可惜你现在还没有对抗传说中千年毒蛇的能力，无法得到信任啊！还是先去完成被盗灵魂任务吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始千年毒蛇任务呢！</par>
                    <par>毒蛇山谷中出现了传说中的蛇，叫做珍珍的小孩好像已经中毒了！当务之急就是找那条传说中的蛇取得解毒剂。</par>
                    <par>先去毒蛇山谷去见珍珍的毒蛇山谷老太吧！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>幸亏你及时找到药，毒蛇山谷的珍珍现在已经平安无恙了！</par>
                    <par>辛苦你了！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_tarak] = {
        level = 16,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始堕落道士任务呢！</par>
                    <par>经过浴血奋战从半兽人手中拿到的不死牌，却被某个道士给偷走了。因此必须粉碎这个堕落道士计划利用不死牌进行的邪恶阴谋！这就是堕落道士任务。</par>
                    <par>可惜你现在还没有能力阻止堕落道士的阴谋。先去把等级提高到16以上吧。</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        prequest = questName_younghon,
        prequest_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>这不是还没有完成被盗灵魂任务的你所应该关心的事儿！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始堕落道士任务呢！</par>
                    <par>经过浴血奋战从半兽人手中拿到的不死牌，却被某个道士给偷走了。 因此必须粉碎这个堕落道士计划利用不死牌进行的邪恶阴谋！</par>
                    <par>去向曾经保管不死牌的比奇城城主打听具体的情况吧！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你做得很棒！由于你的努力堕落道士引发的灾难终于到此结束了！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_umyun] = {
        level = 20,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>抱歉，对于这个任务我几乎一无所知。</par>
                    <par>我只是听说过去曾经追随沃玛教主的沃玛教最后幸存者为了弥补过去的罪过而与邪恶势力在孤军奋战。去帮助他破坏沃玛教主在地上发挥力量的根源---灵魂明珠，并处决沃玛教主好让那些惨死的冤魂们升天。</par>
                    <par>同沃玛教主对抗需要很强的实力。可惜你现在还没有这个能力，因此首先要将等级提高到20以上再说吧。</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>抱歉，对于这个任务我几乎一无所知。</par>
                    <par>我只是听说过去曾经追随沃玛教主的沃玛教最后幸存者为了弥补过去的罪过而与邪恶势力在孤军奋战。去帮助他破坏沃玛教主在地上发挥力量的根源---灵魂明珠，并处决沃玛教主好让那些惨死的冤魂们升天。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },
}

setEventHandler(
{
    [SYS_ENTER] = function(uid, args)
        uidPostXML(uid,
        [[
            <layout>
                <par>江湖上的朋友都叫我万拍子，不是我吹，你不了解的任务我都可以给你解答。你有什么想问的吗？</par>
                <par></par>
                <par><event id="npc_show_quest_list">询问一般的任务</event></par>
                <par><event id="npc_daily_quest">对今日的任务进行了解</event></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_EXIT)
    end,

    npc_show_quest_list = function(uid, args)
        local fn_create_tag = function(questName)
            return string.format([[<event id="npc_run_quest" args="%s">%s</event>]], questName, questName)
        end

        uidPostXML(uid,
        [[
            <layout>
                <par>来吧，你有什么任务？</par>
                <par>%s，%s，%s</par>
                <par></par>

                <par>（等级 9）</par>
                <par>%s，%s，%s</par>
                <par></par>

                <par>（等级 11）</par>
                <par>%s，%s</par>
                <par></par>

                <par>（等级 16）</par>
                <par>%s，%s，%s</par>
                <par></par>

                <par>（等级 20）</par>
                <par>%s</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]],

        fn_create_tag(questName_mujun),
        fn_create_tag(questName_pariche),
        fn_create_tag(questName_dolumi),

        fn_create_tag(questName_wang),
        fn_create_tag(questName_bichun),
        fn_create_tag(questName_yaksa),

        fn_create_tag(questName_kyunggap),
        fn_create_tag(questName_oma),

        fn_create_tag(questName_younghon),
        fn_create_tag(questName_bamgol),
        fn_create_tag(questName_tarak),

        fn_create_tag(questName_umyun),

        SYS_ENTER, SYS_EXIT)
    end,

    npc_daily_quest = function(uid, args)
        if uidQueryLevel(uid) < 7 then
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有足够能力执行此任务，修炼到7级后，再来找我吧。</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        else
            require('npc.include.dailyquest').setQuest(0, uid, args)
        end
    end,

    npc_run_quest = function(uid, args)
        if not quest_config[args] then
            uidPostXML(uid,
            [[
                <layout>
                    <par>我对<t color="red">%s</t>一无所知。</par>
                    <par></par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]], args)

        elseif quest_config[args].prequest and plyapi.getQuestState(uid, quest_config[args].prequest) ~= SYS_DONE then
            if quest_config[args].prequest_failure then
                quest_config[args].prequest_failure(uid)

            else
                uidPostXML(uid,
                [[
                    <layout>
                        <par>你还没有开始完成<t color="red">%s</t>呢！</par>
                        <par></par>
                        <par><event id="npc_show_quest_list">前一步</event></par>
                    </layout>
                ]], quest_config[args].prequest)
            end

        elseif quest_config[args].level and plyapi.getLevel(uid) < quest_config[args].level then
            if quest_config[args].level_failure then
                quest_config[args].level_failure(uid)

            else
                uidPostXML(uid,
                [[
                    <layout>
                        <par>你还没有达到<t color="red">%d</t>级呢！</par>
                        <par></par>
                        <par><event id="npc_show_quest_list">前一步</event></par>
                    </layout>
                ]], quest_config[args].leve)
            end

        else
            local questState = plyapi.getQuestState(uid, args)
            if questState == nil then
                if quest_config[args].on_start then
                    quest_config[args].on_start(uid, args)

                else
                    uidPostXML(uid,
                    [[
                        <layout>
                            <par>暂无关于<t color="red">%s</t>的更多信息...</par>
                            <par></par>
                            <par><event id="npc_show_quest_list">前一步</event></par>
                        </layout>
                    ]], args)
                end

            elseif questState == SYS_DONE then
                if quest_config[args].on_done then
                    quest_config[args].on_done(uid, args)

                else
                    uidPostXML(uid,
                    [[
                        <layout>
                            <par>你已经完成了<t color="red">%s</t></par>
                            <par></par>
                            <par><event id="npc_show_quest_list">前一步</event></par>
                        </layout>
                    ]], args)
                end

            else
                runEventHandler(uid, {SYS_EPUID, args}, SYS_ENTER)
            end
        end
    end,
})
