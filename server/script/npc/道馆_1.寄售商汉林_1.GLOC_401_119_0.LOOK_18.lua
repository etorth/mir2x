setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>您好，我是<t color="YELLOW">%s</t>，想看看一般店铺里买不到的物品吗？</par>
                <par>如果您想寄售物品，我也可以帮忙。您需要先进行寄售登记，手续费为<t color="RED">5000金币</t>。物品卖出后，另收<t color="RED">2%%</t>的手续费。</par>
                <par>这不是蛮划算吗？不妨来试试吧。请选择要买卖的物品。</par>
                <par>每人最多可以寄售<t color="RED">20</t>件物品。</par>
                <par>注意事项：任务用道具过一定时间后会自动消失,所以尽量不要购买托管在我这里的道具。</par>
                <par></par>

                <par><event id="npc_goto_1">查看所有寄售的物品</event></par>
                <par><event id="npc_goto_1">查看衣服</event></par>
                <par><event id="npc_goto_1">查看武器</event></par>
                <par><event id="npc_goto_1">查看项链</event></par>
                <par><event id="npc_goto_1">查看头盔（帽子）</event></par>
                <par><event id="npc_goto_1">查看戒指</event></par>
                <par><event id="npc_goto_1">查看手镯（手套）</event></par>
                <par><event id="npc_goto_1">查看鞋类</event></par>
                <par><event id="npc_goto_1">查看药品</event></par>
                <par><event id="npc_goto_1">查看图书</event></par>
                <par><event id="npc_goto_1">查看其他物品</event></par>
                <par></par>

                <par>你以前寄售过物品吗？</par>
                <par><event id="npc_goto_1">查看您寄售物品的销售情况</event></par>

                <par></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], getNPCName(), SYS_EXIT)
    end,

    ["npc_goto_1"] = function(uid, value)
        uidGrant(uid, '斩马刀', 2)
        uidGrant(uid, '五彩鞋', 1)
        uidGrant(uid, '井中月', 1)
    end,
})
