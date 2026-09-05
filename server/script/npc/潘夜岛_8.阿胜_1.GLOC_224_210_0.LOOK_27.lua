-- converted from Envir/Market_Def/02Weapon_HalfNight-8.txt

local smith = require('npc.include.merchant.smith')
local dialogue = require('npc.include.dialogue')
local spiritItemID = getItemID('潘夜天灵')

if spiritItemID == 0 then
    addLog(LOGTYPE_WARNING, 'Pan Ye spirit exchange unavailable: missing item record for 潘夜天灵')
end

local function postSpiritFarewell(uid)
    dialogue.post(uid,
    {
        '哎，何时再能重新封印潘夜神殿呢？老朽愿有生之年能看见这一天.....',
    }, {dialogue.link(SYS_EXIT, '离开')})
end

smith.setSmith
{
    greet =
    {
        '欢迎光临，你需要什么？',
    },

    redName = '有什么事？我跟你无话可说。',

    goods =
    {
        '青铜斧',
        '八荒',
        '凌风',
        '斩马刀',
        '修罗',
        '海魂',
        '半月',
    },

    buyText =
    {
        '要想在这种偏僻的地方生存下去，必须借助精良的武器，来看一下吧。',
    },

    sellText =
    {
        '请把你不用的武器卖给我。',
    },

    repairText =
    {
        '我可以给你修理武器，不过持久性可能会降低，这我也没办法。',
    },

    repairDone = '这已经算修得不错的了，拿走吧。',

    -- @main_0_A takes precedence over the ordinary shop while the player has the spirit.
    onEnter = function(uid, value)
        if spiritItemID == 0 or not server.player.hasItem(uid, '潘夜天灵', 1) then
            return false
        end
        dialogue.post(uid,
        {
            '......这不是<t color="red"> 潘夜天灵 </t>吗？ 沉睡千年的天灵，终于重见天日了.....传说中，潘夜牛魔王被传说中的武林宗师打败后押在了潘夜岛。封印在了潘夜神殿最深处。而封印的时使用的就是潘夜天灵，然而随着时间流逝，封印的力量变弱，牛魔王复活于神殿之中。并且窃取了封印的力量为己有，躲在潘夜山洞里养精蓄锐，等待时机东山再起！',
        }, {dialogue.link('npc_hntl_01', '我该怎么办？')})
        return true
    end,

    extra =
    {
        npc_hntl_01 = function(uid, value)
            dialogue.post(uid,
            {
                '潘夜天灵虽其貌不扬，但蕴含着强烈的寒冰之气，为玛法至阴至寒之物之一，<t color="red"> 寒冰之气 </t>为潘夜畜族之煞星，乃封印潘夜诸魔之利器，重见天日的潘夜天灵应移交给潘夜岛武林宗师的后人，由他们进行对潘夜畜族的重新封印。当然，世人皆知，潘夜天灵来之不易，<t color="red"> ' .. uidQueryName(uid) .. ' </t>勇士，你可以将天灵交给老夫，由老夫转交给武林宗师之传人。为了报答你历经艰险取得的珍宝，老朽特为你准备制作一把精良的武器.不知尊意如何？',
            },
            {
                dialogue.link('npc_hntl_02', '既然如此,那我应该义不容辞地将此物托付给您了'),
                dialogue.link('npc_hntl_01_fail', '想一想还是算了.'),
            })
        end,

        npc_hntl_01_fail = function(uid, value)
            postSpiritFarewell(uid)
        end,

        npc_hntl_02 = function(uid, value)
            dialogue.post(uid,
            {
                '潘夜岛的百姓将会永远感谢你的.....',
                '作为报酬,老朽就将这把武器送给你吧，但愿你能用得上.',
            }, {dialogue.link('npc_hntl_receive', '取得武器.')})
        end,

        npc_hntl_receive = function(uid, value)
            if spiritItemID == 0 then
                dialogue.post(uid, {'目前无法兑换潘夜天灵，请稍后再来。'}, {dialogue.link(SYS_EXIT, '离开')})
                return
            end

            -- Select the reward and exchange items together so closing the conversation
            -- cannot consume the spirit without granting the weapon.
            local received = uidRemoteCall(uid, spiritItemID,
                getItemID('潘夜炼狱'), getItemID('潘夜魔杖'), getItemID('潘夜银蛇'),
            [[
                local spiritItemID, warriorReward, wizardReward, taoistReward = ...
                local reward = taoistReward
                for _, job in ipairs(getJobList()) do
                    if job == '战士' then
                        reward = warriorReward
                        break
                    elseif job == '法师' then
                        reward = wizardReward
                    end
                end
                if not removeItem(spiritItemID, 0, 1) then
                    return false
                end
                addItem(reward, 1)
                return true
            ]])
            assertType(received, 'boolean')
            if not received then
                dialogue.post(uid, {'你已经没有潘夜天灵了。'}, {dialogue.link(SYS_EXIT, '离开')})
                return
            end
            postSpiritFarewell(uid)
        end,
    },
}
