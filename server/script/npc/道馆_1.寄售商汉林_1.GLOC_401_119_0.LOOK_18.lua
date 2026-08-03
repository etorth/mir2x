setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>您好，我是<t color="YELLOW">%s</t>，想看看一般店铺里买不到的物品吗？</par>
                <par>如果您想寄售物品，我也可以帮忙。您需要先进行寄售登记，手续费为<t color="RED">500金币</t>。物品卖出后，另收<t color="RED">2%%</t>的手续费。</par>
                <par>这不是蛮划算吗？不妨来试试吧。请选择要买卖的物品。</par>
                <par>每人最多可以寄售<t color="RED">20</t>件物品。</par>
                <par>注意事项：任务用道具过一定时间后会自动消失,所以尽量不要购买托管在我这里的道具。</par>
                <par></par>

                <par><event id="npc_goto_all">查看所有寄售的物品</event></par>
                <par><event id="npc_goto_dress">查看衣服</event></par>
                <par><event id="npc_goto_weapon">查看武器</event></par>
                <par><event id="npc_goto_necklace">查看项链</event></par>
                <par><event id="npc_goto_helmet">查看头盔（帽子）</event></par>
                <par><event id="npc_goto_ring">查看戒指</event></par>
                <par><event id="npc_goto_armring">查看手镯（手套）</event></par>
                <par><event id="npc_goto_shoes">查看鞋类</event></par>
                <par><event id="npc_goto_potion">查看药品</event></par>
                <par><event id="npc_goto_book">查看图书</event></par>
                <par><event id="npc_goto_other">查看其他物品</event></par>
                <par></par>

                <par>你以前寄售过物品吗？</par>
                <par><event id="npc_goto_mine">查看您寄售物品的销售情况</event></par>

                <par></par>
                <par><event id="%s" close="1">关闭</event></par>
            </layout>
        ]], getNPCName(), SYS_EXIT)
    end,

    ["npc_goto_all"     ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_ALL     ) end,
    ["npc_goto_dress"   ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_DRESS   ) end,
    ["npc_goto_weapon"  ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_WEAPON  ) end,
    ["npc_goto_necklace"] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_NECKLACE) end,
    ["npc_goto_helmet"  ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_HELMET  ) end,
    ["npc_goto_ring"    ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_RING    ) end,
    ["npc_goto_armring" ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_ARMRING ) end,
    ["npc_goto_shoes"   ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_SHOES   ) end,
    ["npc_goto_potion"  ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_POTION  ) end,
    ["npc_goto_book"    ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_BOOK    ) end,
    ["npc_goto_other"   ] = function(uid, value) uidPostAcutionItemList(uid, ACUTIONCAT_OTHER   ) end,
    ["npc_goto_mine"    ] = function(uid, value) end,
})
