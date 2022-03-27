processNPCEvent =
{
    [SYS_NPCINIT] = function(uid, value)
        uidPostXML(uid, string.format(
        [[
            <layout>
                <par>客官%s你好我是%s，我可以给你展示系统所有的账号！<emoji id="0"/></par>
                <par></par>
                <par><event id="npc_goto_1">展示</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], uidQueryName(uid), getNPCName(), SYS_NPCDONE))
    end,

    ["npc_goto_1"] = function(uid, value)
        local result = dbQuery('select * from tbl_account')
        local parStr = ''
        for _, row in ipairs(result) do
            parStr = parStr .. string.format('<par>fld_id: %d, fld_account: %s</par>', row.fld_dbid, row.fld_account)
        end

        local clickCount = argDef(dbGetGKey('click_count'), 0)
        uidPostXML(uid,
        [[
            <layout>
                <par>数据库玩家账号有：</par>
                <par></par>
                %s
                <par></par>
                <par>上次查询：</par>
                <par>fld_float：%f</par>
                <par>fld_integer：%d</par>
                <par>fld_text：%s</par>
                <par></par>
                <par>查询统计：</par>
                <par>click_count：%d</par>
                <par></par>
                <par><event id="npc_goto_1">刷新</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]],

        parStr,
        argDef(uidDBGetKey(uid, 'fld_float'), 0.0),
        argDef(uidDBGetKey(uid, 'fld_integer'), 0),
        argDef(uidDBGetKey(uid, 'fld_text'), '(nil)'),
        clickCount,
        SYS_NPCDONE)

        dbSetGKey('click_count', clickCount + 1)
        uidDBSetKey(uid, 'fld_float', 23.74589)
        uidDBSetKey(uid, 'fld_integer', getAbsTime())
        uidDBSetKey(uid, 'fld_text', randString(20, 'abcdefghijklmn'))
    end,
}
