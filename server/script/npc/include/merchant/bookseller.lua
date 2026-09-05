local dialogue = require('npc.include.dialogue')
local invop = require('npc.include.invop')

-- 书店, legacy Market_Def/05Book_*.txt (6)
--
-- buys and sells, never repairs. what makes it its own trade is the 关于武功书的说明 topic: a
-- bookseller will talk you through which 武功书 each class can study, which is the one place a
-- new player learns that a plain 技能书 is raw material and only a 秘籍 can be studied
-- (see ItemRecord::isMagicBook)
--
-- that topic is a two-level menu in legacy — @NPC_HelpBooks lists the classes and their books,
-- each book being its own page @NPC_Help_<class><book>. no two booksellers explain the same set:
--
--   05Book_Bichon-0    all seven, 战士 + 魔法师 + 道士
--   05Book_Kugkyung-01 基本剑术 and 半月弯刀 only, a warrior-book shop, and it hangs the menu
--                      off @NPC_QuestionPrize under a different label — 询问有关物品的事
--   05Book_Eunhang-02  火球术 and 霹雳掌 only, a mage-book shop
--
-- so the pages live here, keyed by book, and each NPC lists the books it will explain:
--
--     local bookseller = require('npc.include.merchant.bookseller')
--     bookseller.setBookseller
--     {
--         greet = {'欢迎光临，你来买练武功的书？'},
--         goods = {'基本剑术', '火球术'},
--         books = {'基本剑术', '半月弯刀'},   -- the 关于武功书的说明 menu, in legacy's order
--         booksLabel  = '询问',               -- 聆听 for most
--         booksSuffix = '有关物品的事',       -- 关于武功书的说明 for most
--     }

-- false means this shop explicitly does not do it; nil means fall back to the trade's norm
local function optList(value, default)
    if value == false then
        return nil
    end
    return value or default
end

local bookseller = {}

-- @NPC_Help_11 .. @NPC_Help_33. legacy writes the level thresholds into the prose, so they are
-- transcribed as prose rather than recomputed — the book records carry no such requirement yet
--
-- tag is the legacy label this page came from, kept so the provenance is greppable
--
-- every one of these pages ends with <前一步/@helpbooks>, which is dangling in the two shops that
-- hold the menu elsewhere — going back would have dropped the player out of the conversation.
-- here 前一步 returns to whichever menu opened the page, which is what the button plainly means
bookseller.BOOK_HELP =
{
    {name = '基本剑术',   class = '战士',   tag = 'NPC_Help_11', level = {7, 11, 16}},
    {name = '半月弯刀',   class = '战士',   tag = 'NPC_Help_12', level = {14, 16, 18}},
    {name = '火球术',     class = '魔法师', tag = 'NPC_Help_21', level = {7, 11, 16}},

    {name = '霹雳掌',     class = '魔法师', tag = 'NPC_Help_22', level = {8, 10, 12},
        text = {'<t color="red">霹雳掌</t>1级修炼可在8等级, 2级修炼可在10等级, 3级修炼可在12等级开始。'}},

    {name = '治愈术',     class = '道士',   tag = 'NPC_Help_31', level = {7, 11, 16}},
    {name = '精神力战法', class = '道士',   tag = 'NPC_Help_32', level = {8, 10, 12}},
    {name = '施毒术',     class = '道士',   tag = 'NPC_Help_33', level = {12, 14, 16}},
}

local function findBook(name)
    for _, book in ipairs(bookseller.BOOK_HELP) do
        if book.name == name then
            return book
        end
    end
    fatalPrintf('no book help page for %s, add it to bookseller.BOOK_HELP', name)
end

function bookseller.setBookseller(spec)
    assertType(spec, 'table')
    assertType(spec.greet, 'table', 'function')
    assert(type(spec.greet) == 'function' or #spec.greet > 0, 'empty bookseller greeting')

    local label = spec.label or '图书'
    local trade = optList(spec.trade, {'技能书'})
    local price = spec.price or 50
    local back = {dialogue.link(SYS_ENTER, spec.backLabel or '前一步')}
    local menu = {}
    local handler = {}

    if spec.goods then
        setNPCSell(spec.goods)
        table.insert(menu, dialogue.link('npc_buy', spec.buyLabel or '购买', spec.buySuffix or label))
        handler.npc_buy = function(uid, value)
            dialogue.post(uid, spec.buyText or {'请挑选你想要的书。'}, back)
            uidPostSell(uid)
        end
    end

    if trade then
        table.insert(menu, dialogue.link('npc_sell', spec.sellLabel or '出售', spec.sellSuffix or label))
        handler.npc_sell = function(uid, value)
            dialogue.post(uid, spec.sellText or {'请把要出售的书放在上面。'}, back)
            invop.uidStartTrade(uid, 'npc_sell_query', 'npc_sell_commit', trade)
        end
        handler.npc_sell_query = function(uid, value)
            invop.postQueryTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price)
        end
        handler.npc_sell_commit = function(uid, value)
            invop.postCommitTrade(uid, value, 'npc_sell_query', 'npc_sell_commit', trade, price)
        end
    end

    -- 关于武功书的说明
    if spec.books then
        assertType(spec.books, 'table')

        local bookList = {}
        for _, name in ipairs(spec.books) do
            table.insert(bookList, findBook(name))
        end

        -- the classes in the order this shop's books introduce them, so a warrior-book shop
        -- opens on 战士 and never mentions the other two
        local classList = {}
        local classBook = {}

        for _, book in ipairs(bookList) do
            if not classBook[book.class] then
                classBook[book.class] = {}
                table.insert(classList, book.class)
            end
            table.insert(classBook[book.class], book)
        end

        local function bookTag(book)
            return string.format('npc_book_%s', string.lower(book.tag))
        end

        for _, book in ipairs(bookList) do
            handler[bookTag(book)] = function(uid, value)
                local text = spec.bookHelp and spec.bookHelp[book.name] or book.text or
                {
                    string.format(
                        '等级为%d时可以修炼<t color="red">%s</t>的第一阶段， 等级为%d时修炼第2阶段，%d级时可以完成第3阶段的修炼。',
                        book.level[1], book.name, book.level[2], book.level[3]),
                }
                dialogue.post(uid, text, {dialogue.link('npc_book_explain', '前一步')})
            end
        end

        table.insert(menu, dialogue.link('npc_book_explain', spec.booksLabel or '聆听', spec.booksSuffix or '关于武功书的说明'))
        handler.npc_book_explain = function(uid, value)
            local text = {}
            local intro = spec.booksText or '你想听哪类书的介绍？'
            for _, line in ipairs(type(intro) == 'table' and intro or {intro}) do
                table.insert(text, line)
            end
            local explanations = {}
            for _, class in ipairs(classList) do
                local option = {}
                for _, book in ipairs(classBook[class]) do
                    table.insert(option, dialogue.link(bookTag(book), book.name))
                end
                local last = table.remove(option)
                local books = #option > 0 and table.concat(option, ',') .. '和' .. last or last
                table.insert(explanations, string.format('%s可以学习%s', class, books))
            end
            table.insert(text, table.concat(explanations, '，'))
            dialogue.post(uid, text, back)
        end
    end

    for _, topic in ipairs(spec.topics or {}) do
        table.insert(menu, dialogue.link(topic.id, topic.label, topic.suffix, topic.prefix))
        handler[topic.id] = topic.handler or function(uid, value)
            dialogue.post(uid, topic.text, {dialogue.link(SYS_ENTER, topic.back or spec.backLabel or '前一步')})
        end
    end

    if spec.today then
        table.insert(menu, dialogue.link('npc_today', '对今日的任务进行了解'))
        handler.npc_today = function(uid, value)
            dialogue.post(uid, {spec.today}, {dialogue.link(SYS_EXIT, spec.todayExit or '结束')})
        end
    end

    table.insert(menu, dialogue.link(SYS_EXIT, spec.exitLabel or '结束'))
    handler[SYS_ENTER] = function(uid, value)
        dialogue.post(uid, spec.greet, menu)
    end

    for tag, callback in pairs(spec.extra or {}) do
        handler[tag] = callback
    end
    for tag, callback in pairs(handler) do
        if type(callback) == 'function' and tag ~= SYS_LABEL and tag ~= SYS_HIDE and tag ~= SYS_CHECKACTIVE and tag ~= SYS_ALLOWREDNAME then
            handler[tag] = dialogue.guardRedName(callback, spec.redName, spec.redNameExit or '结束')
        end
    end
    handler[SYS_ALLOWREDNAME] = true
    setEventHandler(handler)
    return handler
end

return bookseller
