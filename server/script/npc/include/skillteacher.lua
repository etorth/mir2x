local dialog = require('include.dialog')
-- the 15Magic teachers
--
-- a 技能书 dropped by a monster or bought in a shop is raw material and teaches nothing. a
-- teacher of the right school copies it out into a 秘籍 for a fee, and that is what
-- Player::consumeBook will actually study
--
-- legacy put a 1-in-20 chance on the copy failing, taking the book and the fee with it, see
-- QuestDiary/MU_Total_Sell/<Class>/<skill>.txt
--
--     local skillteacher = require('npc.include.skillteacher')
--
--     skillteacher.setTeacher
--     {
--         greet  = '贫道就是清明子。',
--         ask    = '那，你来找我有什么事？',
--         wrongJob =
--         {
--             ['战士'] = '不过你是战士，你还是去边境城市吧。',
--             ['法师'] = '不过你是魔法师，你还是去银杏山谷吧。',
--         },
--         intro  = '贫道就是清明子，专门在这里指导那些想修炼成为道士的人。',
--
--         -- keyed by school, a teacher may serve more than one
--         books  =
--         {
--             ['道士'] =
--             {
--                 {band = '1 - 10 等级 修炼魔法', list = {{'治愈术', 700}, {'精神力战法', 800}}},
--             },
--         },
--     }

local skillteacher = {}

local FAIL_ODDS = 20

local function manualName(book)
    return book .. '（秘籍）'
end

-- one handler tag per skill, kept stable so a stale XML tag can not land on another skill
local function skillTag(book)
    return 'npc_skill_' .. getItemID(book)
end

function skillteacher.setTeacher(args)
    assertType(args, 'table')
    assertType(args.greet, 'string')
    assertType(args.books, 'table')

    assertType(args.ask, 'string', 'nil')
    assertType(args.intro, 'string', 'nil')
    assertType(args.redName, 'string', 'nil')
    assertType(args.wrongJob, 'table', 'nil')

    -- flatten the level bands into one lookup, the bands are only how the menu reads
    local priceList = {}
    for job, bandList in pairs(args.books) do
        assertType(job, 'string')
        for _, group in ipairs(bandList) do
            assertType(group.band, 'string')
            for _, entry in ipairs(group.list) do
                local book, price = entry[1], entry[2]
                assertType(book, 'string')
                assertType(price, 'integer')

                if getItemID(book) <= 0 then
                    fatalPrintf('Teacher offers unknown book %s', book)
                end

                if getItemID(manualName(book)) <= 0 then
                    fatalPrintf('No 秘籍 exists for %s', book)
                end
                priceList[book] = price
            end
        end
    end

    -- which school's list to show this player, nil when the teacher can not help
    local function servedJob(uid)
        for job, _ in pairs(args.books) do
            if server.player.hasJob(uid, job) then
                return job
            end
        end
        return nil
    end

    local handler = {}

    handler[SYS_ENTER] = function(uid, value)
        if uidQueryRedName(uid) then
            dialog.post(uid, args.redName or '跟你这种人我无话可说。',
            dialog.link(SYS_EXIT, '结束'))
            return
        end

        -- wrong school, point them at the right town
        if not servedJob(uid) then
            local hint
            for job, text in pairs(args.wrongJob or {}) do
                if server.player.hasJob(uid, job) then
                    hint = text
                end
            end

            dialog.post(uid,
            {
                args.intro or args.greet,
                hint,
            },
            dialog.link(SYS_EXIT, '结束'))
            return
        end

        dialog.post(uid,
        {
            args.greet,
            args.ask or '你找我有什么事情吗?',
        },
        {
            dialog.link('npc_show_skills', '寻求武功指导'),
            dialog.link(SYS_EXIT, '结束'),
        })
    end

    handler['npc_show_skills'] = function(uid, value)
        local job = servedJob(uid)
        if not job then
            return
        end

        local text =
        {
            '我可以指导你以下的武功。',
        }
        for _, group in ipairs(args.books[job]) do
            table.insert(text, string.format('（%s）', group.band))

            local line = {}
            for _, entry in ipairs(group.list) do
                table.insert(line, dialog.link(skillTag(entry[1]), entry[1]))
            end
            table.insert(text, table.concat(line, ' , '))
        end

        dialog.post(uid, text,
        dialog.link(SYS_EXIT, '结束'))
    end

    -- ask for the fee, then copy the book out
    for book, price in pairs(priceList) do
        handler[skillTag(book)] = function(uid, value)
            dialog.post(uid, string.format('如果想学%s，请支付<t color="red">%d</t>钱。想得到指教吗？', book, price),
            {
                dialog.link(skillTag(book) .. '_commit', '请写武功秘籍！'),
                dialog.link('npc_hesitate', '结束'),
            })
        end

        handler[skillTag(book) .. '_commit'] = function(uid, value)
            if not server.player.hasItem(uid, book, 1) then
                dialog.post(uid, string.format('请首先拿来%s秘籍。', book),
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            if not server.player.removeGold(uid, price) then
                dialog.post(uid, '世界上的事情没有免费的。修炼武功也是同样的。下次不要忘了带修炼费来。',
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            server.player.removeItem(uid, book, 1)

            -- the fee is already gone, legacy kept it on a botched copy too
            if math.random(FAIL_ODDS) == 1 then
                dialog.post(uid, '哦，非常抱歉！书太旧了，这是无论如何也无法看清楚。请找到保存状态好写的书！',
                dialog.link(SYS_EXIT, '结束'))
                return
            end

            dialog.post(uid,
            '这里有秘诀，请拿着吧！' ..
            '江湖是很冷酷的地方。' ..
            '你千万要专心于一个领域。' ..
            '如果不如此，不要说天下绝世武功，就是成为一名真正的人都很困难。' ..
            '江湖呀。。',
            dialog.link(SYS_EXIT, '结束'))

            server.player.addItem(uid, manualName(book), 1)
        end
    end

    handler['npc_hesitate'] = function(uid, value)
        dialog.post(uid, '嗯。。你犹豫什么？千万记住要学的东西很多，年轻的岁月很短。',
        dialog.link(SYS_EXIT, '结束'))
    end

    setEventHandler(handler)
end

return skillteacher
