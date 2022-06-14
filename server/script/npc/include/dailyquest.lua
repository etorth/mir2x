local dq = {}

local RSVD_NAME_questList = rotable(
{
    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>这次事情去找一下比奇城的<t color="RED">王大人</t>吧。</par>
                <par>王大人也许在比奇城<t color="RED">390，397</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>想这次事情去找一下比奇城的<t color="RED">苏百花</t>吧。</par>
                <par>苏百花许在比奇城<t color="RED">483，405</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的材料商人<t color="RED">太俊</t>吧。</par>
                <par>太俊也许在比奇县<t color="RED">417，405</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的肉店<t color="RED">金氏</t>吧。</par>
                <par>金氏也许在比奇县<t color="RED">446，405</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的肉店的<t color="RED">肉店老板</t>吧。</par>
                <par>肉店老板也许在比奇县<t color="RED">441，404</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的武器商<t color="RED">老张</t>吧。</par>
                <par>老张也许在比奇县<t color="RED">423，395</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的铁匠铺<t color="RED">阿康</t>吧。</par>
                <par>阿康也许在比奇县<t color="RED">402，356</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的棉布店<t color="RED">怡美</t>吧。</par>
                <par>怡美也许在比奇县<t color="RED">480，407</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>鞋子商<t color="RED">慧媛</t>吧。</par>
                <par>慧媛也许在比奇县<t color="RED">478，407</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的中药商<t color="RED">药店老板</t>吧。</par>
                <par>药店老板也许在比奇县<t color="RED">397，363</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的中药商<t color="RED">恩英</t>吧。</par>
                <par>恩英老板也许在比奇县<t color="RED">486，414</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>里药铺的<t color="RED">药剂师</t>吧。</par>
                <par>药剂师也许在比奇县<t color="RED">412，410</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的书店<t color="RED">店员</t>吧。</par>
                <par>店员也许在比奇县<t color="RED">470，424</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的仓库<t color="RED">客栈店员</t>吧。</par>
                <par>客栈店员也许在比奇县<t color="RED">425，361</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的杂货店<t color="RED">杂货商</t>吧。</par>
                <par>杂货商也许在比奇县<t color="RED">450，413</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的饰品店<t color="RED">恩实</t>吧。</par>
                <par>恩实也许在比奇县<t color="RED">414，349</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的彩卷商<t color="RED">世玉</t>吧。</par>
                <par>世玉也许在比奇县<t color="RED">440，341</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的？？<t color="RED">酒娘</t>吧。</par>
                <par>酒娘也许在比奇县<t color="RED">420，431</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的<t color="RED">图书管理员</t>吧。</par>
                <par>图书管理员也许在比奇县<t color="RED">473，428</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的<t color="RED">比奇道长</t>吧。</par>
                <par>比奇道长也许在比奇县<t color="RED">449，433</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的夜市商人<t color="RED">老生</t>吧。</par>
                <par>老生也许在比奇县<t color="RED">452，297</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">比奇城</t>的<t color="RED">美容师</t>吧。</par>
                <par>美容师也许在比奇县<t color="RED">462，420</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的材料商人<t color="RED">阿辉</t>吧。</par>
                <par>阿勋也许在边境城市<t color="RED">431，307</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的肉店<t color="RED">金氏</t>吧。</par>
                <par>金氏也许在边境城市<t color="RED">425，274</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的铁匠铺<t color="RED">德秀</t>吧。</par>
                <par>德秀也许在边境城市<t color="RED">459，279</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的棉布店<t color="RED">顺子</t>吧。</par>
                <par>顺子也许在边境城市<t color="RED">442，296</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的中药商<t color="RED">尹老人</t>吧。</par>
                <par>尹老人也许在边境城市<t color="RED">273，189</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的书店<t color="RED">老余</t>吧。</par>
                <par>老余也许在边境城市<t color="RED">445，319</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>

        </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的仓库<t color="RED">赵老头</t>吧。</par>
                <par>赵老头也许在边境城市<t color="RED">471，279</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的杂货商<t color="RED">地摊商人</t>吧。</par>
                <par>地摊商人也许在边境城市<t color="RED">433，282</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">边境城市</t>的饰品商<t color="RED">晓丽</t>吧。</par>
                <par>晓丽也许在边境城市<t color="RED">444，270</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的材料商人<t color="RED">阿勋</t>吧。</par>
                <par>阿勋也许在银杏山谷<t color="RED">269，186</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的肉店<t color="RED">许氏</t>吧。</par>
                <par>许氏也许在银杏山谷<t color="RED">228，194</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的铁匠铺<t color="RED">铁匠师傅</t>吧。</par>
                <par>铁匠师傅也许在银杏山谷<t color="RED">284，197</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的棉布店<t color="RED">布店晓芙</t>吧。</par>
                <par>布店晓芙也许在银杏山谷<t color="RED">260，177</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的中药商<t color="RED">许中医</t>吧。</par>
                <par>许中医也许在银杏山谷<t color="RED">273，189</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>里药铺的<t color="RED">药剂师</t>吧。</par>
                <par>药剂师也许在银杏山谷<t color="RED">272，189</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的书店<t color="RED">才石</t>吧。</par>
                <par>才石也许在银杏山谷<t color="RED">233，222</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的仓库<t color="RED">阿雯</t>吧。</par>
                <par>阿雯也许在银杏山谷<t color="RED">250，190</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的杂货商<t color="RED">地摊老周</t>吧。</par>
                <par>地摊老周也许在银杏山谷<t color="RED">236，205</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>

        </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的饰品商<t color="RED">晓兰</t>吧。</par>
                <par>晓兰也许在银杏山谷<t color="RED">275，222</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的<t color="RED">南宫小姐</t>吧。</par>
                <par>南宫小姐也许在银杏山谷<t color="RED">264，201</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的<t color="RED">化天先生</t>吧。</par>
                <par>化天先生也许在银杏山谷<t color="RED">353，212</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">银杏山谷</t>的<t color="RED">霹雳尊者</t>吧。</par>
                <par>霹雳尊者也许在银杏山谷<t color="RED">265，145</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的材料商人<t color="RED">天星</t>吧。</par>
                <par>天星也许在道馆<t color="RED">386，119</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的武器商<t color="RED">阿潘</t>吧。</par>
                <par>阿潘也许在道馆<t color="RED">429，120</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的武器商<t color="RED">铁匠</t>吧。</par>
                <par>铁匠也许在道馆<t color="RED">426，117</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>

        </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的棉布店<t color="RED">阿浩</t>吧。</par>
                <par>阿浩也许在道馆<t color="RED">415，133</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的棉布店<t color="RED">梁生</t>吧。</par>
                <par>梁生也许在道馆<t color="RED">412，133</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的中药商<t color="RED">老药</t>吧。</par>
                <par>老药也许在道馆<t color="RED">388，113</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的中药商<t color="RED">药中</t>吧。</par>
                <par>药中也许在道馆<t color="RED">393，113</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>里药铺的<t color="RED">禄英</t>吧。</par>
                <par>禄英也许在道馆<t color="RED">388，113</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>里药铺的<t color="RED">药神</t>吧。</par>
                <par>药神也许在道馆<t color="RED">387，118</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的书店<t color="RED">书神</t>吧。</par>
                <par>书神也许在道馆<t color="RED">415，96</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的书店<t color="RED">书鬼</t>吧。</par>
                <par>书鬼也许在道馆<t color="RED">418，96</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的仓库<t color="RED">阿天</t>吧。</par>
                <par>阿天也许在道馆<t color="RED">396，116</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的杂货商<t color="RED">大老板</t>吧。</par>
                <par>大老板也许在道馆<t color="RED">394，169</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的杂货商<t color="RED">小老板</t>吧。</par>
                <par>小老板也许在道馆<t color="RED">391，167</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的饰品商<t color="RED">峰儿</t>吧。</par>
                <par>峰儿也许在道馆<t color="RED">367，134</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆</t>的饰品商<t color="RED">峰儿</t>吧。</par>
                <par>峰儿也许在道馆<t color="RED">370，135</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆前边城市</t>的肉店<t color="RED">钱老板</t>吧。</par>
                <par>钱老板也许在道馆<t color="RED">366，182</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆前边城市</t>的铁匠铺<t color="RED">金氏</t>吧。</par>
                <par>金氏也许在道馆<t color="RED">382，198</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">道馆前边城市</t>的中药商<t color="RED">华玉</t>吧。</par>
                <par>华玉也许在道馆<t color="RED">363，176</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,
    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的<t color="RED">蛇谷老矿夫</t>吧。</par>
                <par>阿福也许在毒蛇山谷<t color="RED">360，209</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的<t color="RED">蛇谷老太</t>吧。</par>
                <par>阿福也许在毒蛇山谷<t color="RED">337，223</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的材料商人<t color="RED">阿福</t>吧。</par>
                <par>阿福也许在毒蛇山谷<t color="RED">331，225</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的铁匠铺<t color="RED">阿康</t>吧。</par>
                <par>阿康也许在毒蛇山谷<t color="RED">351，219</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的棉布店<t color="RED">金莲</t>吧。</par>
                <par>金莲也许在毒蛇山谷<t color="RED">342，235</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的中药商<t color="RED">金中医</t>吧。</par>
                <par>金中医也许在毒蛇山谷<t color="RED">334，224</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的仓库<t color="RED">客栈保管员</t>吧。</par>
                <par>客栈保管员也许在毒蛇山谷<t color="RED">346，234</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的杂货商<t color="RED">流浪卢侠</t>吧。</par>
                <par>流浪卢侠也许在毒蛇山谷<t color="RED">361，226</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市</t>的<t color="RED">蛇谷老人</t>吧。</par>
                <par>蛇谷老人也许在毒蛇山谷<t color="RED">343，219</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷城市 </t>的<t color="RED">断乔先生</t>吧。</par>
                <par>蛇谷老人也许在毒蛇山谷<t color="RED">347，193</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">蛇谷矿山</t>的<t color="RED">矿山老人</t>吧。</par>
                <par>矿山老人也许在矿山1层<t color="RED">34，368</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的材料商人<t color="RED">俞公</t>吧。</par>
                <par>俞公也许在沙巴克城<t color="RED">206，255</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的肉店<t color="RED">老陆</t>吧。</par>
                <par>老陆也许在沙巴克城<t color="RED">221，192</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>

        </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的武器商<t color="RED">大熊</t>吧。</par>
                <par>大熊也许在沙巴克城<t color="RED">195，239</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的铁匠铺<t color="RED">老胡</t>吧。</par>
                <par>老胡也许在沙巴克城<t color="RED">183，236</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的头盔商<t color="RED">财顺</t>吧。</par>
                <par>财顺也许在沙巴克城<t color="RED">230，208</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的棉布店<t color="RED">喜儿</t>吧。</par>
                <par>喜儿也许在沙巴克城<t color="RED">231，208</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的中药商<t color="RED">晶晶</t>吧。</par>
                <par>晶晶也许在沙巴克城<t color="RED">203，253</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>

        </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的仓库<t color="RED">惠莲</t>吧。</par>
                <par>惠莲也许在沙巴克城<t color="RED">214，131</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的杂货商<t color="RED">金氏</t>吧。</par>
                <par>金氏也许在沙巴克城<t color="RED">229，224</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的戒指商<t color="RED">吉祥</t>吧。</par>
                <par>吉祥也许在沙巴克城<t color="RED">212，240</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的手镯商<t color="RED">多多</t>吧。</par>
                <par>多多也许在沙巴克城<t color="RED">209，233</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙巴克城</t>的项链商<t color="RED">美玉</t>吧。</par>
                <par>美玉也许在沙巴克城<t color="RED">209，233</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的材料商人<t color="RED">石宇</t>吧。</par>
                <par>石宇也许在绿洲<t color="RED">433，72</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的材料商人<t color="RED">润京</t>吧。</par>
                <par>润京也许在绿洲<t color="RED">437，71</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的肉店<t color="RED">屠夫</t>吧。</par>
                <par>屠夫也许在绿洲<t color="RED">438，52</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的铁匠铺<t color="RED">王铁匠</t>吧。</par>
                <par>王铁匠也许在绿洲<t color="RED">449，73</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的铁匠铺<t color="RED">丁铁匠</t>吧。</par>
                <par>丁铁匠也许在绿洲<t color="RED">448，76</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的棉布店<t color="RED">织女</t>吧。</par>
                <par>织女也许在绿洲<t color="RED">462，89</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的棉布店<t color="RED">布商女店员</t>吧。</par>
                <par>布商女店员也许在绿洲<t color="RED">462，91</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的中药商<t color="RED">白老中医</t>吧。</par>
                <par>白老中医也许在绿洲<t color="RED">433，72</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的中药商<t color="RED">医仙</t>吧。</par>
                <par>医仙也许在绿洲<t color="RED">430，75</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的仓库<t color="RED">？绿洲仓库管家</t>吧。</par>
                <par>绿洲仓库管家也许在绿洲<t color="RED">433，72</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的仓库<t color="RED">绿洲仓库保管员</t>吧。</par>
                <par>绿洲仓库保管员也许在绿洲<t color="RED">438，68</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的杂货商<t color="RED">洪老板</t>吧。</par>
                <par>洪老板也许在绿洲<t color="RED">470，67</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的法师<t color="RED">梅山侠</t>吧。</par>
                <par>梅山侠也许在绿洲<t color="RED">476，57</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>

        </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的饰品商<t color="RED">峨嵋侠</t>吧。</par>
                <par>峨嵋侠也许在绿洲<t color="RED">475，55</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">绿洲</t>的<t color="RED">唯我独尊</t>吧。</par>
                <par>唯我独尊也许在绿洲<t color="RED">426，97</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的材料商人<t color="RED">阿宋</t>吧。</par>
                <par>阿宋也许在沙漠土城<t color="RED">197，235</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的肉店<t color="RED">黄老板</t>吧。</par>
                <par>黄老板也许在沙漠土城<t color="RED">200，249</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的铁匠铺<t color="RED">勇汉</t>吧。</par>
                <par>勇汉也许在沙漠土城<t color="RED">190，249</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的铁匠铺<t color="RED">铁汉</t>吧。</par>
                <par>铁汉也许在沙漠土城<t color="RED">189，252</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的棉布店<t color="RED">晓敏</t>吧。</par>
                <par>晓敏也许在沙漠土城<t color="RED">179，278</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的棉布店<t color="RED">喜儿</t>吧。</par>
                <par>喜儿也许在沙漠土城<t color="RED">177，28</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>鞋子商<t color="RED">润真</t>吧。</par>
                <par>润真也许在沙漠土城<t color="RED">181，277</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的中药商<t color="RED">医神</t>吧。</par>
                <par>医神也许在沙漠土城<t color="RED">193，231</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的中药商<t color="RED">药郎</t>吧。</par>
                <par>药郎也许在沙漠土城<t color="RED">200，237</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>

        </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>药铺的<t color="RED">洪药剂师</t>吧。</par>
                <par>洪药剂师也许在沙漠土城<t color="RED">198，236</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的仓库<t color="RED">沙漠仓库管家</t>吧。</par>
                <par>沙漠仓库管家也许在沙漠土城<t color="RED">159，267</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的仓库<t color="RED">沙漠仓库小管家</t>吧。</par>
                <par>沙漠仓库小管家也许在沙漠土城<t color="RED">171，265</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的杂货商<t color="RED">杂货商老李</t>吧。</par>
                <par>杂货商老李也许在沙漠土城<t color="RED">192，232</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的饰品商<t color="RED">雪儿</t>吧。</par>
                <par>雪儿也许在沙漠土城<t color="RED">203，241</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的饰品商<t color="RED">倩儿</t>吧。</par>
                <par>倩儿也许在沙漠土城<t color="RED">208，246</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的<t color="RED">芙蓉仙子</t>吧。</par>
                <par>芙蓉仙子也许在沙漠土城<t color="RED">230，188</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">沙漠土城</t>的<t color="RED">小秀玉女</t>吧。</par>
                <par>小秀玉女也许在沙漠土城<t color="RED">199，203</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的材料商人<t color="RED">阿贵</t>吧。</par>
                <par>阿贵也许在盟重土城<t color="RED">287，290</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的铁匠铺<t color="RED">英龙</t>吧。</par>
                <par>英龙也许在盟重土城<t color="RED">354，297</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的棉布店<t color="RED">晓薇</t>吧。</par>
                <par>晓薇也许在盟重土城<t color="RED">324，277</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的中药商<t color="RED">凌峰</t>吧。</par>
                <par>凌峰也许在盟重土城<t color="RED">289，289</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的仓库<t color="RED">丘叔</t>吧。</par>
                <par>丘叔也许在盟重土城<t color="RED">323，310</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的杂货商<t color="RED">龙叔</t>吧。</par>
                <par>龙叔也许在盟重土城<t color="RED">293，278</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的饰品商<t color="RED">善颖</t>吧。</par>
                <par>善颖也许在盟重土城<t color="RED">311，273</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的<t color="RED">盟重道长</t>吧。</par>
                <par>盟重道长也许在盟重土城<t color="RED">297，285</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">盟重土城</t>的<t color="RED">雷翁断珠</t>吧。</par>
                <par>雷翁断珠也许在盟重土城<t color="RED">290，299</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">潘夜岛</t>的材料商人<t color="RED">泼皮</t>吧。</par>
                <par>泼皮也许在潘夜岛<t color="RED">271，222</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">潘夜岛</t>的铁匠铺<t color="RED">太俊</t>吧。</par>
                <par>太俊也许在潘夜岛<t color="RED">224，210</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">潘夜岛</t>的棉布店<t color="RED">晓洋</t>吧。</par>
                <par>晓洋也许在潘夜岛<t color="RED">249，257</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">潘夜岛</t>的中药商<t color="RED">成赫</t>吧。</par>
                <par>成赫也许在潘夜岛<t color="RED">282，224</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">潘夜岛</t>的仓库<t color="RED">满春</t>吧。</par>
                <par>满春也许在潘夜岛<t color="RED">231，260</t>附近的建筑物内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">潘夜岛</t>的杂货商<t color="RED">中叔</t>吧。</par>
                <par>中叔也许在潘夜岛<t color="RED">253，291</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">潘夜岛</t>的饰品商<t color="RED">晓华</t>吧。</par>
                <par>晓华也许在潘夜岛<t color="RED">266，285</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">祖玛神殿</t>的中药商<t color="RED">阿山</t>吧。</par>
                <par>阿山也许在<t color="RED">祖玛神殿7层大厅</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">石阁阵</t>的铁匠铺<t color="RED">铁虎</t>吧。</par>
                <par>铁虎也许在<t color="RED">石阁阵</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">石阁阵</t>的棉布店<t color="RED">阿紫</t>吧。</par>
                <par>阿紫也许在<t color="RED">石阁阵</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">石阁阵</t>的中药商<t color="RED">晴儿</t>吧。</par>
                <par>晴儿也许在<t color="RED">石阁阵</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">石阁阵</t>的饰品商<t color="RED">灵儿</t>吧。</par>
                <par>灵儿也许在<t color="RED">石阁阵</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">南部蚂蚁洞穴入口</t>的杂货商<t color="RED">阿全</t>吧。</par>
                <par>阿全也许在沙漠<t color="RED">284，723</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">半兽洞穴</t>的仓库<t color="RED">洞穴保管员</t>吧。</par>
                <par>洞穴保管员也许在半兽洞穴2层<t color="RED">174，216</t>附近。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end,

    function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>只说一遍，希望认真听好。。</par>
                <par>这次事情去找一下在<t color="RED">半兽洞穴</t>的杂货商<t color="RED">奸商欧阳</t>吧。</par>
                <par>奸商欧阳也许在半兽洞穴2层<t color="RED">191，231</t>附近的洞窟内。</par>
                <par></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_NPCDONE)
    end
})

function dq.setQuest(questID, uid, value)
    assertType(questID, 'integer')
    assertType(uid, 'integer')

    if questID < 0 then
        fatalPrintf('Invalid quest id: %d', questID)

    elseif questID == 0 then
        -- questID = math.random(1, #RSVD_NAME_questList)
        questID = 7

    else
        questID = 1 + (questID % #RSVD_NAME_questList)
    end

    RSVD_NAME_questList[questID](uid, value)
end

return dq
