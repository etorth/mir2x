#include "sditem.hpp"
#include "serdesmsg.hpp"

std::string SDItem::str() const
{
    return str_printf("(name, itemID, seqID, count, duration) = (%s, %zu, %zu, %zu, (%zu, %zu))", to_cstr(DBCOM_ITEMRECORD(itemID).name), to_uz(itemID), to_uz(seqID), count, duration[0], duration[1]);
}

std::u8string SDItem::getXMLLayout(const std::unordered_map<int, std::string> & params, SDItem::SDItemXMLLayoutType layoutType) const
{
    fflassert(*this);
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(layoutType >= XMLLAYOUT_BEGIN || layoutType < XMLLAYOUT_END, layoutType);
    std::u8string xmlStr;

    xmlStr += str_printf(u8R"###( <layout> )###""\n");
    if(layoutType == XMLLAYOUT_FULL){
        xmlStr += str_printf(u8R"###( <par>【名称】%s</par> )###""\n", ir.name);
        xmlStr += str_printf(u8R"###( <par>【类型】%s</par> )###""\n", ir.type);
        xmlStr += str_printf(u8R"###( <par>【重量】%d</par> )###""\n", ir.weight);

        if(const auto p = params.find(SDItem::XML_PRICE); p != params.end()){
            const auto priceColor = params.contains(SDItem::XML_PRICECOLOR) ? to_cstr(params.at(SDItem::XML_PRICECOLOR)) : "green";
            xmlStr += str_printf(u8R"###( <par>【售价】<t color='%s'>%s</t></par> )###""\n", priceColor, to_cstr(p->second));
        }

        if(duration[1] > 0){
            fflassert(duration[0] <= duration[1]);
            const auto duraColorStr = [this]() -> const char *
            {
                if(duration[0] <= to_uz(duration[1] / 4) || (duration[0] <= 1)){
                    return "red";
                }
                else if(duration[0] <= to_uz(duration[1] / 2)){
                    return "yellow";
                }
                else{
                    return "white";
                }
            }();
            xmlStr += str_printf(u8R"###( <par>【持久】<t color='%s'>%zu/%zu/%d</t></par> )###""\n", duraColorStr, duration[0], duration[1], ir.equip.duration);
        }

        xmlStr += str_printf(u8R"###( <par></par> )###""\n");
        xmlStr += str_printf(u8R"###( <par>%s</par> )###""\n", str_haschar(ir.description) ? ir.description : u8"游戏处于开发阶段，暂无物品描述。");
        xmlStr += str_printf(u8R"###( <par></par> )###""\n");
    }

    const auto fnAddAttrValuePair = [&xmlStr, this](int val0, int val1, const char8_t *valName, std::optional<int> extAttr)
    {
        if(val0 > 0 || val1 > 0 || (extAttr.has_value() && extAttr.value() != 0)){
            const auto extAttrStr = str_printf("<t color='green'>（%+d）</t>", extAttr.value_or(0));
            xmlStr += str_printf(u8R"###( <par>%s %d - %d%s</par> )###""\n", to_cstr(valName), val0, val1 + extAttr.value_or(0), extAttr.has_value() ? extAttrStr.c_str() : "");
        }
    };

    const auto fnAddAttrValue = [&xmlStr, this](int val, const char8_t *valName, std::optional<int> extAttr)
    {
        if(val || (extAttr.has_value() && extAttr.value() != 0)){
            const auto extAttrStr = str_printf("<t color='green'>（%+d）</t>", extAttr.value_or(0));
            xmlStr += str_printf(u8R"###( <par>%s %+d%s</par> )###""\n", to_cstr(valName), val + extAttr.value_or(0), extAttr.has_value() ? extAttrStr.c_str() : "");
        }
    };

    fnAddAttrValuePair(ir.equip. dc[0], ir.equip. dc[1], u8"攻击", getExtAttr<SDItem::EA_DC_t >());
    fnAddAttrValuePair(ir.equip. mc[0], ir.equip. mc[1], u8"魔法", getExtAttr<SDItem::EA_MC_t >());
    fnAddAttrValuePair(ir.equip. sc[0], ir.equip. sc[1], u8"道术", getExtAttr<SDItem::EA_SC_t >());
    fnAddAttrValuePair(ir.equip. ac[0], ir.equip. ac[1], u8"防御", getExtAttr<SDItem::EA_AC_t >());
    fnAddAttrValuePair(ir.equip.mac[0], ir.equip.mac[1], u8"魔防", getExtAttr<SDItem::EA_MAC_t>());

    fnAddAttrValue(ir.equip.dcHit,   u8"命中",     getExtAttr<SDItem::EA_DCHIT_t  >());
    fnAddAttrValue(ir.equip.mcHit,   u8"魔法命中", getExtAttr<SDItem::EA_MCHIT_t  >());
    fnAddAttrValue(ir.equip.dcDodge, u8"闪避",     getExtAttr<SDItem::EA_DCDODGE_t>());
    fnAddAttrValue(ir.equip.mcDodge, u8"魔法闪避", getExtAttr<SDItem::EA_MCDODGE_t>());
    fnAddAttrValue(ir.equip.speed,   u8"速度",     getExtAttr<SDItem::EA_SPEED_t  >());
    fnAddAttrValue(ir.equip.comfort, u8"舒适度",   getExtAttr<SDItem::EA_COMFORT_t>());

    if(const auto extLuckCurseAttr = getExtAttr<SDItem::EA_LUCKCURSE_t>(); ir.equip.luckCurse || (extLuckCurseAttr.value_or(0) != 0)){
        if(const auto luckCurseSum = ir.equip.luckCurse + extLuckCurseAttr.value_or(0); luckCurseSum >= 0){
            const auto extAttrStr = str_printf("<t color='%s'>（%+d）</t>", (extLuckCurseAttr.value_or(0) >= 0) ? "green" : "red", extLuckCurseAttr.value_or(0));
            xmlStr += str_printf(u8R"###( <par>幸运 %+d%s</par> )###""\n", luckCurseSum, extLuckCurseAttr.value_or(0) ? extAttrStr.c_str() : "");
        }
        else{
            const auto extAttrStr = str_printf("<t color='%s'>（%+d）</t>", (extLuckCurseAttr.value_or(0) >= 0) ? "red" : "green", -1 * extLuckCurseAttr.value_or(0));
            xmlStr += str_printf(u8R"###( <par>诅咒 %+d%s</par> )###""\n", std::abs(luckCurseSum), extLuckCurseAttr.value_or(0) ? extAttrStr.c_str() : "");
        }
    }

    fnAddAttrValue(ir.equip.hp.add,     u8"生命上限", getExtAttr<SDItem::EA_HPADD_t>());
    fnAddAttrValue(ir.equip.hp.steal,   u8"生命盗取", getExtAttr<SDItem::EA_HPSTEAL_t>());
    fnAddAttrValue(ir.equip.hp.recover, u8"生命恢复", getExtAttr<SDItem::EA_HPRECOVER_t>());

    fnAddAttrValue(ir.equip.mp.add,     u8"魔法上限", getExtAttr<SDItem::EA_MPADD_t>());
    fnAddAttrValue(ir.equip.mp.steal,   u8"魔法盗取", getExtAttr<SDItem::EA_MPSTEAL_t>());
    fnAddAttrValue(ir.equip.mp.recover, u8"魔法恢复", getExtAttr<SDItem::EA_MPRECOVER_t>());

    if(ir.equip.dcElem.fire    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：火 %+d</par> )###""\n",   ir.equip.dcElem.fire   ); }
    if(ir.equip.dcElem.ice     > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：冰 %+d</par> )###""\n",   ir.equip.dcElem.ice    ); }
    if(ir.equip.dcElem.light   > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：雷 %+d</par> )###""\n",   ir.equip.dcElem.light  ); }
    if(ir.equip.dcElem.wind    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：风 %+d</par> )###""\n",   ir.equip.dcElem.wind   ); }
    if(ir.equip.dcElem.holy    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：神圣 %+d</par> )###""\n", ir.equip.dcElem.holy   ); }
    if(ir.equip.dcElem.dark    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：暗黑 %+d</par> )###""\n", ir.equip.dcElem.dark   ); }
    if(ir.equip.dcElem.phantom > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：幻影 %+d</par> )###""\n", ir.equip.dcElem.phantom); }

    if(ir.equip.acElem.fire    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：火 %+d</par> )###""\n",   ir.equip.acElem.fire   ); }
    if(ir.equip.acElem.ice     > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：冰 %+d</par> )###""\n",   ir.equip.acElem.ice    ); }
    if(ir.equip.acElem.light   > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：雷 %+d</par> )###""\n",   ir.equip.acElem.light  ); }
    if(ir.equip.acElem.wind    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：风 %+d</par> )###""\n",   ir.equip.acElem.wind   ); }
    if(ir.equip.acElem.holy    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：神圣 %+d</par> )###""\n", ir.equip.acElem.holy   ); }
    if(ir.equip.acElem.dark    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：暗黑 %+d</par> )###""\n", ir.equip.acElem.dark   ); }
    if(ir.equip.acElem.phantom > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：幻影 %+d</par> )###""\n", ir.equip.acElem.phantom); }

    if(ir.equip.acElem.fire    < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：火 %+d</par> )###""\n",   std::abs(ir.equip.acElem.fire   )); }
    if(ir.equip.acElem.ice     < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：冰 %+d</par> )###""\n",   std::abs(ir.equip.acElem.ice    )); }
    if(ir.equip.acElem.light   < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：雷 %+d</par> )###""\n",   std::abs(ir.equip.acElem.light  )); }
    if(ir.equip.acElem.wind    < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：风 %+d</par> )###""\n",   std::abs(ir.equip.acElem.wind   )); }
    if(ir.equip.acElem.holy    < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：神圣 %+d</par> )###""\n", std::abs(ir.equip.acElem.holy   )); }
    if(ir.equip.acElem.dark    < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：暗黑 %+d</par> )###""\n", std::abs(ir.equip.acElem.dark   )); }
    if(ir.equip.acElem.phantom < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：幻影 %+d</par> )###""\n", std::abs(ir.equip.acElem.phantom)); }

    if(const auto buffIDOpt = getExtAttr<SDItem::EA_BUFFID_t>(); buffIDOpt.has_value() && buffIDOpt.value()){
        xmlStr += str_printf(u8R"###( <par color='green'>附加BUFF：%s</par> )###""\n", to_cstr(DBCOM_BUFFRECORD(buffIDOpt.value()).name));
    }

    if(false
            || ir.equip.load.body > 0
            || ir.equip.load.weapon > 0
            || ir.equip.load.inventory > 0){

        xmlStr += str_printf(u8R"###( <par></par> )###""\n");
        fnAddAttrValue(ir.equip.load.body,      u8"身体负重", getExtAttr<SDItem::EA_LOADBODY_t>());
        fnAddAttrValue(ir.equip.load.weapon,    u8"武器负重", getExtAttr<SDItem::EA_LOADWEAPON_t>());
        fnAddAttrValue(ir.equip.load.inventory, u8"包裹负重", getExtAttr<SDItem::EA_LOADINVENTORY_t>());
    }

    if(false
            || ir.equip.req.dc > 0
            || ir.equip.req.mc > 0
            || ir.equip.req.sc > 0
            || ir.equip.req.level > 0
            || str_haschar(ir.equip.req.job)){

        xmlStr += str_printf(u8R"###( <par></par> )###""\n");
        if(ir.equip.req.dc > 0){
            xmlStr += str_printf(u8R"###( <par>需要攻击 %d</par> )###""\n", ir.equip.req.dc);
        }

        if(ir.equip.req.mc > 0){
            xmlStr += str_printf(u8R"###( <par>需要魔法 %d</par> )###""\n", ir.equip.req.mc);
        }

        if(ir.equip.req.sc > 0){
            xmlStr += str_printf(u8R"###( <par>需要道术 %d</par> )###""\n", ir.equip.req.sc);
        }

        if(ir.equip.req.level > 0){
            xmlStr += str_printf(u8R"###( <par>需要等级 %d</par> )###""\n", ir.equip.req.level);
        }

        if(str_haschar(ir.equip.req.job)){
            xmlStr += str_printf(u8R"###( <par>需要职业 %s</par> )###""\n", ir.equip.req.job);
        }
    }

    xmlStr += str_printf(u8R"###( </layout> )###""\n");
    return xmlStr;
}

std::vector<SDItem> SDItem::buildItemList(uint32_t itemID, size_t count)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    fflassert(ir);
    fflassert(count > 0);

    std::vector<SDItem> itemList;
    while(count > 0){
        const auto itemCount = ir.isGold() ? count : std::min<size_t>(ir.packable() ? SYS_INVGRIDMAXHOLD : 1, count);
        itemList.push_back(SDItem
        {
            .itemID = itemID,
            .count  = itemCount,
            .duration
            {
                to_uz(std::max<int>(0, ir.equip.duration)),
                to_uz(std::max<int>(0, ir.equip.duration)),
            },
        });
        count -= itemCount;
    }
    return itemList;
}

std::vector<SDItem> SDItem::buildGoldItem(size_t count)
{
    fflassert(count > 0);
    std::vector<SDItem> itemList;

    while(count > 0){
        if(count < 10){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（小）"),
                .count  = count,
            });
            break;
        }
        else if(count < 100){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（中）"),
                .count  = count,
            });
            break;
        }
        else if(count < 500){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（大）"),
                .count  = count,
            });
            break;
        }
        else if(count < 2000){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（特）"),
                .count  = count,
            });
            break;
        }
        else if(count < 5000){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（超）"),
                .count  = count,
            });
            break;
        }
        else{
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（超）"),
                .count  = 5000,
            });
            count -= 5000;
        }
    }
    return itemList;
}

SDItem::operator bool () const
{
    if(!itemID){
        return false;
    }

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        return false;
    }

    const auto maxCount = [&ir]() -> size_t
    {
        if(ir.isGold()){
            return SIZE_MAX;
        }

        if(ir.packable()){
            return SYS_INVGRIDMAXHOLD;
        }
        return 1;
    }();
    return count > 0 && count <= maxCount;
}

luaf::luaVar SDItem::asLuaVar() const
{
    std::unordered_map<std::string, luaf::luaVar> table
    {
        {"itemID", itemID},
        { "seqID",  seqID},
        { "count", to_i64(count)}, // size_t -> lua_Integer narrows, cast explicitly to avoid list-init error
    };

    std::unordered_map<std::string, luaf::luaVar> extAttrs;
    template for(constexpr int I: std::views::iota(EA_BEGIN, EA_END)){
        if(const auto attrOpt = getExtAttr<EA_t<I>>(); attrOpt.has_value()){
            extAttrs.emplace(EA_t<I>::name, luaf::buildLuaVar(attrOpt.value()));
        }
    }

    if(!extAttrs.empty()){
        table.emplace("extAttrs", luaf::buildLuaVar(std::move(extAttrs)));
    }

    return luaf::buildLuaVar(table);
}

SDItem SDItem::fromLuaVar(const luaf::luaVar &var)
{
    return std::visit(stdf::VarDispatcher
    {
        [](const luaf::luaTable &table) -> SDItem
        {
            SDItem item{};
            for(const auto &[keyWrapper, valueWrapper]: table){
                const auto &key   = std::get<std::string>(keyWrapper.get());
                const auto &value = valueWrapper.get();

                if(key == "itemID"){
                    item.itemID = luaf::luaVarAs<uint32_t>(value);
                }
                else if(key == "seqID"){
                    item.seqID = luaf::luaVarAs<uint32_t>(value);
                }
                else if(key == "count"){
                    item.count = luaf::luaVarAs<size_t>(value);
                }
                else if(key == "duration"){
                    const auto &dura = std::get<luaf::luaArray>(value);
                    fflassert(dura.size() == 2, dura.size());

                    item.duration[0] = luaf::luaVarAs<size_t>(dura[0].get());
                    item.duration[1] = luaf::luaVarAs<size_t>(dura[1].get());
                }
                else if(key == "extAttrs"){
                    const auto &attrs = std::get<luaf::luaTable>(value);
                    for(const auto &[attrKeyWrapper, attrValueWrapper]: attrs){
                        const auto &attrName  = std::get<std::string>(attrKeyWrapper.get());
                        const auto &attrValue = attrValueWrapper.get();

                        bool matched = false;
                        template for(constexpr int I: std::views::iota(EA_BEGIN, EA_END)){
                            if(attrName == EA_t<I>::name){
                                using ValType = typename EA_t<I>::type;
                                item.extAttrList[I] = cerealf::serialize<ValType>(luaf::luaVarAs<ValType>(attrValue), -1);
                                matched = true;
                            }
                        }

                        if(!matched){
                            throw fflerror("SDItem::fromLuaVar: unknown extended attribute: %s", attrName.c_str());
                        }
                    }
                }
                else{
                    throw fflerror("SDItem::fromLuaVar: unknown field: %s", key.c_str());
                }
            }

            if(!item){
                throw fflerror("SDItem::fromLuaVar: invalid item built from luaVar");
            }
            return item;
        },

        [](const auto &) -> SDItem
        {
            throw fflerror("SDItem::fromLuaVar: expect a lua table");
        }
    }, var);
}
