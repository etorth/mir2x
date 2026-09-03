-- converted from Envir/QuestDiary/MU_taoist/heal.txt
--
-- the taoist counterpart of 基本剑术: no trial, no reward, 清明子 just wants you at level 7
-- with the plain 治愈术 技能书 in hand and then copies it out into a 秘籍
--
-- heal.txt ends with a @mugong_heal_explain block, 如果想学习治愈术，基本的修炼结束后，请再来找我！,
-- that no NPC ever calls. it is dead in the legacy data too and has no place to go here

_G.minQuestLevel = 7

_G.magicName = '治愈术'
_G.bookName  = '治愈术'
_G.mijiName  = '治愈术（秘籍）'

_G.teacherMap = '本馆_1_002'
_G.teacherNPC = '清明子_1'

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

    -- checkitem 治愈术 1, the plain book he copies from. he asks for it in two places
    local function postNeedBook(uid)
        uidPostXML(uid, questPath,
        [=[
            <layout>
                <par>有了<t color="red">治愈术魔法书</t>，我可以教你魔法。</par>
                <par></par>
                <par><event id="%s" close="1">结束</event></par>
            </layout>
        ]=], SYS_EXIT)
    end

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼治愈术',

        -- @mugong_heal answered out of flag [715] and stayed clickable either way
        [SYS_CHECKACTIVE] = function(uid)
            if not server.player.hasJob(uid, '道士') then
                return false
            end

            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [715] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你不是已经收到书吗？那么你为什么还要索要？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 治愈术
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我看你正在修炼<t color="red">治愈术</t>。</par>
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
                        <par>如果想熟练治愈术，武功级别最少要达<t color="red">%d</t>级以上。</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], minQuestLevel, SYS_EXIT)
                return
            end

            if not server.player.hasItem(uid, bookName, 1) then
                postNeedBook(uid)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>想学习治愈术的样子。练习武功的过程中将遇到各种困难，我将给你进行详细地说明。</par>
                    <par></par>
                    <par>那么，在给你武功秘籍之前，先对武功进行进行简单的说明吗？</par>
                    <par><event id="npc_lore">拜托了！</event></par>
                    <par><event id="%s" close="1">没有必要</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_heal_next3
        npc_lore = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>治愈术是将消磨尽的<t color="red">自身体力或者别人的体力在瞬间之内使之恢复</t>的武功，是道士最重要的武功。</par>
                    <par></par>
                    <par><event id="npc_take_book">很基础的魔法嘛。</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_heal_next4, take the plain book and hand back the 秘籍
        npc_take_book = function(uid, value)
            if not server.player.hasItem(uid, bookName, 1) then
                postNeedBook(uid)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>你现在已经有治愈术秘籍了，以前不理解的部分也可以理解了。</par>
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
