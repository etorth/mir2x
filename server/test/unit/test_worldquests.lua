local root = (arg[1] and (arg[1] .. '/') or '') .. 'server/script/quest/'
local names = {'比奇商会', '半兽人任务', '堕落道士任务', '沃玛教主任务', '石母任务', '被盗灵魂任务', '千年毒蛇任务'}
local count = 0
local function check(v, text)
    assert(v, text)
    count = count + 1
end
local function harness(name)
    local env = setmetatable({}, {__index=_G})
    env._G = env
    for _, key in ipairs({'SYS_ENTER', 'SYS_DONE', 'SYS_EXIT', 'SYS_LUANIL', 'SYS_CHECKACTIVE', 'SYS_EPUID', 'SYS_EPQST', 'SYS_LABEL', 'SYS_HIDE', 'SYS_ON_APPEAR', 'SYS_ON_OFFLINE', 'SYS_ON_KILL'}) do env[key] = key end
    env.SYS_GOLDNAME = '金币'
    env.SYS_QSTFSM = 'main'
    local h = {env=env, name=name, fsms={}, state={}, handlers={}, behavior={}, inventory={}, flags={}, qflags={}, descriptions={}, gold=0, level=30, job='战士', drops={}}
    local context
    local function code(text, ...)
        return assert(load(text, 'embedded:' .. name, 't', env))(...)
    end
    local function npcKey(map, npc) return map .. '/' .. npc end
    env.getNPCharUID = npcKey
    env.getUID = function() return context or 'quest' end
    env.getQuestName = function() return name end
    env.getNPCMapName = function() return '比奇县_0' end
    env.getNPCName = function() return '杂货商_1' end
    env.getLevel = function() return h.level end
    env.getMapLoc = function() return 1, 1 end
    env.dbHasFlag = function(flag) return h.flags[flag] end
    env.dbAddFlag = function(flag) h.flags[flag] = true end
    env.hasQuestFlag = function(_, flag) return h.qflags[flag] end
    env.addQuestFlag = function(_, flag) h.qflags[flag] = true end
    env.dbGetQuestState = function(_, fsm) return h.state[fsm or 'main'] end
    env.setQuestDesp = function(a) h.descriptions[a.fsm or 'main'] = a[1] end
    env.setQuestFSMTable = function(a,b) h.fsms[b and a or 'main'] = b or a end
    env.setQuestState = function(a)
        local fsm = a.fsm or 'main'
        if a.from then
            local accepted = false
            for _, from in ipairs(type(a.from) == 'table' and a.from or {a.from}) do
                if h.state[fsm] == (from == 'SYS_LUANIL' and nil or from) then accepted = true end
                if from == 'SYS_LUANIL' and h.state[fsm] == nil then accepted = true end
            end
            if not accepted then return false end
        end
        h.state[fsm] = a.state
        if h.fsms[fsm] and h.fsms[fsm][a.state] then h.fsms[fsm][a.state](a.uid, a.args) end
        return true
    end
    env.uidRemoteCall = function(target,...)
        local args = table.pack(...)
        local previous = context
        context = target
        local out = table.pack(code(args[args.n], table.unpack(args, 1, args.n-1)))
        context = previous
        return table.unpack(out,1,out.n)
    end
    env.setQuestHandler = function(_, handler) h.handlers[context] = handler end
    env.setupNPCQuestBehavior = function(map,npc,uid,argstr,text)
        local args
        if text then args = table.pack(code(argstr)) else text=argstr; args={n=0} end
        h.behavior[npcKey(map,npc)] = code(text, table.unpack(args,1,args.n))
    end
    env.setupMapUIDGridTrigger = function() end
    env.setupMapGridTrigger = function() end
    env.setupInstanceUIDGridTrigger = function() end
    env.addTrigger = function() end
    env.addQuestTrigger = function() end
    env.clearNPCQuestBehavior = function() end
    env.isNightTime = function() return h.night end
    env.runEventHandler = function(uid, _, key) h.current[key](uid) end
    local dialog = {link=function(key,text,args) return {key=key,text=text,args=args} end, post=function(...) h.lastDialog=table.pack(...) end}
    env.require = function(module)
        if module=='include.dialog' then return dialog end
        if module=='quest.include.mondrop' then return {addDropTrigger=function(_, drops) for _,d in ipairs(drops) do h.drops[#h.drops+1]=d end end} end
        if module=='quest.include.npcbattle' then return {} end
        error(module)
    end
    env.server = {player={}, quest={}}
    local p = env.server.player
    p.getGold=function() return h.gold end
    p.getLevel=function() return h.level end
    p.getName=function() return '测试侠客' end
    p.hasJob=function(_,job) return h.job==job end
    p.dbHasFlag=function(_,flag) return h.flags[flag] end
    p.dbAddFlag=function(_,flag) h.flags[flag]=true end
    p.getQuestState=function(_,quest) return h.externalState and h.externalState[quest] end
    p.hasItem=function(_,item,n) return (h.inventory[item] or 0)>=(n or 1) end
    p.addItem=function(_,item,n) if item=='金币' then h.gold=h.gold+n else h.inventory[item]=(h.inventory[item] or 0)+n end end
    p.removeGold=function(_,n) if h.gold<n then return false end; h.gold=h.gold-n; return true end
    p.removeItem=function(_,item,n)
        if not p.hasItem(1,item,n) then return false end
        h.inventory[item]=h.inventory[item]-n
        return true
    end
    p.removeUpToItem=function(_,item,n) local taken=math.min(h.inventory[item] or 0,n); h.inventory[item]=(h.inventory[item] or 0)-taken; return taken end
    p.spaceMove=function(_,...) h.move=table.pack(...) end
    env.server.quest.setState=function(_,a) return env.setQuestState(a) end
    env.server.quest.getState=function(_,a) return h.state[a.fsm or 'main'] end
    assert(loadfile(root..name..'.lua', 't', env))()
    h.enter=function(state, fsm)
        h.state[fsm or 'main']=state
        return h.fsms[fsm or 'main'][state](1)
    end
    h.click=function(npc,event,args,map)
        local key=npcKey(map or '比奇县_0',npc)
        h.current=h.behavior[key] or h.handlers[key]
        assert(h.current, key)
        return h.current[event](1,args)
    end
    return h
end

for _,name in ipairs(names) do
    local h=harness(name)
    check(h.fsms.main.SYS_DONE~=nil,name..' terminal description')
    h.fsms.main.SYS_DONE(1)
    check(h.descriptions.main and #h.descriptions.main>0,name..' completed journal')
end

-- the last dialog posted: said() finds a line holding text, linked() a link to the event
local function said(h,text)
    local texts=h.lastDialog and h.lastDialog[3]
    if type(texts)=='string' then return texts:find(text,1,true)~=nil end
    for _,line in ipairs(texts or {}) do if line:find(text,1,true) then return true end end
    return false
end
local function linked(h,key)
    local links=h.lastDialog and h.lastDialog[4]
    if links and links.key then links={links} end
    for _,link in ipairs(links or {}) do if link.key==key then return true end end
    return false
end
local function talk(h,npc,event)
    h.lastDialog=nil
    h.click(npc,event)
end

local h=harness('比奇商会')
local libFSM,phaFSM,expFSM=h.env.fsmName_persuade_librarian,h.env.fsmName_persuade_pharmacist,h.env.fsmName_expand_coc
h.enter('quest_persuade_pharmacist_and_librarian')
local news=h.handlers['比奇县_0/世玉_1']
check(not news.SYS_CHECKACTIVE(1),'no 世玉 news before the librarian or the pharmacist joined')
h.state[libFSM]='SYS_DONE'
check(news.SYS_CHECKACTIVE(1),'世玉 news once the librarian joined')
check(news.SYS_LABEL(1)=='比奇省商界','@NPC_Main_1 label for the news')
talk(h,'世玉_1','SYS_ENTER')
check(said(h,'荣阵阁图书管理人已经加入了比奇商会！') and h.state[expFSM]==nil,'wang_20 the librarian only, no expansion yet')
h.state[libFSM]=nil; h.state[phaFSM]='SYS_DONE'
talk(h,'世玉_1','SYS_ENTER')
check(said(h,'听说一小会儿之前，药剂师也加入了比奇商会。') and h.state[expFSM]==nil,'wang_21 the pharmacist only, no expansion yet')
h.state[libFSM]='SYS_DONE'
talk(h,'王大人_1','SYS_ENTER')
check(linked(h,'npc_give_bonus') and not said(h,'世玉'),'王大人 offers the first reward with no invented hint')
talk(h,'世玉_1','SYS_ENTER')
check(said(h,'好像是个从乡下来的人帮助了王大人扩大了比奇商会的势力。') and h.state[expFSM]=='quest_expansion_active','wang_23 starts the expansion, 152')
check(news.SYS_LABEL(1)=='传奇商会','@NPC_Main_2 label once the expansion started')
talk(h,'世玉_1','SYS_ENTER')
check(said(h,'这次由于比奇商会的势力扩张，崔大夫的传奇商会受到重创') and h.state[expFSM]=='quest_expansion_active','wang_24 once the expansion started')
check(h.behavior['比奇县_0/杂货商_1'] and h.behavior['比奇县_0/恩实_1'] and h.behavior['比奇县_0/怡美_1'],'all expansion merchants installed')
talk(h,'恩实_1','SYS_ENTER')
check(said(h,'跟我说这样的话之前，没有什么想先去找王大人跟他说的话吗？'),'wang_51 jeweler before the grocer and the first reward')
talk(h,'怡美_1','SYS_ENTER')
check(said(h,'但是首先我有点拜托你去办的事儿。加入比奇商会的事儿下次再说吧！'),'wang_67 outfitter before the first reward')
h.level=10
talk(h,'怡美_1','SYS_ENTER')
check(said(h,'不过好像跟你这种等级还没有达到11的后生小子没什么可说的。'),'wang_68 outfitter below level 11')
h.level=30

talk(h,'王大人_1','npc_give_bonus')
check(h.gold==5000 and h.flags.done_wang_coc and said(h,'这个虽然菲薄但是我的诚意，请收下吧！'),'wang_15 with the first 5000, original 168')
check(h.state.main=='quest_first_stage_done' and not h.flags.done_wang_coc_expansion,'168 distinct from 169')
check(h.state[expFSM]=='quest_expansion_active' and news.SYS_CHECKACTIVE(1),'expansion and wang_24 stay after 168')
talk(h,'王大人_1','SYS_ENTER')
check(said(h,'现在那个姓崔的家伙的传奇商会就要完蛋啦！') and h.state.main=='quest_first_stage_done','wang_16 before the grocer joined')
talk(h,'恩实_1','SYS_ENTER')
check(said(h,'但仍不能信任还没能收买') and said(h,'测试侠客'),'wang_52 jeweler after 168 before the grocer')

talk(h,'杂货商_1','SYS_ENTER')
check(said(h,'让我去加入比奇商会？呵呵！人啊！要讲信义才行。') and linked(h,'npc_think'),'wang_25 grocer opener')
talk(h,'杂货商_1','npc_think')
check(linked(h,'npc_money') and linked(h,'npc_leave'),'wang_26 two routes')
talk(h,'杂货商_1','npc_money')
check(said(h,'.........') and linked(h,'npc_suggest'),'wang_27')
talk(h,'杂货商_1','npc_suggest')
check(said(h,'...............') and linked(h,'npc_offer'),'wang_28')
talk(h,'杂货商_1','npc_offer')
check(said(h,'唔! 1万5千钱左右怎么样') and linked(h,'npc_pay15000') and linked(h,'npc_pay12000') and linked(h,'npc_refuse'),'wang_29 three offers, legacy digits')
h.gold=15000
talk(h,'杂货商_1','npc_pay15000')
check(h.gold==0 and h.qflags.merchant_grocery and said(h,'明白了，那么我会加入比奇商会的。'),'wang_31 15000 route')
talk(h,'杂货商_1','SYS_ENTER')
check(said(h,'替我向王大人转达我的意思吧！'),'wang_50 after the grocer joined')
talk(h,'王大人_1','SYS_ENTER')
check(said(h,'罢了') and h.state.main=='quest_first_stage_done','wang_17 before the jeweler joined')

talk(h,'恩实_1','SYS_ENTER')
check(said(h,'是来说服我加入比奇商会的吧！') and linked(h,'npc_discuss'),'wang_54 jeweler')
talk(h,'恩实_1','npc_discuss')
check(said(h,'你是说杂货商已经加入到比奇商会了？') and linked(h,'npc_request'),'wang_55')
talk(h,'恩实_1','npc_request')
check(h.qflags.jeweler_request and said(h,'新鲜玩艺儿'),'wang_56 sets 156')
talk(h,'恩实_1','SYS_ENTER')
check(said(h,'这不是什么新鲜玩艺儿啊！你在耍我吗？') and not linked(h,'npc_join'),'wang_64 without a curiosity')
h.inventory['灵魂明珠']=1; h.inventory['角笛']=1
talk(h,'恩实_1','SYS_ENTER')
check(said(h,'地位很高的半兽人战士在指挥半兽人们时使用的笛子') and linked(h,'npc_join'),'wang_57 the first curiosity in @SHOW_BULSA order')
h.inventory['角笛']=0
talk(h,'恩实_1','SYS_ENTER')
check(said(h,'散射出若隐若现光芒的玉石'),'wang_63 for the 灵魂明珠')
talk(h,'恩实_1','npc_join')
check(h.qflags.merchant_jeweler and h.inventory['灵魂明珠']==1 and said(h,'崔大夫？嗯，管它呢！'),'wang_65 jeweler joins, keeps nothing')
talk(h,'恩实_1','SYS_ENTER')
check(said(h,'那么，请你去转告王大人我会加入比奇商会的！'),'wang_66 after the jeweler joined')
talk(h,'王大人_1','SYS_ENTER')
check(said(h,'算了') and h.state.main=='quest_first_stage_done','wang_18 before the outfitter joined')

talk(h,'怡美_1','SYS_ENTER')
check(said(h,'要是真的有话和我说就先帮我一把') and not linked(h,'npc_discuss'),'kyunggap_5 until 轻型盔甲 is done, 170')
h.externalState={['轻型盔甲任务']='SYS_DONE'}
talk(h,'怡美_1','SYS_ENTER')
check(said(h,'又是来劝我加入比奇商会的吧') and said(h,'测试侠客') and linked(h,'npc_discuss'),'wang_69 after 轻型盔甲')
talk(h,'怡美_1','npc_discuss')
check(said(h,'就像其它商人一样，我也有个条件。') and linked(h,'npc_condition'),'wang_70')
talk(h,'怡美_1','npc_condition')
check(said(h,'使用地牢逃脱卷不但没能逃会城里') and linked(h,'npc_request'),'wang_71')
talk(h,'怡美_1','npc_request')
check(h.qflags.outfitter_request and said(h,'回城卷 6个'),'wang_72 sets 158')
h.inventory['回城卷']=5
talk(h,'怡美_1','SYS_ENTER')
check(not h.qflags.merchant_outfitter and h.inventory['回城卷']==5 and said(h,'我就会听从你的劝说。'),'wang_73 short of six scrolls')
h.inventory['回城卷']=6
talk(h,'怡美_1','SYS_ENTER')
check(h.qflags.merchant_outfitter and h.inventory['回城卷']==0 and said(h,'替我转告王大人一声吧！'),'wang_74 takes six scrolls')
talk(h,'怡美_1','SYS_ENTER')
check(said(h,'就替我转告他我会加入比奇商会的！'),'wang_75 after the outfitter joined')

talk(h,'王大人_1','SYS_ENTER')
check(h.gold==25000 and h.flags.done_wang_coc and h.flags.done_wang_coc_expansion and h.state.main=='SYS_DONE','expansion final 25000 and flags 168 169')
check(said(h,'由于你的活动终于使我们比奇商会统一了比奇地区商权。'),'wang_19 after the switch')
check(h.descriptions.main=='王大人召集了所有比奇商人，掌握了比奇商界。','169 terminal journal')

h=harness('比奇商会'); h.enter('SYS_ENTER',expFSM); h.state.main='quest_first_stage_done'; h.externalState={['轻型盔甲任务']='SYS_DONE'}
talk(h,'怡美_1','SYS_ENTER')
check(said(h,'连 <t color="red">杂货商</t>都还没有加入比奇商会') and not linked(h,'npc_discuss'),'kyunggap_27 outfitter waits for the grocer')

h=harness('比奇商会'); h.enter('quest_persuade_pharmacist_and_librarian')
h.state[libFSM]='SYS_DONE'; h.state[phaFSM]='SYS_DONE'
talk(h,'王大人_1','npc_give_bonus')
check(h.state.main=='SYS_DONE' and h.gold==5000 and h.flags.done_wang_coc,'first reward with no expansion ends the quest')
check(not h.handlers['比奇县_0/世玉_1'].SYS_CHECKACTIVE(1),'no expansion once 168 is paid without it')

for _,pair in ipairs({{10000,'npc_pay10000','明白了，那么我会加入比奇商会的。不过'},{12000,'npc_pay12000','我想通了'}}) do
    h=harness('比奇商会'); h.enter('SYS_ENTER',expFSM)
    h.gold=pair[1]; talk(h,'杂货商_1',pair[2])
    check(h.gold==0 and h.qflags.merchant_grocery and said(h,pair[3]),'negotiated price '..pair[1])
end
h=harness('比奇商会'); h.enter('SYS_ENTER',expFSM)
h.gold=14999; talk(h,'杂货商_1','npc_pay15000')
check(h.gold==14999 and h.qflags.grocery_refused and not h.qflags.merchant_grocery and said(h,'这种豪爽的性格的确出乎我的意料啊！'),'wang_30 short of 15000 sets 154')
talk(h,'杂货商_1','SYS_ENTER')
check(said(h,'我没有离开传奇商会的打算，你还是赶紧回去吧！') and linked(h,'npc_reconsider'),'wang_46 the refused grocer, 23 for the missing 21')
talk(h,'杂货商_1','npc_reconsider')
check(linked(h,'npc_pay20000'),'wang_47')
h.gold=29999; talk(h,'杂货商_1','npc_pay20000')
check(h.gold==29999 and not h.qflags.merchant_grocery and said(h,'立刻给我滚开！'),'wang_48 check 30000 not 20000')
h.gold=30000; talk(h,'杂货商_1','npc_pay20000')
check(h.gold==10000 and h.qflags.merchant_grocery and said(h,'两万钱就足够了。'),'wang_49 check 30000 take 20000')
for _,pair in ipairs({{'npc_refuse','不要再和我提起这件事儿了！'},{'npc_refuse_7000','不要再和我提起这件事儿。'}}) do
    h=harness('比奇商会'); h.enter('SYS_ENTER',expFSM)
    talk(h,'杂货商_1',pair[1])
    check(h.qflags.grocery_refused and said(h,pair[2]),'low offer '..pair[1]..' sets 154')
end
h=harness('比奇商会'); h.enter('SYS_ENTER',expFSM)
talk(h,'杂货商_1','npc_friend')
check(h.qflags.grocery_friend and said(h,'和你这个朋友很投缘啊'),'wang_40 sets 153')
talk(h,'杂货商_1','SYS_ENTER')
check(said(h,'心一急，嗓子就有点干'),'wang_41 without a soju')
h.inventory['烧酒']=1
talk(h,'杂货商_1','SYS_ENTER')
check(h.inventory['烧酒']==0 and linked(h,'npc_drink'),'wang_42 takes one soju')
talk(h,'杂货商_1','npc_drink'); talk(h,'杂货商_1','npc_drink_done'); talk(h,'杂货商_1','npc_join')
check(h.qflags.merchant_grocery and h.gold==0 and said(h,'那么我就会加入比奇商会的。'),'wang_45 friendship joins, no gold')

-- [146]: a wrong answer is a state of the librarian's fsm, the guards tell their stories again, guard 3 by
-- quest_guard_3_give_info with the replay line as its args, the librarian asks again from the first question
h=harness('比奇商会')
h.env.shuffleArray=function(t) return t end
local cleared={}
h.env.clearNPCQuestBehavior=function(_,npc) cleared[npc]=true end
h.enter('quest_answer_librarian_questions',libFSM)
h.click('图书管理员_1','npc_question_2','1')
check(h.state[libFSM]=='quest_player_answer_incorrectly' and h.descriptions[libFSM]=='你没能回答图书管理员的问题，再去找所有休班卫士听他们的故事吧。','a wrong answer goes to quest_player_answer_incorrectly, [146]')
talk(h,'休班卫士_1','SYS_ENTER')
check(said(h,'但是没那么容易啦！'),'@REPLAY_GUARD1')
talk(h,'休班卫士_2','SYS_ENTER')
check(said(h,'累得要命，你可要仔细听好并记住啊！') and said(h,'这就是现在的比奇省。'),'@REPLAY_GUARD2 tells the story again')
-- quest_guard_3_give_info posts with no quest path, its lines are the second argument
local function story(h) local t=h.lastDialog and h.lastDialog[2]; return type(t)=='table' and table.concat(t,'|') or '' end
talk(h,'休班卫士_3','SYS_ENTER')
check(story(h):find('哦？那我就再给你讲一遍吧！',1,true) and story(h):find('每一寸土地都是用鲜血换来的啊！',1,true) and not story(h):find('这已经是我所知道的全部故事啦',1,true),'@REPLAY_GUARD3 is quest_guard_3_give_info with the replay line')
check(h.state[libFSM]=='quest_answer_librarian_questions','the replay of guard 3 goes back to the questions')
talk(h,'休班卫士_3','SYS_ENTER')
check(story(h):find('哦？那我就再给你讲一遍吧！',1,true) and h.state[libFSM]=='quest_answer_librarian_questions','guard 3 tells it again while the questions wait')
h.click('图书管理员_1','npc_question_3','2')
check(h.state[libFSM]=='quest_player_answer_incorrectly','a wrong answer again')
talk(h,'图书管理员_1','SYS_ENTER')
check(h.state[libFSM]=='quest_answer_librarian_questions','@BICHUN_TEST2, the librarian asks again')
talk(h,'图书管理员_1','npc_question_1')
check(linked(h,'npc_question_2'),'the questions again from the first one')
h.click('图书管理员_1','npc_question_2','3'); h.click('图书管理员_1','npc_question_3','3'); h.click('图书管理员_1','npc_done_question','1')
check(h.state[libFSM]=='SYS_DONE' and cleared['休班卫士_1'] and cleared['休班卫士_2'] and cleared['休班卫士_3'],'the librarian joins, the guards go back to their own talk')

h=harness('堕落道士任务')
h.enter('quest_return_token')
h.inventory['不死牌']=1
h.click('比奇城城主_1','npc_hand_token')
check(h.gold==20000 and h.inventory['白虎剑']==1 and h.inventory['战神油']==2,'fallen final rewards')
h=harness('堕落道士任务'); h.enter('quest_collect_bones')
check(h.drops[1].counterMode.threshold==3 and h.drops[2].counterMode.threshold==3,'separate deterministic bone counters')
check(h.drops[1].counterMode.completed==5 and h.drops[2].counterMode.completed==5,'source bone completion counter5')
for _,bones in ipairs({1,10,12}) do
    h=harness('堕落道士任务'); h.enter('quest_collect_bones')
    h.inventory['僧侣僵尸骨']=bones; h.inventory['雷电僵尸骨']=bones
    h.click('书堂玄震_1','npc_take_charm',nil,'道馆_1')
    check(h.inventory['毁灭护身符']==1 and h.inventory['僧侣僵尸骨']==math.max(0,bones-10) and h.inventory['雷电僵尸骨']==math.max(0,bones-10),'source check1 take-up-to10 '..bones)
end

for _,job in ipairs({'战士','法师','道士'}) do
    local weapon=job=='战士' and '修罗' or job=='法师' and '偃月' or '降魔'
    -- @UMKING_MUMYUNG13_6 gives nothing, only @UMKING_MUMYUNG13_7, the refusal, gives the weapon
    h=harness('沃玛教主任务'); h.job=job; h.enter('quest_smash_orb')
    h.click('无名老人_1','npc_take_job',nil,h.env.hermitMap)
    check(h.state.main=='quest_kill_king' and h.gold==0,'accept goes on to quest_kill_king '..job)
    for _,other in ipairs({'修罗','偃月','降魔'}) do check((h.inventory[other] or 0)==0,'accept gives no ordinary weapon '..job) end
    h=harness('沃玛教主任务'); h.job=job; h.enter('quest_smash_orb')
    h.click('无名老人_1','npc_decline_job',nil,h.env.hermitMap)
    check(h.state.main=='SYS_DONE' and h.gold==0,'decline ends the quest '..job)
    for _,other in ipairs({'修罗','偃月','降魔'}) do check((h.inventory[other] or 0)==(other==weapon and 1 or 0),'decline gives the one job weapon '..job) end
    h=harness('沃玛教主任务'); h.job=job; h.enter('quest_king_dead'); h.click('无名老人_1','npc_comfort',nil,h.env.hermitMap)
    check(h.inventory[job=='战士' and '沃玛修罗' or job=='法师' and '沃玛偃月' or '沃玛降魔']==1,'special one job weapon '..job)
end
for _,stage in ipairs({0,168,169}) do
    h=harness('沃玛教主任务'); h.enter('quest_sell_medal'); h.inventory['沃玛金牌']=1
    h.flags.done_wang_coc=stage>=168; h.flags.done_wang_coc_expansion=stage==169
    h.click('王大人_1','npc_sell')
    check(h.gold==(stage==169 and 200000 or stage==168 and 100000 or 50000),'medal exact flag tier'..stage)
end
h=harness('沃玛教主任务'); h.enter('SYS_ENTER')
check(not h.drops[1].kills and not h.drops[1].counterMode,'medal direct counter')
h=harness('沃玛教主任务'); h.enter('quest_find_orb')
check(not h.drops[1].kills and not h.drops[1].counterMode,'orb direct counter')
h=harness('石母任务'); h.level=6; h.click('石母_1','SYS_ENTER',nil,'比奇县_0_003')
check(not h.lastDialog,'stone level6 blocked')
h.level=7; h.click('石母_1','SYS_ENTER',nil,'比奇县_0_003')
check(h.lastDialog~=nil,'stone level7 admitted')
h.enter('SYS_ENTER')
for _,night in ipairs({false,true}) do h.night=night; h.click('老生_1','SYS_ENTER'); check(h.lastDialog[3]=='深更半夜的什么事啊？你来找我有什么事？','original day/night text') end
h=harness('半兽人任务'); h.externalState={['比奇商会']='quest_first_stage_done'}; h.flags.done_wang_coc=true
h.click('比奇城城主_1','SYS_ENTER')
check(h.lastDialog~=nil,'half-orc accepts168 while expansion active')
h.enter('quest_hunt_warrior')
check(h.drops[1].moveTo[1]=='半兽洞穴1层_D001' and h.drops[1].moveTo[2]==303 and h.drops[1].moveTo[3]==70,'half-orc boss source return coordinates')

local function dropHarness(mode)
    local h=harness('比奇商会')
    local vars={}
    h.env.dbGetQuestVar=function(_,key) return vars[key] end
    h.env.dbSetQuestVar=function(_,key,value) vars[key]=value end
    h.env.math=setmetatable({random=function() return 1 end},{__index=math})
    local real=assert(loadfile(root..'include/mondrop.lua','t',h.env))()
    local runDrop
    for i=1,30 do
        local name,value=debug.getupvalue(real._runDropOnKill,i)
        if not name then break end
        if name=='runDrop' then runDrop=value end
    end
    assert(runDrop)
    local drop={map={},need={},exclude={},give={{'测试物品',1}},take={},chance=1,kills=1,counterMode=mode,counter='source_counter'}
    return h,vars,function() return runDrop(1,drop,'test') end
end
for _,pair in ipairs({
    {{threshold=10,initial=3,incrementChance=2,randomFrom=9},10},
    {{threshold=5,initial=3,incrementChance=2,randomFrom=4},5},
    {{threshold=12,initial=3,incrementChance=2,randomFrom=11},12},
    {{threshold=3,initial=3,completed=5},3},
}) do
    local h,vars,kill=dropHarness(pair[1])
    for i=1,pair[2]-1 do check(not kill(),'no early progressive counter reward '..i) end
    check(kill() and h.inventory['测试物品']==1,'actual helper source counter completion')
    if pair[1].completed then
        check(vars.source_counter==5,'bone counter stays5')
        h.inventory['测试物品']=0
        check(not kill() and h.inventory['测试物品']==0,'spent/lost bone does not recreate source drop')
    else check(vars.source_counter==nil,'normal progressive counter reset') end
end
local h,vars,kill=dropHarness({threshold=5,initial=3,incrementChance=2,randomFrom=4})
kill();kill()
h.env.math.random=function() return 2 end
kill()
check(vars.source_counter==4,'random ramp can stall at source threshold')
print(string.format('World quest mocked tests: %d assertions passed',count))
