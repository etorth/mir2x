local dialog = require('include.dialog')

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        dialog.post(uid, string.format('客官%s你好我是%s，我可以给你展示系统所有的账号！<emoji id="0"/>', uidQueryName(uid), getNPCName()),
        {
            dialog.link('npc_goto_1', '展示'),
            dialog.link(SYS_EXIT, '关闭'),
        })
    end,

    ["npc_goto_1"] = function(uid, value)
        local result = dbQuery('select * from tbl_account')
        local text =
        {
            '数据库玩家账号有：',
            '',
        }
        for _, row in ipairs(result) do
            table.insert(text, string.format('fld_id: %d, fld_account: %s', row.fld_dbid, row.fld_account))
        end

        local clickCount = argDefault(dbGetGKey('click_count'), 0)

        table.insert(text, '')
        table.insert(text, '上次查询：')
        table.insert(text, string.format('fld_float：%f', argDefault(uidDBGetKey(uid, 'fld_float'), 0.0)))
        table.insert(text, string.format('fld_integer：%d', argDefault(uidDBGetKey(uid, 'fld_integer'), 0)))
        table.insert(text, string.format('fld_text：%s', argDefault(uidDBGetKey(uid, 'fld_text'), '(nil)')))
        table.insert(text, '')
        table.insert(text, '查询统计：')
        table.insert(text, string.format('click_count：%d', clickCount))

        dialog.post(uid, text,
        {
            dialog.link('npc_goto_1', '刷新'),
            dialog.link(SYS_EXIT, '关闭'),
        })

        dbSetGKey('click_count', clickCount + 1)
        uidDBSetKey(uid, 'fld_float', 23.74589)
        uidDBSetKey(uid, 'fld_integer', getAbsTime())
        uidDBSetKey(uid, 'fld_text', randString(20, 'abcdefghijklmn'))
    end,
})
