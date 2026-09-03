-- converted from Envir/QuestDiary/MU_wizard/fireBolt.txt
--
-- the wizard counterpart of 基本剑术 and 治愈术: no trial, no reward, 霹雳尊者 wants you at
-- level 7 carrying the plain 火球术 技能书 and copies it out into a 秘籍
--
-- fireBolt.txt ends with a @mugong_fireball_explain block, 想学习火球术魔法，基本的训练结束后再来找我吧！,
-- that no NPC ever calls. it is dead in the legacy data too and has no place to go here
--
-- the flag [745] branch reads 你还没有收到火球术秘籍吗？ in the legacy text, which is the
-- question inverted — the branch only runs once you have it. kept as written

_G.minQuestLevel = 7

_G.magicName = '火球术'
_G.bookName  = '火球术'
_G.mijiName  = '火球术（秘籍）'

_G.teacherMap = '银杏山谷_02'
_G.teacherNPC = '霹雳尊者_1'

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

    -- checkitem 火球术 1, the plain book he copies from. he asks for it in two places
    local function postNeedBook(uid)
        uidPostXML(uid, questPath,
        [=[
            <layout>
                <par>有了<t color="red">火球术魔法书</t>我就可以教你魔法。</par>
                <par></par>
                <par><event id="%s" close="1">结束</event></par>
            </layout>
        ]=], SYS_EXIT)
    end

    setQuestHandler(questName,
    {
        [SYS_LABEL] = '修炼火球术',

        -- @mugong_fireball answered out of flag [745] and stayed clickable either way
        [SYS_CHECKACTIVE] = function(uid)
            if not server.player.hasJob(uid, '法师') then
                return false
            end

            local state = server.quest.getState(questUID, {uid=uid})
            return (state == nil) or (state == SYS_DONE)
        end,

        [SYS_ENTER] = function(uid, value)
            -- check [745] 1
            if server.quest.getState(questUID, {uid=uid}) == SYS_DONE then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>你还没有收到火球术秘籍吗？</par>
                        <par></par>
                        <par><event id="%s" close="1">结束</event></par>
                    </layout>
                ]=], SYS_EXIT)
                return
            end

            -- checkmagic 火球术
            if server.player.hasMagic(uid, magicName) then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>我看你已经掌握了<t color="red">火球术</t>魔法。</par>
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
                        <par>如果想学习火球术魔法，武功等级最低要达到<t color="red">%d</t>级。</par>
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
                    <par>想要学习火球术的样子。但是像你一样的初学者，在学习武功的过程中将遇到各种困难，我将给你进行详细地说明。现在你已经正式进入了成为魔法师的大门，恭喜你！</par>
                    <par></par>
                    <par>那么在给你秘籍之前，想听对武功的简单说明吗？</par>
                    <par><event id="npc_lore">拜托您了！</event></par>
                    <par><event id="%s" close="1">没有必要了！</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        -- @mugong_fireball_next3
        npc_lore = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>火球术是魔法师的最基本魔法，<t color="red">制作火团</t>攻击远处的敌人。</par>
                    <par></par>
                    <par><event id="npc_take_book">很基础的魔法嘛</event></par>
                </layout>
            ]=])
        end,

        -- @mugong_fireball_next4, take the plain book and hand back the 秘籍
        npc_take_book = function(uid, value)
            if not server.player.hasItem(uid, bookName, 1) then
                postNeedBook(uid)
                return
            end

            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>现在你已经有了火球术秘籍，以前不理解的地方现在都可以理解了。</par>
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
