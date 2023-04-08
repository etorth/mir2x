function main()
    uidExecute(getNPCharUID('道馆_1', '万事通_1'),
    [[
        return setQuestHandler('%s', {
            ['%s'] = function(uid)
                return uidExecute(uid, [=[ return getLevel() ]=]) >= 7
            end,

            ['%s'] = function(uid, value)
                uidPostXML(uid, {'%s', '%s'}, [=[
                    <layout>
                        <par>我万拍子近日愁眉苦脸，都是被那城门外的钉耙猫害的啊！少侠你已经%%d级了，愿意帮助老夫吗？</par>
                        <par><event id="%s">退出</event></par>
                    </layout>
                ]=], uidExecute(uid, [=[ return getLevel() ]=]))
            end
        })
    ]], getQuestName(), SYS_CHECKACTIVE, SYS_NPCINIT, SYS_EPQST, getQuestName(), SYS_NPCDONE)
end
