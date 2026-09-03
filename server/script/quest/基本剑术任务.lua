-- converted from Envir/QuestDiary/MU_warrior/wesu.txt
--
-- 龙血先生 does not send you anywhere for this one. 基本剑术 is the first thing a warrior
-- learns, so all he asks is that you are level 7 and already carrying the plain 基本剑术
-- 技能书 — then he talks you through it and copies the book out into a 秘籍
--
-- the plain book is raw material and can not be studied, only the 秘籍 can, see
-- ItemRecord::isMagicBook
--
-- wesu.txt ends with a @mugong_wesu_explain block, 想学习基本剑术，基本的训练结束后再来找我吧！,
-- that no NPC ever calls. it is dead in the legacy data too and has no place to go here

_G.minQuestLevel = 7

_G.magicName = '基本剑术'
_G.bookName  = '基本剑术'
_G.mijiName  = '基本剑术（秘籍）'

_G.teacherMap = '边境城市_01'
_G.teacherNPC = '龙血先生_1'

-- nothing to track, the whole thing happens in one conversation
setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setQuestDesp{uid=uid, ''}
    end,
})

uidRemoteCall(getNPCharUID(teacherMap, teacherNPC), getUID(), getQuestName(), minQuestLevel, magicName, bookName, mijiName,
[[
    local questUID, questName, minQuestLevel, magicName, bookName, mijiName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼基本剑术',

        -- @mugong_wesu answered out of flag [700] and stayed clickable either way
        [SYS_CHECKACTIVE] = function(uid)
            if not server.player.hasJob(uid, '战士') then
                return false
            end

            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [700] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到<t color="red">基本剑术秘籍</t>吗？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 基本剑术
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我看你已经掌握了<t color="red">基本剑术</t>。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checklevel 7
            if server.player.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>如果想学基本剑术，武功级别最少要达到<t color="red">%d</t>以上。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], minQuestLevel, SYS_EXIT)
                return
            end

            -- checkitem 基本剑术 1, the plain book he copies from
            if not server.player.hasItem(uid, bookName, 1) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>有了<t color="red">基本剑术魔法书</t>，我就可以教你魔法。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想学习基本剑术的样子。但是像你一样的初学者修炼武功还是有各种各样的困难，我将给你进行详细的说明。你现在也正式进入了成为战士之路。祝贺你！</par>
                    <par></par>
                    <par>那么在对秘籍进行解说之前，要听对武功的说明吗？</par>
                    <par><event id="npc_lore">拜托了！</event></par>
                    <par><event id="%s" close="1">不需要了！</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_wesu_next3
        npc_lore = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>基本剑术是凭借通过反复的训练获得灵敏的感觉，找到敌人弱点进行攻击的方法，是战士的基本武功。</par>
                    <par></par>
                    <par><event id="npc_take_book">很基础的魔法嘛。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_wesu_next4, take the plain book and hand back the 秘籍
        npc_take_book = function(uid, value)
            -- he checks again, you could have dropped it during the talk
            if not server.player.hasItem(uid, bookName, 1) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>有了<t color="red">基本剑术魔法书</t>，我就可以教你魔法。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你现在已经有基本剑术秘籍了，以前不理解的部分也可以理解了。</par>
                    <par></par>
                    <par><event id="%s" close="1">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            server.player.removeItem(uid, bookName, 1)
            server.player.addItem(uid, mijiName, 1)
            server.quest.setState(questUID, {uid=uid, state=SYS_DONE})
        end,
    })
]])
