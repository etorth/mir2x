#include <cmath>
#include <ranges>
#include <memory>
#include <numeric>
#include <cstring>
#include "actionnode.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"
#include "uidf.hpp"
#include "mathf.hpp"
#include "pathf.hpp"
#include "raiitimer.hpp"
#include "sysconst.hpp"
#include "log.hpp"
#include "mapbindb.hpp"
#include "pngtexdb.hpp"
#include "bgmusicdb.hpp"
#include "soundeffectdb.hpp"
#include "sdldevice.hpp"
#include "clientargparser.hpp"
#include "processrun.hpp"
#include "clientluamodule.hpp"
#include "clientpathfinder.hpp"
#include "notifyboard.hpp"
#include "gui/npcchatboard/npcchatboard.hpp"
#include "modalstringboard.hpp"
#include "fflerror.hpp"
#include "totype.hpp"
#include "gui/skillboard/skillboard.hpp"
#include "lochashtable.hpp"
#include "clienttaodog.hpp"
#include "clienttaoskeleton.hpp"
#include "clienttaoskeletonext.hpp"

extern Log *g_log;
extern Client *g_client;
extern PNGTexDB *g_mapDB;
extern MapBinDB *g_mapBinDB;
extern SDLDevice *g_sdlDevice;
extern PNGTexDB *g_progUseDB;
extern BGMusicDB *g_bgmDB;
extern SoundEffectDB *g_seffDB;
extern PNGTexDB *g_itemDB;
extern NotifyBoard *g_notifyBoard;
extern ClientArgParser *g_clientArgParser;

ProcessRun::ProcessRun(const SMOnlineOK &smOOK)
    : Process()
    , m_myHeroUID(smOOK.uid)
    , m_luaModule(this)
    , m_defaultChatPeer
      {
          .id = uidf::getPlayerDBID(smOOK.uid),
          .name = smOOK.name.as_rawcstr(),
          .despvar = SDChatPeerPlayerVar
          {
              .gender = to_bool(smOOK.gender),
              .job = smOOK.job,
          },
      }
    , m_guiManager(this)
    , m_mousePixlLoc(DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0XFF, 0X00, 0X00, 0X00))
    , m_mouseGridLoc(DIR_UPLEFT, 0, 0, u8"", 0, 15, 0, colorf::RGBA(0XFF, 0X00, 0X00, 0X00))
    , m_teamFlag(5)
{
    loadMap(smOOK.mapUID, smOOK.action.x, smOOK.action.y);
    m_coList.insert_or_assign(m_myHeroUID, std::unique_ptr<ClientCreature>(new MyHero
    {
        m_myHeroUID,
        to_bool(smOOK.gender),
        smOOK.job,
        this,
        ActionStand
        {
            .direction = DIR_DOWN,
            .x = smOOK.action.x,
            .y = smOOK.action.y,
        },
    }));
    RegisterUserCommand();
}

void ProcessRun::scrollMap()
{
    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
    const auto showWindowW = rendererW;
    const auto showWindowH = rendererH - getWidget("ControlBoard")->h();

    const int nViewX = getMyHero()->x() * SYS_MAPGRIDXP - showWindowW / 2;
    const int nViewY = getMyHero()->y() * SYS_MAPGRIDYP - showWindowH / 2;

    const int nDViewX = nViewX - m_viewX;
    const int nDViewY = nViewY - m_viewY;

    if(m_mapScrolling
            ||  (std::abs(nDViewX) > showWindowW / 6)
            ||  (std::abs(nDViewY) > showWindowH / 6)){

        m_mapScrolling = true;

        m_viewX += to_d(std::lround(std::copysign((std::min<int>)(3, std::abs(nDViewX)), nDViewX)));
        m_viewY += to_d(std::lround(std::copysign((std::min<int>)(2, std::abs(nDViewY)), nDViewY)));

        m_viewX = (std::max<int>)(0, m_viewX);
        m_viewY = (std::max<int>)(0, m_viewY);
    }

    // stop rolling the map when
    //   1. the hero is at the required position
    //   2. the hero is not moving
    if((nDViewX == 0) && (nDViewY == 0) && !getMyHero()->moving()){
        m_mapScrolling = false;
    }
}

void ProcessRun::update(double fUpdateTime)
{
    updateMouseFocus();
    m_teamFlag.update(fUpdateTime * 1000);
    m_aniTimer.update(std::lround(fUpdateTime));

    scrollMap();
    m_guiManager.update(fUpdateTime);
    m_delayCmdQ.exec();

    for(auto p = m_strikeGridList.begin(); p != m_strikeGridList.end();){
        if(hres_tstamp().to_msec() > p->second + 1000){
            p = m_strikeGridList.erase(p);
        }
        else{
            p++;
        }
    }

    getMyHero()->update(fUpdateTime);
    const int myHeroX = getMyHero()->x();
    const int myHeroY = getMyHero()->y();

    for(auto p = m_coList.begin(); p != m_coList.end();){
        if(p->second.get() == getMyHero()){
            ++p;
            continue;
        }

        const auto [locX, locY] = p->second->location();
        const auto locDistance2 = mathf::LDistance2(myHeroX, myHeroY, locX, locY);
        if(p->second->visible() && (locDistance2 < 1000)){
            if(p->second->lastActive() + 5000 < SDL_GetTicks() && p->second->lastQuerySelf() + 5000 < SDL_GetTicks()){
                p->second->querySelf();
            }
            p->second->update(fUpdateTime);
            ++p;
        }
        else{
            p = m_coList.erase(p);
        }
    }

    for(auto p = m_fixedLocMagicList.begin(); p != m_fixedLocMagicList.end();){
        if((*p)->update(fUpdateTime)){
            p = m_fixedLocMagicList.erase(p);
        }
        else{
            ++p;
        }
    }

    for(auto p = m_followUIDMagicList.begin(); p != m_followUIDMagicList.end();){
        if((*p)->update(fUpdateTime)){
            p = m_followUIDMagicList.erase(p);
        }
        else{
            ++p;
        }
    }

    for(auto p = m_ascendStrList.begin(); p != m_ascendStrList.end();){
        if((*p)->ratio() < 1.00){
            (*p)->update(fUpdateTime);
            ++p;
        }
        else{
            p = m_ascendStrList.erase(p);
        }
    }

    if(true){
        centerMyHero();
    }

    m_starRatio = std::fmod(m_starRatio + 0.05, 2.50);
    m_iconRatio = std::fmod(m_iconRatio + 0.05, 1.00);

    if(const auto currTick = SDL_GetTicks(); m_lastPingDone && (m_lastPingTick + 10ULL * 1000 < currTick)){
        m_lastPingDone = false;
        m_lastPingTick = currTick;

        CMPing cmP;
        std::memset(&cmP, 0, sizeof(cmP));

        cmP.Tick = currTick;
        g_client->send({CM_PING, cmP});
    }
}

uint64_t ProcessRun::getFocusUID(int focusType, bool allowMyHero) const
{
    switch(focusType){
        case FOCUS_MOUSE:
            {
                const auto [mouseWinPX, mouseWinPY] = SDLDeviceHelper::getMousePLoc();
                const auto mousePX = mouseWinPX + m_viewX;
                const auto mousePY = mouseWinPY + m_viewY;

                if(auto coPtr = findUID(m_focusUIDTable[FOCUS_MOUSE]); coPtr && coPtr->canFocus(mousePX, mousePY)){
                    return m_focusUIDTable[FOCUS_MOUSE];
                }

                ClientCreature *focusCOPtr = nullptr;
                for(const auto &[uid, coPtr]: m_coList){
                    if((uid == getMyHeroUID()) && !allowMyHero){
                        continue;
                    }

                    if(!coPtr->canFocus(mousePX, mousePY)){
                        continue;
                    }

                    if(!focusCOPtr || focusCOPtr->y() < coPtr->y()){
                        focusCOPtr = coPtr.get();
                    }
                }
                return focusCOPtr ? focusCOPtr->UID() : 0;
            }
        case FOCUS_MAGIC:
        case FOCUS_FOLLOW:
        case FOCUS_ATTACK:
            {
                return m_focusUIDTable[focusType];
            }
        default:
            {
                throw fflvalue(focusType, allowMyHero);
            }
    }
}

void ProcessRun::setFocusUID(int focusType, uint64_t uid)
{
    switch(focusType){
        case FOCUS_MAGIC:
        case FOCUS_FOLLOW:
        case FOCUS_ATTACK:
            {
                m_focusUIDTable[focusType] = uid;
                return;
            }
        default:
            {
                throw fflerror("invalid focus type: %d", focusType);
            }
    }
}

void ProcessRun::draw() const
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    const int x0 = mathf::bound<int>(-SYS_OBJMAXW + (m_viewX - 2 * SYS_MAPGRIDXP) / SYS_MAPGRIDXP,                                    0, m_mir2xMapData.w());
    const int y0 = mathf::bound<int>(-SYS_OBJMAXH + (m_viewY - 2 * SYS_MAPGRIDYP) / SYS_MAPGRIDYP,                                    0, m_mir2xMapData.h());
    const int x1 = mathf::bound<int>(+SYS_OBJMAXW + (m_viewX + 2 * SYS_MAPGRIDXP + g_sdlDevice->getRendererWidth() ) / SYS_MAPGRIDXP, 0, m_mir2xMapData.w());
    const int y1 = mathf::bound<int>(+SYS_OBJMAXH + (m_viewY + 2 * SYS_MAPGRIDYP + g_sdlDevice->getRendererHeight()) / SYS_MAPGRIDYP, 0, m_mir2xMapData.h());

    drawTile(x0, y0, x1, y1);

    // ground objects
    for(int y = y0; y <= y1; ++y){
        for(int x = x0; x <= x1; ++x){
            drawObject(x, y, OBJD_GROUND, false);
        }
    }

    if(g_clientArgParser->drawTranspGrid){
        for(const auto &entry: DBCOM_MAPRECORD(mapID()).mapSwitchList){
            for(int tpy = entry.y; tpy < entry.y + entry.h; ++tpy){
                for(int tpx = entry.x; tpx < entry.x + entry.w; ++tpx){
                    if(true
                            && m_mir2xMapData.validC(tpx, tpy)
                            && mathf::pointInRectangle(tpx, tpy, x0, y0, x1 - x0 + 1, y1 - y0 + 1)){
                        g_sdlDevice->fillRectangle(colorf::RGBA(0XFF, 0, 0, 100), tpx * SYS_MAPGRIDXP - m_viewX, tpy * SYS_MAPGRIDYP - m_viewY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                    }
                }
            }
        }
    }

    // over ground object
    for(int y = y0; y <= y1; ++y){
        for(int x = x0; x <= x1; ++x){
            drawObject(x, y, OBJD_OVERGROUND0, false);
            drawObject(x, y, OBJD_OVERGROUND1, false);
        }
    }

    // draw ground ash, ice mark etc.
    // should be draw immediately before dead actors, like over-ground objects
    for(const auto &p: m_fixedLocMagicList){
        if(p->checkMagic(u8"魔法特效_火焰灰烬", u8"运行")){
            dynamic_cast<FireAshEffect_RUN *>(p.get())->drawGroundAsh(m_viewX, m_viewY, colorf::WHITE + colorf::A_SHF(150));
        }

        if(p->checkMagic(u8"冰沙掌", u8"运行")){
            dynamic_cast<IceThrust_RUN *>(p.get())->drawGroundIce(m_viewX, m_viewY, colorf::WHITE + colorf::A_SHF(150));
        }
    }

    // draw dead actors
    // dead actors are shown before all active actors
    for(auto &p: m_coList){
        p.second->draw(m_viewX, m_viewY, 0);
    }

    drawGroundItem(x0, y0, x1, y1);

    const auto coLocList = [this]()
    {
        LocHashTable<std::vector<ClientCreature *>> table;
        for(auto &p: m_coList){
            table[p.second->location()].push_back(p.second.get());
        }
        return table;
    }();

    const auto fireWallList = [this]()
    {
        LocHashTable<std::vector<FixedLocMagic *>> table;
        for(auto &p: m_fixedLocMagicList){
            if(p->getGfxEntry()->onGround){
                table[{p->x(), p->y()}].push_back(p.get());
            }
        }
        return table;
    }();

    // over ground objects
    for(int y = y0; y <= y1; ++y){
        for(int x = x0; x <= x1; ++x){
            drawObject(x, y, OBJD_OVERGROUND0, false);
        }

        for(int x = x0; x <= x1; ++x){
            if(auto p = fireWallList.find({x, y}); p != fireWallList.end()){
                for(auto magicPtr: p->second){
                    magicPtr->drawViewOff(m_viewX, m_viewY, colorf::WHITE_A255);
                }
            }

            if(auto p = m_strikeGridList.find({x, y}); p != m_strikeGridList.end()){
                if(hres_tstamp().to_msec() <= p->second + 1000){
                    g_sdlDevice->fillRectangle(colorf::RED + colorf::A_SHF(96), x * SYS_MAPGRIDXP - m_viewX, y * SYS_MAPGRIDYP - m_viewY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                }
            }

            if(auto p = coLocList.find({x, y}); p != coLocList.end()){
                for(auto creaturePtr: p->second){
                    if(!(creaturePtr && creaturePtr->location() == std::make_tuple(x, y))){
                        throw fflerror("invalid creature location table");
                    }

                    if(!creaturePtr->alive()){
                        continue;
                    }

                    int focusMask = 0;
                    for(int f = FOCUS_BEGIN; f < FOCUS_END; ++f){
                        if(getFocusUID(f) == creaturePtr->UID()){
                            focusMask |= (1 << f);
                        }
                    }
                    creaturePtr->draw(m_viewX, m_viewY, focusMask);
                }

                if(g_clientArgParser->drawCreatureCover){
                    SDLDeviceHelper::EnableRenderColor enableColor(colorf::RGBA(0, 0, 255, 128));
                    SDLDeviceHelper::EnableRenderBlendMode enableBlendMode(SDL_BLENDMODE_BLEND);
                    g_sdlDevice->fillRectangle(x * SYS_MAPGRIDXP - m_viewX, y * SYS_MAPGRIDYP - m_viewY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                }
            }
        }

        for(int x = x0; x <= x1; ++x){
            drawObject(x, y, OBJD_OVERGROUND1, false);
        }
    }

    if(g_clientArgParser->fillMapBlockGrid){
        for(int y = y0; y <= y1; ++y){
            for(int x = x0; x <= x1; ++x){
                if(!(m_mir2xMapData.validC(x, y) && m_mir2xMapData.cell(x, y).land.canThrough())){
                    g_sdlDevice->fillRectangle(colorf::YELLOW + colorf::A_SHF(127), x * SYS_MAPGRIDXP - m_viewX, y * SYS_MAPGRIDYP - m_viewY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                }
            }
        }
    }

    if(g_clientArgParser->drawMapGrid){
        const int gridX0 = m_viewX / SYS_MAPGRIDXP;
        const int gridY0 = m_viewY / SYS_MAPGRIDYP;

        const int gridX1 = (m_viewX + g_sdlDevice->getRendererWidth ()) / SYS_MAPGRIDXP;
        const int gridY1 = (m_viewY + g_sdlDevice->getRendererHeight()) / SYS_MAPGRIDYP;

        SDLDeviceHelper::EnableRenderColor drawColor(colorf::RGBA(0, 255, 0, 128));
        for(int x = gridX0; x <= gridX1; ++x){
            g_sdlDevice->drawLine(x * SYS_MAPGRIDXP - m_viewX, 0, x * SYS_MAPGRIDXP - m_viewX, g_sdlDevice->getRendererHeight());
        }

        for(int y = gridY0; y <= gridY1; ++y){
            g_sdlDevice->drawLine(0, y * SYS_MAPGRIDYP - m_viewY, g_sdlDevice->getRendererWidth(), y * SYS_MAPGRIDYP - m_viewY);
        }
    }

    // draw all rotating stars
    // notify players that there is somethig to check
    drawRotateStar(x0, y0, x1, y1);

    // draw magics
    for(auto &p: m_fixedLocMagicList){
        if(!p->getGfxEntry()->onGround){
            p->drawViewOff(m_viewX, m_viewY, colorf::WHITE_A255);
        }
    }

    for(auto &p: m_followUIDMagicList){
        if(!p->done()){
            p->drawViewOff(m_viewX, m_viewY, colorf::WHITE_A255);
        }
    }

    if(m_drawMagicKey){
        int magicKeyOffX = 0;
        for(const auto &[magicID, magicKey]: dynamic_cast<const SkillBoard *>(this->getWidget("SkillBoard"))->getConfig().getMagicKeyList()){
            if(const auto &iconGfx = SkillBoard::getMagicIconGfx(magicID); iconGfx && iconGfx.magicIcon != SYS_U32NIL){
                if(auto texPtr = g_progUseDB->retrieve(iconGfx.magicIcon + to_u32(0X00001000))){
                    g_sdlDevice->drawTexture(texPtr, magicKeyOffX, 0);
                    const auto coolDownAngle = getMyHero()->getMagicCoolDownAngle(magicID);
                    const auto colorRatio = [coolDownAngle]() -> float
                    {
                        const float r = to_f(coolDownAngle) / 360.0;
                        return r * r * r * r;
                    }();

                    const auto texW = SDLDeviceHelper::getTextureWidth(texPtr);
                    const auto coverTexW = std::lround(1.41421356237309504880 * texW);
                    auto coverTexPtr = g_sdlDevice->getCover(coverTexW / 2, coolDownAngle);
                    SDLDeviceHelper::EnableTextureModColor enableModColor(coverTexPtr, colorf::fadeRGBA(colorf::RGBA(255, 51, 51, 255), colorf::GREEN + colorf::A_SHF(80), colorRatio));

                    const auto offCoverX = (coverTexW - texW) / 2;
                    g_sdlDevice->drawTexture(coverTexPtr, magicKeyOffX, 0, offCoverX, offCoverX, texW, texW);
                    magicKeyOffX += texW;
                }
            }
        }
    }

    if(getMyHero()->getSDBuffIDListOpt().has_value()){
        constexpr int buffIconDrawW = 30;
        constexpr int buffIconDrawH = 30;
        int buffIconOffX = g_sdlDevice->getRendererWidth() - buffIconDrawW;

        if(auto boardPtr = getWidget("MiniMapBoard"); boardPtr->show()){
            buffIconOffX -= boardPtr->w();
        }

        for(const auto id: getMyHero()->getSDBuffIDListOpt().value().idList | std::views::reverse){
            const auto &br = DBCOM_BUFFRECORD(id);
            fflassert(br);

            if(br.icon.gfxID != SYS_U32NIL){
                if(auto iconTexPtr = g_progUseDB->retrieve(br.icon.gfxID)){
                    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(iconTexPtr);
                    g_sdlDevice->drawTexture(iconTexPtr, buffIconOffX, 0, buffIconDrawW, buffIconDrawH, 0, 0, texW, texH);
                    buffIconOffX -= buffIconDrawW;
                }
            }
        }
    }

    for(auto p: m_ascendStrList){
        p->draw(m_viewX, m_viewY);
    }

    // draw underlay at the bottom
    // there is one pixel transparent rectangle
    const auto [winW, winH] = g_sdlDevice->getRendererSize();
    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 0), 0, winH - 4, winW, 4);

    if(getMyHero()->dead().value_or(false)){
        g_sdlDevice->fillRectangle(colorf::RGBA(128, 0, 0, 64), 0, 0, winW, winH);
    }

    m_guiManager.drawRoot();
    if(const auto selectedItemID = getMyHero()->getInvPack().getGrabbedItem().itemID){
        if(const auto &ir = DBCOM_ITEMRECORD(selectedItemID)){
            if(auto texPtr = g_itemDB->retrieve(ir.pkgGfxID | 0X01000000)){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                const auto [ptrX, ptrY] = SDLDeviceHelper::getMousePLoc();
                g_sdlDevice->drawTexture(texPtr, ptrX - texW / 2, ptrY - texH / 2);
            }
        }
    }

    switch(m_cursorState){
        case CURSOR_TEAMFLAG:
            {
                const auto teamFlagIndex = m_teamFlag.seqFrame() % 13;
                if(auto flagTex = g_progUseDB->retrieve(0X00000210 + teamFlagIndex)){
                    const auto [mouseX, mouseY] = SDLDeviceHelper::getMousePLoc();
                    g_sdlDevice->drawTexture(flagTex, DIR_NONE, mouseX, mouseY);
                }
                break;
            }
        default:
            {
                break;
            }
    }

    // draw NotifyBoard
    if(false){
        const int w = std::max<int>(g_notifyBoard->pw() + 10, 160);
        const int h = g_notifyBoard->h();
        const int x = 0;
        const int y = g_sdlDevice->getRendererHeight() - h - 133;

        g_sdlDevice->fillRectangle(colorf::GREEN + colorf::A_SHF(180), x, y, w, h);
        g_sdlDevice->drawRectangle(colorf::BLUE  + colorf::A_SHF(255), x, y, w, h);
        g_notifyBoard->drawAt(DIR_UPLEFT, x, y);
    }

    if(g_clientArgParser->drawMouseLocation){
        drawMouseLocation();
    }

    if(getRuntimeConfig<RTCFG_SHOWFPS>()){
        drawFPS();
    }
}

void ProcessRun::processEvent(const SDL_Event &event)
{
    const bool tookEvent = m_guiManager.processRootEvent(event, true, 0, 0);
    m_guiManager.purge();

    if(tookEvent){
        return;
    }

    if(getMyHero()->dead().value_or(true)){
        return;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                const auto [mouseGridX, mouseGridY] = fromPLoc2Grid(event.button.x, event.button.y);
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            if(const auto uid = getFocusUID(FOCUS_MOUSE, true)){
                                switch(m_cursorState){
                                    case CURSOR_DEFAULT:
                                        {
                                            switch(uidf::getUIDType(uid)){
                                                case UID_MON:
                                                    {
                                                        setFocusUID(FOCUS_ATTACK, uid);
                                                        trackAttack(true, uid);
                                                        break;
                                                    }
                                                case UID_NPC:
                                                    {
                                                        sendNPCEvent(uid, {}, SYS_ENTER);
                                                    }
                                                default:
                                                    {
                                                        break;
                                                    }
                                            }
                                            break;
                                        }
                                    case CURSOR_TEAMFLAG:
                                        {
                                            if(uidf::isPlayer(uid)){
                                                requestJoinTeam(uid);
                                            }
                                            break;
                                        }
                                    default:
                                        {
                                            break;
                                        }
                                }
                            }

                            else if(const auto grabbedItem = getMyHero()->getInvPack().getGrabbedItem(); grabbedItem){
                                requestDropItem(grabbedItem.itemID, grabbedItem.seqID, grabbedItem.count);
                            }

                            else if(m_mir2xMapData.validC(mouseGridX, mouseGridY) && !getGroundItemIDList(mouseGridX, mouseGridY).empty()){
                                // for small map becasue hero may be forced to put in center
                                // then there are some grid on screen are actually invalid, skip them
                                getMyHero()->emplaceAction(ActionPickUp
                                {
                                    .speed = SYS_DEFSPEED,
                                    .x = mouseGridX,
                                    .y = mouseGridY,
                                });
                            }

                            else if(m_mir2xMapData.cell(mouseGridX, mouseGridY).land.canMine() && DBCOM_ITEMRECORD(getMyHero()->getWLItem(WLG_WEAPON).itemID).equip.weapon.mine){
                                getMyHero()->emplaceAction(ActionMine
                                {
                                    .speed = SYS_DEFSPEED,
                                    .x = mouseGridX,
                                    .y = mouseGridY,
                                });
                            }
                            break;
                        }
                    case SDL_BUTTON_RIGHT:
                        {
                            switch(m_cursorState){
                                case CURSOR_NONE:
                                    {
                                        break;
                                    }
                                case CURSOR_DEFAULT:
                                    {
                                        // in mir2ei how human moves
                                        // 1. client send motion request to server
                                        // 2. client put motion lock to human
                                        // 3. server response with "+GOOD" or "+FAIL" to client
                                        // 4. if "+GOOD" client will release the motion lock
                                        // 5. if "+FAIL" client will use the backup position and direction

                                        setFocusUID(FOCUS_ATTACK, 0);
                                        setFocusUID(FOCUS_FOLLOW, 0);

                                        if(auto nUID = getFocusUID(FOCUS_MOUSE)){
                                            setFocusUID(FOCUS_FOLLOW, nUID);
                                        }

                                        else if(mathf::LDistance2(getMyHero()->currMotion()->endX, getMyHero()->currMotion()->endY, mouseGridX, mouseGridY)){
                                            // we get a valid dst to go
                                            // provide myHero with new move action command

                                            // when post move action don't use X() and Y()
                                            // since if clicks during hero moving then X() may not equal to EndX

                                            getMyHero()->emplaceAction(ActionMove
                                            {
                                                .speed = SYS_DEFSPEED,
                                                .x = getMyHero()->currMotion()->endX,
                                                .y = getMyHero()->currMotion()->endY,
                                                .aimX = mouseGridX,
                                                .aimY = mouseGridY,
                                                .extParam
                                                {
                                                    .onHorse = getMyHero()->onHorse(),
                                                },
                                            });
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        setCursor(CURSOR_DEFAULT);
                                        break;
                                    }
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case SDL_MOUSEMOTION:
            {
                break;
            }
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        {
                            centerMyHero();
                            break;
                        }
                    case SDLK_TAB:
                        {
                            requestPickUp();
                            break;
                        }
                    default:
                        {
                            if(event.key.keysym.mod & (KMOD_LALT | KMOD_RALT)){
                                switch(SDLDeviceHelper::getKeyChar(event, false)){
                                    case 'e':
                                        {
                                            std::exit(0);
                                            break;
                                        }
                                    case 'f':
                                        {
                                            g_sdlDevice->toggleWindowFullscreen();
                                            break;
                                        }
                                    default:
                                        {
                                            break;
                                        }
                                }
                            }
                            else{
                                checkMagicSpell(event);
                            }
                            break;
                        }
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessRun::loadMap(uint64_t newMapUID, int centerGX, int centerGY)
{
    fflassert(uidf::isMap(newMapUID));
    fflassert(mapUID() != newMapUID, mapUID(), newMapUID);

    const auto lastMapUID = mapUID();
    ModalStringBoard loadStringBoard;

    const auto fnUpdateLoadRatio = [&loadStringBoard, newMapUID](int ratio)
    {
        const std::string mapName = to_cstr(DBCOM_MAPRECORD(uidf::getMapID(newMapUID)).name);
        loadStringBoard.loadXML(str_printf
        (
            u8R"###( <layout>                                     )###""\n"
            u8R"###(     <par>加载地图<t color="red">%s</t></par> )###""\n"
            u8R"###(     <par>完成<t color="red">%%%d</t></par>   )###""\n"
            u8R"###( </layout>                                    )###""\n",

            mapName.substr(0, mapName.find('_')).c_str(),
            mathf::bound<int>(ratio, 0, 100)
        ));
        loadStringBoard.drawScreen(true);
    };

    g_sdlDevice->stopSoundEffect();
    fnUpdateLoadRatio(0);
    {
        const auto mapBinPtr = g_mapBinDB->retrieve(uidf::getMapID(newMapUID));
        fflassert(mapBinPtr);
        fnUpdateLoadRatio(30);

        m_mapUID = newMapUID;
        m_mir2xMapData = *mapBinPtr;
        m_groundItemIDList.clear();
        fnUpdateLoadRatio(40);

        const int winW = 1200;
        const int winH = 1200;

        const int x0 = mathf::bound<int>(centerGX - winW / 2 / SYS_MAPGRIDXP - SYS_OBJMAXW, 0, m_mir2xMapData.w());
        const int x1 = mathf::bound<int>(centerGX + winW / 2 / SYS_MAPGRIDXP + SYS_OBJMAXW, 0, m_mir2xMapData.w());
        const int y0 = mathf::bound<int>(centerGY - winH / 2 / SYS_MAPGRIDYP - SYS_OBJMAXH, 0, m_mir2xMapData.h());
        const int y1 = mathf::bound<int>(centerGY + winH / 2 / SYS_MAPGRIDYP + SYS_OBJMAXH, 0, m_mir2xMapData.h());

        int lastRatio = 0;
        int doneGridCount = 0;
        const int totalGridCount = (y1 - y0 + 1) * (x1 - x0 + 1);

        for(int y = y0; y < y1; ++y){
            for(int x = x0; x <= x1; ++x){
                if(m_mir2xMapData.validC(x, y)){
                    if((x % 2 == 0) && (y % 2 == 0)){
                        if(const auto &tile = m_mir2xMapData.tile(x, y); tile.valid){
                            g_mapDB->retrieve(tile.texID);
                        }
                    }

                    for(const int i: {0, 1}){
                        if(const auto &obj = m_mir2xMapData.cell(x, y).obj[i]; obj.valid){
                            g_mapDB->retrieve(obj.texID);
                        }
                    }
                }

                if(const auto currRatio = to_d(std::lround(to_f(doneGridCount++ * (100 - 40)) / totalGridCount)); currRatio > lastRatio){
                    lastRatio = currRatio;
                    fnUpdateLoadRatio(40 + currRatio);
                }
            }
        }
    }

    fnUpdateLoadRatio(100);
    if(auto boardPtr = dynamic_cast<MiniMapBoard *>(getWidget("MiniMapBoard"))){
        if(boardPtr->show()){
            if(boardPtr->getMiniMapTexture()){
                boardPtr->setPLoc();
            }
            else{
                boardPtr->setShow(false);
                addCBLog(CBLOG_ERR, u8"没有可用的地图");
            }
        }
    }

    const auto lastBGMIDOpt = DBCOM_MAPRECORD(uidf::isMap(lastMapUID) ? uidf::getMapID(lastMapUID): 0).bgmID;
    const auto  newBGMIDOpt = DBCOM_MAPRECORD(                          uidf::getMapID( newMapUID)   ).bgmID;

    if(lastBGMIDOpt != newBGMIDOpt){
        g_sdlDevice->stopBGM();
        if(newBGMIDOpt.has_value()){
            g_sdlDevice->playBGM(g_bgmDB->retrieve(newBGMIDOpt.value()));
        }
    }
}

int ProcessRun::checkPathGrid(int argX, int argY) const
{
    if(!m_mir2xMapData.validC(argX, argY)){
        return PF_NONE;
    }

    if(!m_mir2xMapData.cell(argX, argY).land.canThrough()){
        return PF_OBSTACLE;
    }

    // we should take EndX/EndY, not X()/Y() as occupied
    // because server only checks EndX/EndY, if we use X()/Y() to request move it just fails

    bool locked = false;
    for(auto &p: m_coList){
        if(true
                && (p.second)
                && (p.second->currMotion()->endX == argX)
                && (p.second->currMotion()->endY == argY)){
            return PF_OCCUPIED;
        }

        if(!locked
                && p.second->x() == argX
                && p.second->y() == argY){
            locked = true;
        }
    }
    return locked ? PF_LOCKED : PF_FREE;
}

bool ProcessRun::canMove(bool checkGround, int checkCreature, int argX, int argY)
{
    switch(const auto pfGrid = checkPathGrid(argX, argY)){
        case PF_FREE:
            {
                return true;
            }
        case PF_OBSTACLE:
        case PF_NONE:
            {
                return checkGround ? false : true;
            }
        case PF_OCCUPIED:
        case PF_LOCKED:
            {
                switch(checkCreature){
                    case 0:
                    case 1:
                        {
                            return true;
                        }
                    case 2:
                        {
                            return false;
                        }
                    default:
                        {
                            throw fflvalue(checkCreature);
                        }
                }
            }
        default:
            {
                throw fflvalue(checkGround, checkCreature, argX, argY, pfGrid);
            }
    }
}

bool ProcessRun::canMove(bool checkGround, int checkCreature, int srcX, int srcY, int dstX, int dstY)
{
    return oneStepCost(nullptr, checkGround, checkCreature, srcX, srcY, DIR_BEGIN, dstX, dstY).value_or(-1.00) >= 0.0;
}

std::optional<double> ProcessRun::oneStepCost(const ClientPathFinder *finder, bool checkGround, int checkCreature, int srcX, int srcY, int srcDir, int dstX, int dstY) const
{
    fflassert(checkCreature >= 0, checkCreature);
    fflassert(checkCreature <= 2, checkCreature);

    int hopSize = -1;
    switch(mathf::LDistance2(srcX, srcY, dstX, dstY)){
        case  1:
        case  2: hopSize = 1; break;
        case  4:
        case  8: hopSize = 2; break;
        case  9:
        case 18: hopSize = 3; break;
        case  0: return .0;
        default: return {};
    }

    double gridExtraPen = 0.00;
    const auto hopDir = pathf::getOffDir(srcX, srcY, dstX, dstY);

    for(int stepSize = 1; stepSize <= hopSize; ++stepSize){
        const auto [currX, currY] = pathf::getFrontGLoc(srcX, srcY, hopDir, stepSize);
        switch(const auto pfGrid = finder ? finder->getGrid(currX, currY) : this->checkPathGrid(currX, currY)){
            case PF_FREE:
                {
                    break;
                }
            case PF_LOCKED:
            case PF_OCCUPIED:
                {
                    switch(checkCreature){
                        case 0:
                            {
                                break;
                            }
                        case 1:
                            {
                                gridExtraPen += 100.00;
                                break;
                            }
                        case 2:
                            {
                                return {};
                            }
                        default:
                            {
                                throw fflvalue(checkCreature);
                            }
                    }
                    break;
                }
            case PF_NONE:
            case PF_OBSTACLE:
                {
                    if(checkGround){
                        return {};
                    }

                    gridExtraPen += 10000.00;
                    break;
                }
            default:
                {
                    throw fflvalue(currX, currY, pfGrid);
                }
        }
    }

    return 1.00 + hopSize * 0.10 + gridExtraPen + pathf::getDirAbsDiff(srcDir, hopDir) * 0.01;
}

bool ProcessRun::luaCommand(const char *luaCmdString)
{
    if(!luaCmdString){
        return false;
    }

    const auto callResult = m_luaModule.execString("%s", luaCmdString);
    if(callResult.valid()){
        return true;
    }

    const auto fnReplaceTabChar = [](std::string line) -> std::string
    {
        for(auto pos = line.find('\t'); pos != std::string::npos; pos = line.find('\t')){
            line.replace(pos, 1, "    ");
        }
        return line;
    };

    const sol::error err = callResult;
    std::stringstream errStream(err.what());

    std::string errLine;
    while(std::getline(errStream, errLine, '\n')){
        addCBLog(CBLOG_ERR, to_u8cstr(fnReplaceTabChar(errLine)));
    }
    return true;
}

bool ProcessRun::userCommand(const char *userCmdString)
{
    if(!userCmdString){
        return false;
    }

    const char *beginPtr = userCmdString;
    const char *endPtr   = userCmdString + std::strlen(userCmdString);

    std::vector<std::string> tokenList;
    while(true){
        beginPtr = std::find_if_not(beginPtr, endPtr, [](char chByte)
        {
            return chByte == ' ';
        });

        if(beginPtr == endPtr){
            break;
        }

        const char *donePtr = std::find(beginPtr, endPtr, ' ');
        tokenList.emplace_back(beginPtr, donePtr);
        beginPtr = donePtr;
    }

    if(tokenList.empty()){
        return true;
    }

    int matchCount = 0;
    UserCommand *entryPtr = nullptr;

    for(auto &entry : m_userCommandList){
        if(entry.command.substr(0, tokenList[0].size()) == tokenList[0]){
            entryPtr = &entry;
            matchCount++;
        }
    }

    switch(matchCount){
        case 0:
            {
                addCBLog(CBLOG_ERR, u8"-> Invalid user command: %s", tokenList[0].c_str());
                return true;
            }
        case 1:
            {
                if(!entryPtr->callback){
                    throw fflerror("command callback is not callable: %s", entryPtr->command.c_str());
                }

                entryPtr->callback(tokenList);
                return true;
            }
        default:
            {
                addCBLog(CBLOG_ERR, u8">> Ambiguous user command: %s", tokenList[0].c_str());
                for(auto &entry : m_userCommandList){
                    if(entry.command.substr(0, tokenList[0].size()) == tokenList[0]){
                        addCBLog(CBLOG_ERR, u8">> Candicate: %s", entry.command.c_str());
                    }
                }
                return true;
            }
    }
}

std::vector<int> ProcessRun::GetPlayerList()
{
    std::vector<int> result;
    for(auto p = m_coList.begin(); p != m_coList.end();){
        if(p->second->visible()){
            switch(p->second->type()){
                case UID_PLY:
                    {
                        result.push_back(p->second->UID());
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
            ++p;
        }else{
            p = m_coList.erase(p);
        }
    }
    return result;
}

void ProcessRun::RegisterUserCommand()
{
    const auto fnCreateLuaCmdString = [](const std::string &luaCommand, const std::vector<std::string> &parmList) -> std::string
    {
        // don't use parmList[0] as lua function name
        // user commands can have shortcut matching which fails in lua

        switch(parmList.size()){
            case 0:
                {
                    throw fflerror("argument list empty");
                }
            case 1:
                {
                    return luaCommand + "()";
                }
            default:
                {
                    std::string luaString = luaCommand + "(" + parmList[1];
                    for(size_t i = 2; i < parmList.size(); ++i){
                        luaString += (std::string(", ") + parmList[i]);
                    }
                    return luaString + ")";
                }
        }
    };

    m_userCommandList.emplace_back("moveTo", [&fnCreateLuaCmdString, this](const std::vector<std::string> &parmList) -> int
    {
        return luaCommand(fnCreateLuaCmdString("moveTo", parmList).c_str());
    });

    m_userCommandList.emplace_back("luaEditor", [this](const std::vector<std::string> &) -> int
    {
        addCBLog(CBLOG_ERR, u8">> Lua editor not implemented yet");
        return 0;
    });

    m_userCommandList.emplace_back("makeItem", [this](const std::vector<std::string> &parmList) -> int
    {
        switch(parmList.size()){
            case 1 + 0:
                {
                    addCBLog(CBLOG_SYS, u8"@make 物品名字");
                    return 1;
                }
            case 1 + 1:
            case 1 + 2:
                {
                    if(const auto itemID = DBCOM_ITEMID(to_u8cstr(parmList[1]))){
                        const auto count = [&parmList]() -> int
                        {
                            if(parmList.size() == 1 + 1){
                                return 1;
                            }

                            try{
                                if(const auto count = std::stoi(parmList[2]); count > 1){
                                    return count;
                                }
                                else{
                                    return -1;
                                }
                            }
                            catch(...){
                                return -1;
                            }
                        }();

                        if(count > 1){
                            requestMakeItem(itemID, count);
                            return 0;
                        }
                        else{
                            addCBLog(CBLOG_ERR, u8"无效的物品数量：%s", to_cstr(parmList[2]));
                            return 1;
                        }
                    }
                    else{
                        addCBLog(CBLOG_ERR, u8"无效的物品名：%s", to_cstr(parmList[1]));
                        return 1;
                    }
                }
            default:
                {
                    addCBLog(CBLOG_ERR, u8"Invalid argument to @make");
                    return 1;
                }
        }
    });

    m_userCommandList.emplace_back("getAttackUID", [this](const std::vector<std::string> &) -> int
    {
        addCBLog(CBLOG_ERR, to_u8cstr(std::to_string(getFocusUID(FOCUS_ATTACK))));
        return 1;
    });

    m_userCommandList.emplace_back("addHP", [this](const std::vector<std::string> &parms) -> int
    {
        if(parms.size() >= 2){
            requestAddHP(to_u64(std::stol(parms.at(1))));
        }
        return 0;
    });

    m_userCommandList.emplace_back("addExp", [this](const std::vector<std::string> &parms) -> int
    {
        if(parms.size() >= 2){
            requestAddExp(to_u64(std::stol(parms.at(1))));
        }
        return 0;
    });

    m_userCommandList.emplace_back("killPets", [this](const std::vector<std::string> &) -> int
    {
        requestKillPets();
        addCBLog(CBLOG_SYS, u8"杀死所有宝宝");
        return 0;
    });

    m_userCommandList.emplace_back("die", [this](const std::vector<std::string> &) -> int
    {
        requestDie();
        addCBLog(CBLOG_SYS, u8"自杀");
        return 0;
    });

    m_userCommandList.emplace_back("revive", [this](const std::vector<std::string> &) -> int
    {
        if(getMyHero()->dead().value_or(true)){
            requestAddHP(1);
            addCBLog(CBLOG_SYS, u8"复活");
        }
        return 0;
    });

    m_userCommandList.emplace_back("help", [this](const std::vector<std::string> &) -> int
    {
        for(const auto &cmd: m_userCommandList){
            addCBLog(CBLOG_SYS, u8"@%s", cmd.command.c_str());
        }
        return 0;
    });
}

void ProcessRun::registerLuaExport(ClientLuaModule *luaModulePtr)
{
    fflassert(luaModulePtr);

    luaModulePtr->execString("CBLOG_DEF = %d", CBLOG_DEF);
    luaModulePtr->execString("CBLOG_SYS = %d", CBLOG_SYS);
    luaModulePtr->execString("CBLOG_DBG = %d", CBLOG_DBG);
    luaModulePtr->execString("CBLOG_ERR = %d", CBLOG_ERR);

    luaModulePtr->bindFunction("addCBLog", [this](sol::object logType, sol::object logInfo)
    {
        if(logType.is<int>() && logInfo.is<std::string>()){
            switch(logType.as<int>()){
                case CBLOG_DEF:
                case CBLOG_SYS:
                case CBLOG_DBG:
                case CBLOG_ERR:
                    {
                        addCBLog(logType.as<int>(), to_u8cstr(logInfo.as<std::string>()));
                        return;
                    }
                default:
                    {
                        addCBLog(CBLOG_ERR, u8"Invalid argument: logType requires [CBLOG_DEF, CBLOG_SYS, CBLOG_DBG, CBLOG_ERR]");
                        return;
                    }
            }
        }

        if(logType.is<int>()){
            addCBLog(CBLOG_ERR, str_printf(u8"Invalid argument: addCBLog(%d, \"?\")", logType.as<int>()).c_str());
            return;
        }

        if(logInfo.is<std::string>()){
            addCBLog(CBLOG_ERR, str_printf(u8"Invalid argument: addCBLog(?, \"%s\")", logInfo.as<std::string>().c_str()).c_str());
            return;
        }

        addCBLog(CBLOG_ERR, u8"Invalid argument: addCBLog(?, \"?\")");
        return;
    });

    // register command playerList
    // return a table (userData) to lua for ipairs() check
    luaModulePtr->bindFunction("playerList", [this](sol::this_state stThisLua)
    {
        return sol::make_object(sol::state_view(stThisLua), GetPlayerList());
    });

    // register command moveTo(x, y)
    // wait for server to move player if possible
    luaModulePtr->bindFunction("moveTo", [this](sol::variadic_args args)
    {
        int locX = 0;
        int locY = 0;

        uint32_t argMapID = 0;
        uint64_t argMapUID = 0;

        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 0:
                {
                    argMapUID = mapUID();
                    std::tie(locX, locY) = getRandLoc(uidf::getMapID(mapUID()));
                    break;
                }
            case 1:
                {
                    if(!argList[0].is<lua_Integer>()){
                        throw fflerror("invalid arguments: moveTo(mapID: int)");
                    }

                    argMapID = to_u32(argList[0].as<int>());
                    std::tie(locX, locY) = getRandLoc(argMapID);
                    break;
                }
            case 2:
                {
                    if(!(argList[0].is<int>() && argList[1].is<int>())){
                        throw fflerror("invalid arguments: moveTo(x: int, y: int)");
                    }

                    argMapUID = mapUID();
                    locX  = argList[0].as<int>();
                    locY  = argList[1].as<int>();
                    break;
                }
            case 3:
                {
                    if(!(argList[0].is<lua_Integer>() && argList[1].is<int>() && argList[2].is<int>())){
                        throw fflerror("invalid arguments: moveTo(mapID: int, x: int, y: int)");
                    }

                    argMapID = to_u32(argList[0].as<int>());
                    locX = argList[1].as<int>();
                    locY = argList[2].as<int>();
                    break;
                }
            default:
                {
                    throw fflerror("invalid arguments: moveTo([mapID: int,] x: int, y: int)");
                }
        }

        const auto fnRequestSpaceMove = [locX, locY, this](uint64_t argMapUID)
        {
            if(requestSpaceMove(argMapUID, locX, locY)){
                addCBLog(CBLOG_SYS, u8"Move request (mapName = %s, x = %d, y = %d) sent", to_cstr(DBCOM_MAPRECORD(uidf::getMapID(argMapUID)).name), locX, locY);
            }
            else{
                addCBLog(CBLOG_ERR, u8"Move request (mapName = %s, x = %d, y = %d) failed", to_cstr(DBCOM_MAPRECORD(uidf::getMapID(argMapUID)).name), locX, locY);
            }
        };

        if(argMapUID){
            fnRequestSpaceMove(argMapUID);
        }
        else if(argMapID){
            queryMapBaseUID(argMapID, fnRequestSpaceMove);
        }
        else{
            addCBLog(CBLOG_ERR, u8"Move request failed: No map provided");
        }
    });

    luaModulePtr->bindFunction("makeItem", [this](sol::variadic_args args)
    {
        size_t count = 1;
        uint32_t itemID = 0;

        const std::vector<sol::object> argList(args.begin(), args.end());
        switch(argList.size()){
            case 1:
            case 2:
                {
                    if(argList[0].is<lua_Integer>()){
                        itemID = to_u32(argList[0].as<lua_Integer>());
                    }
                    else if(argList[0].is<std::string>()){
                        itemID = DBCOM_ITEMID(to_u8cstr(argList[0].as<std::string>()));
                    }
                    else{
                        throw fflerror("invalid arguments: makeItem(itemID: [int,string])");
                    }

                    if(argList.size() == 2){
                        if(argList[1].is<lua_Integer>()){
                            count = to_uz(argList[1].as<lua_Integer>());
                        }
                        else{
                            throw fflerror("invalid arguments: makeItem(itemID: [int,string], count: int)");
                        }
                    }
                    break;
                }
            default:
                {
                    throw fflerror("invalid arguments: makeItem(itemID: [int,string], count: int)");
                }
        }

        if(!itemID){
            throw fflerror("invalid itemID: %llu", to_llu(itemID));
        }

        if(count <= 0){
            throw fflerror("invalid count: %llu", to_llu(count));
        }

        requestMakeItem(itemID, count);
    });

    // register command ``listPlayerInfo"
    // this command call to get a player info table and print to out port
    luaModulePtr->execString(R"#(
        function listPlayerInfo ()
            for k, v in ipairs(playerList())
            do
                addLog(CBLOG_SYS, "> " .. tostring(v))
            end
        end
    )#");

    // register command ``help"
    // part-1: divide into two parts, part-1 create the table for help
    luaModulePtr->execString(R"#(
        helpInfoTable = {
            wear     = "put on different dress",
            moveTo   = "move to other position on current map",
            randMove = "random move on current map"
        }
    )#");

    // part-2: make up the function to print the table entry
    luaModulePtr->execString(R"#(
        function help (queryKey)
            if helpInfoTable[queryKey]
            then
                printLine(0, "> ", helpInfoTable[queryKey])
            else
                printLine(2, "> ", "No entry find for input")
            end
        end
    )#");

    // register command ``myHero.xxx"
    // I need to insert a table to micmic a instance myHero in the future
    luaModulePtr->bindFunction("myHero_dress", [this](int nDress)
    {
        if(nDress >= 0){
            getMyHero()->setWLItem(WLG_DRESS, SDItem
            {
                .itemID = to_u32(nDress),
            });
        }
    });

    // register command ``myHero.xxx"
    // I need to insert a table to micmic a instance myHero in the future
    luaModulePtr->bindFunction("myHero_weapon", [this](int nWeapon)
    {
        if(nWeapon >= 0){
            getMyHero()->setWLItem(WLG_WEAPON, SDItem
            {
                .itemID = to_u32(nWeapon),
            });
        }
    });

    luaModulePtr->bindFunction("addExp", [this](uint64_t exp)
    {
        requestAddExp(exp);
    });
}

void ProcessRun::addCBLog(int logType, const char8_t *format, ...)
{
    std::u8string logStr;
    str_format(format, logStr);
    dynamic_cast<ControlBoard *>(getGUIManager()->getWidget("ControlBoard"))->addLog(logType, to_cstr(logStr));
}

void ProcessRun::addCBParLog(const char8_t *format, ...)
{
    std::u8string logStr;
    str_format(format, logStr);
    dynamic_cast<ControlBoard *>(getGUIManager()->getWidget("ControlBoard"))->addParLog(to_cstr(logStr));
}

ClientCreature *ProcessRun::findUID(uint64_t uid, bool checkVisible) const
{
    if(!uid){
        return nullptr;
    }

    // TODO don't remove invisible creatures, this causes too much crash
    // let ProcessRun::update() loop clean it

    if(auto p = m_coList.find(uid); p != m_coList.end()){
        if(p->second->UID() != uid){
            throw fflerror("invalid creature: %p, UID = %llu", to_cvptr(p->second.get()), to_llu(p->second->UID()));
        }

        if(!checkVisible || p->second->visible()){
            return p->second.get();
        }
    }
    return nullptr;
}

bool ProcessRun::trackAttack(bool bForce, uint64_t nUID)
{
    if(findUID(nUID)){
        if(bForce || getMyHero()->stayIdle()){
            return getMyHero()->emplaceAction(ActionAttack
            {
                .speed = SYS_DEFSPEED,
                .x = getMyHero()->currMotion()->endX,
                .y = getMyHero()->currMotion()->endY,
                .aimUID = nUID,
                .extParam
                {
                    .magicID = [this]() -> uint32_t
                    {
                        if(getMyHero()->getNextStrike()){
                            getMyHero()->setNextStrike(false);
                            return DBCOM_MAGICID(u8"攻杀剑术");
                        }
                        else if(getMyHero()->hasSwingMagic(DBCOM_MAGICID(u8"莲月剑法"))){
                            getMyHero()->toggleSwingMagic(DBCOM_MAGICID(u8"莲月剑法"), false);
                            return DBCOM_MAGICID(u8"莲月剑法");
                        }
                        else if(getMyHero()->hasSwingMagic(DBCOM_MAGICID(u8"翔空剑法"))){
                            getMyHero()->toggleSwingMagic(DBCOM_MAGICID(u8"翔空剑法"), false);
                            return DBCOM_MAGICID(u8"翔空剑法");
                        }
                        else if(getMyHero()->hasSwingMagic(DBCOM_MAGICID(u8"烈火剑法"))){
                            getMyHero()->toggleSwingMagic(DBCOM_MAGICID(u8"烈火剑法"), false);
                            return DBCOM_MAGICID(u8"烈火剑法");
                        }
                        else if(getMyHero()->hasSwingMagic(DBCOM_MAGICID(u8"十方斩"))){
                            return DBCOM_MAGICID(u8"十方斩");
                        }
                        else if(getMyHero()->hasSwingMagic(DBCOM_MAGICID(u8"半月弯刀"))){
                            return DBCOM_MAGICID(u8"半月弯刀");
                        }
                        else{
                            return DBCOM_MAGICID(u8"物理攻击");
                        }
                    }(),
                },
            });
        }
    }
    return false;
}

void ProcessRun::addAscendStr(int nType, int nValue, int nX, int nY)
{
    m_ascendStrList.push_back(std::make_unique<AscendStr>(nType, nValue, nX, nY));
}

void ProcessRun::centerMyHero()
{
    const auto nMotion     = getMyHero()->currMotion()->type;
    const auto nDirection  = getMyHero()->currMotion()->direction;
    const auto nX          = getMyHero()->currMotion()->x;
    const auto nY          = getMyHero()->currMotion()->y;
    const auto currFrame   = getMyHero()->currMotion()->frame;
    const auto frameCount  = getMyHero()->getFrameCount(getMyHero()->currMotion());

    if(frameCount <= 0){
        throw fflerror("invalid frame count: %d", frameCount);
    }

    const auto fnSetOff = [this, nX, nY, nDirection, currFrame, frameCount](int stepLen)
    {
        const auto [rendererWidth, rendererHeight] = g_sdlDevice->getRendererSize();
        const auto controlBoardPtr = dynamic_cast<ControlBoard *>(getGUIManager()->getWidget("ControlBoard"));
        const auto showWindowW = rendererWidth;
        const auto showWindowH = rendererHeight - controlBoardPtr->h();

        switch(stepLen){
            case 0:
                {
                    m_viewX = nX * SYS_MAPGRIDXP - showWindowW / 2;
                    m_viewY = nY * SYS_MAPGRIDYP - showWindowH / 2;
                    return;
                }
            case 1:
            case 2:
            case 3:
                {
                    const auto [shiftX, shiftY] = getMyHero()->getShift(getMyHero()->currMotion()->frame);
                    m_viewX = nX * SYS_MAPGRIDXP + shiftX - showWindowW / 2;
                    m_viewY = nY * SYS_MAPGRIDYP + shiftY - showWindowH / 2;
                    return;
                }
            default:
                {
                    return;
                }
        }
    };

    switch(nMotion){
        case MOTION_WALK:
        case MOTION_ONHORSEWALK:
            {
                fnSetOff(1);
                break;
            }
        case MOTION_RUN:
            {
                fnSetOff(2);
                break;
            }
        case MOTION_ONHORSERUN:
            {
                fnSetOff(3);
                break;
            }
        default:
            {
                fnSetOff(0);
                break;
            }
    }
}

std::tuple<int, int> ProcessRun::getRandLoc(uint32_t reqMapID, size_t tryCount)
{
    std::shared_ptr<Mir2xMapData> newPtr;
    const auto mapBinPtr = [reqMapID, &newPtr, this]() -> const Mir2xMapData *
    {
        if(reqMapID == 0 || reqMapID == mapID()){
            return &m_mir2xMapData;
        }

        if((newPtr = g_mapBinDB->retrieve(reqMapID))){
            return newPtr.get();
        }
        return nullptr;
    }();

    if(!mapBinPtr){
        throw fflerror("failed to find map with mapID = %llu", to_llu(reqMapID));
    }

    for(size_t i = 0; (tryCount == 0) || (i < tryCount); ++i){
        const int nX = std::rand() % mapBinPtr->w();
        const int nY = std::rand() % mapBinPtr->h();

        if(mapBinPtr->groundValid(nX, nY)){
            return {nX, nY};
        }
    }

    throw fflerror("can not find a valid location on mapID %llu by %zu tries", to_llu(reqMapID), tryCount);
}

bool ProcessRun::requestSpaceMove(uint64_t nMapUID, int nX, int nY)
{
    const auto mapBinPtr = g_mapBinDB->retrieve(uidf::getMapID(nMapUID));
    if(!mapBinPtr){
        return false;
    }

    if(!(mapBinPtr->validC(nX, nY) && mapBinPtr->cell(nX, nY).land.canThrough())){
        return false;
    }

    CMRequestSpaceMove cmRSM;
    std::memset(&cmRSM, 0, sizeof(cmRSM));

    cmRSM.mapUID = nMapUID;
    cmRSM.X      = nX;
    cmRSM.Y      = nY;

    g_client->send({CM_REQUESTSPACEMOVE, cmRSM});
    return true;
}

void ProcessRun::requestSetMagicKey(uint32_t magicID, char key)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9'));

    CMSetMagicKey cmSMK;
    std::memset(&cmSMK, 0, sizeof(cmSMK));

    cmSMK.magicID = magicID;
    cmSMK.key = key;

    g_client->send({CM_SETMAGICKEY, cmSMK});
}

void ProcessRun::requestRemoveSecuredItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(DBCOM_ITEMRECORD(itemID));

    CMRequestRetrieveSecuredItem cmRRSI;
    std::memset(&cmRRSI, 0, sizeof(cmRRSI));

    cmRRSI.itemID = itemID;
    cmRRSI.seqID = seqID;

    g_client->send({CM_REQUESTRETRIEVESECUREDITEM, cmRRSI});
}

void ProcessRun::requestJoinTeam(uint64_t uid)
{
    fflassert(uidf::isPlayer(uid));

    CMRequestJoinTeam cmRJT;
    std::memset(&cmRJT, 0, sizeof(cmRJT));

    cmRJT.uid = uid;
    g_client->send({CM_REQUESTJOINTEAM, cmRJT});
}

void ProcessRun::requestLeaveTeam(uint64_t uid)
{
    fflassert(uidf::isPlayer(uid));

    CMRequestLeaveTeam cmRLT;
    std::memset(&cmRLT, 0, sizeof(cmRLT));

    cmRLT.uid = uid;
    g_client->send({CM_REQUESTLEAVETEAM, cmRLT});
}

void ProcessRun::requestLatestChatMessage(const std::vector<uint64_t> &cpids, size_t limitCount, bool sendIncluded, bool recvIncluded)
{
    CMRequestLatestChatMessage cmRLCM;
    std::memset(&cmRLCM, 0, sizeof(cmRLCM));

    if(cpids.size() > cmRLCM.cpidList.capacity()){
        throw fflerror("query of %zu cpids exceeds capacity %zu", cpids.size(), cmRLCM.cpidList.capacity());
    }

    for(const auto &cpid: cpids){
        cmRLCM.cpidList.push(cpid);
    }

    cmRLCM.limitCount  = limitCount;
    cmRLCM.includeSend = sendIncluded;
    cmRLCM.includeRecv = recvIncluded;

    g_client->send({CM_REQUESTLATESTCHATMESSAGE, cmRLCM});
}

void ProcessRun::requestDie()
{
    g_client->send(CM_REQUESTDIE);
}

void ProcessRun::requestKillPets()
{
    g_client->send(CM_REQUESTKILLPETS);
}

void ProcessRun::requestAddHP(uint64_t hp)
{
    if(hp){
        CMRequestAddHP cmRAHP;
        std::memset(&cmRAHP, 0, sizeof(cmRAHP));

        cmRAHP.addHP = hp;
        g_client->send({CM_REQUESTADDHP, cmRAHP});
    }
}

void ProcessRun::requestAddExp(uint64_t exp)
{
    if(exp){
        CMRequestAddExp cmRAE;
        std::memset(&cmRAE, 0, sizeof(cmRAE));

        cmRAE.addExp = exp;
        g_client->send({CM_REQUESTADDEXP, cmRAE});
    }
}

void ProcessRun::queryCORecord(uint64_t nUID) const
{
    CMQueryCORecord cmQCOR;
    std::memset(&cmQCOR, 0, sizeof(cmQCOR));

    cmQCOR.AimUID = nUID;
    g_client->send({CM_QUERYCORECORD, cmQCOR});
}

void ProcessRun::onActionSpawn(uint64_t uid, const ActionNode &action)
{
    fflassert(uid);
    fflassert(action.type == ACTION_SPAWN);

    if(uidf::getUIDType(uid) != UID_MON){
        queryCORecord(uid);
        return;
    }

    switch(uidf::getMonsterID(uid)){
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                m_actionBlocker.insert(uid);
                addCBLog(CBLOG_SYS, u8"使用魔法: 召唤骷髅");
                addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                {
                    u8"召唤骷髅",
                    u8"运行",
                    action.x,
                    action.y,

                }))->addTrigger([action, uid, this](BaseMagic *magicPtr) -> bool
                {
                    if(magicPtr->frame() < 10){
                        return false;
                    }

                    m_coList[uid].reset(new ClientTaoSkeleton(uid, this, ActionStand
                    {
                        .direction = DIR_DOWNLEFT,
                        .x = action.x,
                        .y = action.y,
                    }));

                    m_actionBlocker.erase(uid);

                    queryUIDBuff(uid);
                    queryCORecord(uid);
                    return true;
                });
                return;
            }
        case DBCOM_MONSTERID(u8"超强骷髅"):
            {
                m_actionBlocker.insert(uid);
                addCBLog(CBLOG_SYS, u8"使用魔法: 超强召唤骷髅");
                addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                {
                    u8"超强召唤骷髅",
                    u8"运行",
                    action.x,
                    action.y,

                }))->addTrigger([action, uid, this](BaseMagic *magicPtr) -> bool
                {
                    if(magicPtr->frame() < 10){
                        return false;
                    }

                    m_coList[uid].reset(new ClientTaoSkeletonExt(uid, this, ActionStand
                    {
                        .direction = DIR_DOWNLEFT,
                        .x = action.x,
                        .y = action.y,
                    }));

                    m_actionBlocker.erase(uid);

                    queryUIDBuff(uid);
                    queryCORecord(uid);
                    return true;
                });
                return;
            }
        case DBCOM_MONSTERID(u8"神兽"):
            {
                addCBLog(CBLOG_SYS, u8"使用魔法: 召唤神兽");
                m_coList[uid].reset(new ClientTaoDog(uid, this, action));

                queryUIDBuff(uid);
                queryCORecord(uid);
                return;
            }
        default:
            {
                m_coList[uid].reset(ClientMonster::create(uid, this, action));

                queryUIDBuff(uid);
                queryCORecord(uid);
                return;
            }
    }
}

void ProcessRun::sendNPCEvent(uint64_t uid, std::string path, std::string event, std::optional<std::string> value)
{
    fflassert(uidf::getUIDType(uid) == UID_NPC);

    CMNPCEvent cmNPCE;
    std::memset(&cmNPCE, 0, sizeof(cmNPCE));

    cmNPCE.uid = uid;

    fflassert(path.size() < sizeof(cmNPCE.path));
    std::strcpy(cmNPCE.path, path.c_str());

    fflassert(!event.empty());
    fflassert(event.size() < sizeof(cmNPCE.event));
    std::strcpy(cmNPCE.event, event.c_str());

    if(value.has_value()){
        fflassert(value.value().size() < sizeof(cmNPCE.value));
        cmNPCE.valueSize = to_i16(value.value().size());
        std::strcpy(cmNPCE.value, value.value().c_str());
    }
    else{
        cmNPCE.valueSize = -1;
    }
    g_client->send({CM_NPCEVENT, cmNPCE});
}

void ProcessRun::drawGroundItem(int x0, int y0, int x1, int y1) const
{
    for(const auto &p: m_groundItemIDList){
        const auto [x, y] = p.first;
        if(!(x >= x0 && x < x1 && y >= y0 && y < y1)){
            continue;
        }

        for(const auto itemID: p.second){
            const auto &ir = DBCOM_ITEMRECORD(itemID);
            if(!ir){
                throw fflerror("invalid itemID: %llu", to_llu(itemID));
            }

            if(ir.pkgGfxID < 0){
                continue;
            }

            auto texPtr = g_itemDB->retrieve(ir.pkgGfxID);
            if(!texPtr){
                continue;
            }

            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
            const int drawPX = x * SYS_MAPGRIDXP - m_viewX + SYS_MAPGRIDXP / 2 - texW / 2;
            const int drawPY = y * SYS_MAPGRIDYP - m_viewY + SYS_MAPGRIDYP / 2 - texH / 2;

            const auto [mouseX, mouseY] = SDLDeviceHelper::getMousePLoc();
            const int mouseGridX = (mouseX + m_viewX) / SYS_MAPGRIDXP;
            const int mouseGridY = (mouseY + m_viewY) / SYS_MAPGRIDYP;

            bool mouseOver = false;
            if(mouseGridX == x && mouseGridY == y){
                mouseOver = true;
                SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_ADD);
            }
            else{
                SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_BLEND);
            }

            // draw item shadow
            SDL_SetTextureColorMod(texPtr, 0, 0, 0);
            SDL_SetTextureAlphaMod(texPtr, 128);
            g_sdlDevice->drawTexture(texPtr, drawPX + 1, drawPY - 1);

            // draw item body
            SDL_SetTextureColorMod(texPtr, 255, 255, 255);
            SDL_SetTextureAlphaMod(texPtr, 255);
            g_sdlDevice->drawTexture(texPtr, drawPX, drawPY);

            if(mouseOver){
                LabelBoard itemName(DIR_UPLEFT, 0, 0, ir.name, 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF));
                const int drawNameX = x * SYS_MAPGRIDXP - m_viewX + SYS_MAPGRIDXP / 2 - itemName.w() / 2;
                const int drawNameY = y * SYS_MAPGRIDYP - m_viewY + SYS_MAPGRIDYP / 2 - itemName.h() / 2 - 20;
                itemName.drawAt(DIR_UPLEFT, drawNameX, drawNameY);
            }
        }
    }
}

void ProcessRun::drawTile(int x0, int y0, int x1, int y1) const
{
    for(int y = y0; y < y1; ++y){
        for(int x = x0; x <= x1; ++x){
            if(m_mir2xMapData.validC(x, y) && !(x % 2) && !(y % 2)){
                if(const auto &tile = m_mir2xMapData.tile(x, y); tile.valid){
                    if(auto texPtr = g_mapDB->retrieve(tile.texID)){
                        g_sdlDevice->drawTexture(texPtr, x * SYS_MAPGRIDXP - m_viewX, y * SYS_MAPGRIDYP - m_viewY);
                    }
                }
            }
        }
    }
}

void ProcessRun::drawObject(int x, int y, int objd, bool alpha) const
{
    if(!m_mir2xMapData.validC(x, y)){
        return;
    }

    for(const int i: {0, 1}){
        const auto &obj = m_mir2xMapData.cell(x, y).obj[i];
        if(!obj.valid){
            continue;
        }

        if(obj.depth != objd){
            continue;
        }

        uint32_t imageId = obj.texID;
        if(obj.animated){
            const int fileIndex = to_d(imageId >> 16);
            if(false
                    || fileIndex == 11
                    || fileIndex == 26
                    || fileIndex == 41
                    || fileIndex == 56
                    || fileIndex == 71){ // TODO remove this check
                imageId += m_aniTimer.frame(obj.tickType, obj.frameCount);
            }
        }

        if(auto texPtr = g_mapDB->retrieve(imageId)){
            const int texH = SDLDeviceHelper::getTextureHeight(texPtr);
            const auto drawAlphaObj = [&obj, alpha]() -> uint8_t
            {
                if(obj.alpha){
                    return 96;
                }

                if(alpha){
                    return 128;
                }
                return 255;
            }();

            SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, colorf::RGBA(0XFF, 0XFF, 0XFF, drawAlphaObj));
            g_sdlDevice->drawTexture(texPtr, x * SYS_MAPGRIDXP - m_viewX, (y + 1) * SYS_MAPGRIDYP - m_viewY - texH);
        }
    }
}

void ProcessRun::drawRotateStar(int x0, int y0, int x1, int y1) const
{
    if(m_starRatio > 1.0){
        return;
    }

    auto texPtr = g_progUseDB->retrieve(0X00000090);
    if(!texPtr){
        return;
    }

    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
    const auto currSize = to_d(std::lround(m_starRatio * texW / 2.50));

    for(const auto &p: m_groundItemIDList){
        const auto [x, y] = p.first;
        if(p.second.empty()){
            throw fflerror("empty ground item list at (%d, %d)", x, y);
        }

        if(!(x >= x0 && x < x1 && y >= y0 && y < y1)){
            continue;
        }

        const auto drawPX = x * SYS_MAPGRIDXP - m_viewX + SYS_MAPGRIDXP / 2 - currSize / 2;
        const auto drawPY = y * SYS_MAPGRIDYP - m_viewY + SYS_MAPGRIDYP / 2 - currSize / 2;

        // TODO make this to be more informative
        // use different color of rotating star for different type

        SDL_SetTextureAlphaMod(texPtr, 128);
        g_sdlDevice->drawTextureEx(texPtr, 0, 0, texW, texH, drawPX, drawPY, currSize, currSize, currSize / 2, currSize / 2, std::lround(m_starRatio * 360.0));
    }
}

std::tuple<int, int> ProcessRun::getACNum(const std::string &name) const
{
    if(name == "AC"){
        return {1, 2};
    }

    else if(name == "DC"){
        return {2, 3};
    }

    else if(name == "MA"){
        return {3, 4};
    }

    else if(name == "MC"){
        return {4, 5};
    }

    else{
        throw fflerror("invalid argument: %s", name.c_str());
    }
}

void ProcessRun::drawMouseLocation() const
{
    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 230), 0, 0, 200, 60);

    const auto [mouseX, mouseY] = SDLDeviceHelper::getMousePLoc();
    const auto locPixel = str_printf(u8"Pixel: %d, %d", mouseX, mouseY);
    const auto locGrid  = str_printf(u8"Grid: %d, %d", (mouseX + m_viewX) / SYS_MAPGRIDXP, (mouseY + m_viewY) / SYS_MAPGRIDYP);

    LabelBoard locPixelBoard(DIR_UPLEFT, 10, 10, locPixel.c_str(), 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0X00));
    LabelBoard locGridBoard (DIR_UPLEFT, 10, 30, locGrid .c_str(), 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0X00));

    locPixelBoard.drawRoot();
    locGridBoard .drawRoot();
}

void ProcessRun::drawFPS() const
{
    const auto fpsStr = std::to_string(g_sdlDevice->getFPS());
    LabelBoard fpsBoard(DIR_UPLEFT, 0, 0, to_u8cstr(fpsStr), 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF));

    const int winWidth = g_sdlDevice->getRendererWidth();
    fpsBoard.moveTo(winWidth - fpsBoard.w(), 0);

    g_sdlDevice->fillRectangle(colorf::BLACK + colorf::A_SHF(200), fpsBoard.dx() - 1, fpsBoard.dy(), fpsBoard.w() + 1, fpsBoard.h());
    g_sdlDevice->drawRectangle(colorf::BLUE  + colorf::A_SHF(255), fpsBoard.dx() - 1, fpsBoard.dy(), fpsBoard.w() + 1, fpsBoard.h());
    fpsBoard.drawRoot();
}

void ProcessRun::checkMagicSpell(const SDL_Event &event)
{
    const char key = SDLDeviceHelper::getKeyChar(event, false);
    if(key == '\0'){
        return;
    }

    const auto magicID = dynamic_cast<SkillBoard *>(m_guiManager.getWidget("SkillBoard"))->getConfig().key2MagicID(key);
    if(!magicID){
        return;
    }

    // always setup the target UID
    // even the magic itself is cooling down

    if(auto uid = getFocusUID(FOCUS_MOUSE)){
        setFocusUID(FOCUS_MAGIC, uid);
    }
    else{
        if(!findUID(getFocusUID(FOCUS_MAGIC))){
            setFocusUID(FOCUS_MAGIC, 0);
        }
    }

    if(getMyHero()->getMagicCoolDownAngle(magicID) < 360){
        addCBLog(CBLOG_ERR, u8"%s尚未冷却", to_cstr(DBCOM_MAGICRECORD(magicID).name));
        return;
    }

    switch(magicID){
        case DBCOM_MAGICID(u8"空拳刀法"):
            {
                getMyHero()->brakeMove();
                if(const auto uid = getFocusUID(FOCUS_MAGIC)){
                    getMyHero()->emplaceAction(ActionSpinKick
                    {
                        .x = getMyHero()->currMotion()->endX,
                        .y = getMyHero()->currMotion()->endY,
                        .aimUID = uid,
                    });
                }
                else{
                    // even there is no target UID
                    // we still cast the magic to show gfx

                    const auto [aimX, aimY] = getMouseGLoc();
                    getMyHero()->emplaceAction(ActionSpinKick
                    {
                        .x = getMyHero()->currMotion()->endX,
                        .y = getMyHero()->currMotion()->endY,
                        .aimX = aimX,
                        .aimY = aimY,
                    });
                }
                break;
            }
        case DBCOM_MAGICID(u8"烈火剑法"):
        case DBCOM_MAGICID(u8"翔空剑法"):
        case DBCOM_MAGICID(u8"莲月剑法"):
        case DBCOM_MAGICID(u8"半月弯刀"):
        case DBCOM_MAGICID(u8"十方斩"):
            {
                getMyHero()->toggleSwingMagic(magicID);
                break;
            }
        case DBCOM_MAGICID(u8"圣言术"):
        case DBCOM_MAGICID(u8"治愈术"):
        case DBCOM_MAGICID(u8"困魔咒"):
        case DBCOM_MAGICID(u8"施毒术"):
        case DBCOM_MAGICID(u8"云寂术"):
        case DBCOM_MAGICID(u8"回生术"):
        case DBCOM_MAGICID(u8"雷电术"):
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"大火球"):
        case DBCOM_MAGICID(u8"冰咆哮"):
        case DBCOM_MAGICID(u8"龙卷风"):
        case DBCOM_MAGICID(u8"霹雳掌"):
        case DBCOM_MAGICID(u8"风掌"):
        case DBCOM_MAGICID(u8"击风"):
        case DBCOM_MAGICID(u8"月魂断玉"):
        case DBCOM_MAGICID(u8"月魂灵波"):
        case DBCOM_MAGICID(u8"斗转星移"):
        case DBCOM_MAGICID(u8"爆裂火焰"):
        case DBCOM_MAGICID(u8"地狱雷光"):
        case DBCOM_MAGICID(u8"怒神霹雳"):
        case DBCOM_MAGICID(u8"灵魂火符"):
        case DBCOM_MAGICID(u8"冰月神掌"):
        case DBCOM_MAGICID(u8"冰月震天"):
        case DBCOM_MAGICID(u8"乾坤大挪移"):
        case DBCOM_MAGICID(u8"群体治愈术"):
        case DBCOM_MAGICID(u8"幽灵盾"):
        case DBCOM_MAGICID(u8"神圣战甲术"):
        case DBCOM_MAGICID(u8"强魔震法"):
        case DBCOM_MAGICID(u8"猛虎强势"):
        case DBCOM_MAGICID(u8"集体隐身术"):
        case DBCOM_MAGICID(u8"移花接玉"):
        case DBCOM_MAGICID(u8"诱惑之光"):
            {
                getMyHero()->brakeMove();
                if(const auto uid = getFocusUID(FOCUS_MAGIC)){
                    getMyHero()->emplaceAction(ActionSpell
                    {
                        .x = getMyHero()->currMotion()->endX,
                        .y = getMyHero()->currMotion()->endY,
                        .aimUID = uid,
                        .extParam
                        {
                            .magicID = magicID,
                        },
                    });
                }
                else{
                    // even there is no target UID
                    // we still cast the magic to show gfx

                    const auto [aimX, aimY] = getMouseGLoc();
                    getMyHero()->emplaceAction(ActionSpell
                    {
                        .x = getMyHero()->currMotion()->endX,
                        .y = getMyHero()->currMotion()->endY,
                        .aimX = aimX,
                        .aimY = aimY,
                        .extParam
                        {
                            .magicID = magicID,
                        },
                    });
                }
                break;
            }
        case DBCOM_MAGICID(u8"火墙"):
        case DBCOM_MAGICID(u8"风震天"):
        case DBCOM_MAGICID(u8"地狱火"):
        case DBCOM_MAGICID(u8"冰沙掌"):
        case DBCOM_MAGICID(u8"魄冰刺"):
        case DBCOM_MAGICID(u8"疾光电影"):
        case DBCOM_MAGICID(u8"焰天火雨"):
        case DBCOM_MAGICID(u8"瞬息移动"):
        case DBCOM_MAGICID(u8"异形换位"):
            {
                const auto [aimX, aimY] = getMouseGLoc();
                getMyHero()->brakeMove();
                getMyHero()->emplaceAction(ActionSpell
                {
                    .x = getMyHero()->currMotion()->endX,
                    .y = getMyHero()->currMotion()->endY,
                    .aimX = aimX,
                    .aimY = aimY,
                    .extParam
                    {
                        .magicID = magicID,
                    }
                });
                break;
            }
        case DBCOM_MAGICID(u8"隐身术"):
        case DBCOM_MAGICID(u8"凝血离魂"):
        case DBCOM_MAGICID(u8"妙影无踪"):
        case DBCOM_MAGICID(u8"魔法盾"):
        case DBCOM_MAGICID(u8"铁布衫"):
        case DBCOM_MAGICID(u8"阴阳法环"):
        case DBCOM_MAGICID(u8"抗拒火环"):
        case DBCOM_MAGICID(u8"破血狂杀"):
        case DBCOM_MAGICID(u8"召唤骷髅"):
        case DBCOM_MAGICID(u8"超强召唤骷髅"):
        case DBCOM_MAGICID(u8"召唤神兽"):
            {
                getMyHero()->brakeMove();
                getMyHero()->emplaceAction(ActionSpell
                {
                    .x = getMyHero()->currMotion()->endX,
                    .y = getMyHero()->currMotion()->endY,
                    .aimUID = getMyHero()->UID(),
                    .extParam
                    {
                        .magicID = magicID,
                    },
                });
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessRun::requestPickUp()
{
    const int x = getMyHero()->currMotion()->endX;
    const int y = getMyHero()->currMotion()->endY;
    if(getGroundItemIDList(x, y).empty()){
        return;
    }

    CMPickUp cmPU;
    std::memset(&cmPU, 0, sizeof(cmPU));

    cmPU.x = x;
    cmPU.y = y;
    cmPU.mapUID = mapUID();
    g_client->send({CM_PICKUP, cmPU});
}

void ProcessRun::requestMagicDamage(int magicID, uint64_t aimUID)
{
    CMRequestMagicDamage cmRMD;
    std::memset(&cmRMD, 0, sizeof(cmRMD));

    cmRMD.magicID = magicID;
    cmRMD.aimUID  = aimUID;

    g_client->send({CM_REQUESTMAGICDAMAGE, cmRMD});
}

void ProcessRun::queryUIDBuff(uint64_t uid) const
{
    switch(uidf::getUIDType(uid)){
        case UID_PLY:
        case UID_MON:
            {
                CMQueryUIDBuff cmQUIDB;
                std::memset(&cmQUIDB, 0, sizeof(cmQUIDB));

                cmQUIDB.uid = uid;
                g_client->send({CM_QUERYUIDBUFF, cmQUIDB});
                break;
            }
        default:
            {
                throw fflerror("invalid uid: %llu, type: %s", to_llu(uid), uidf::getUIDTypeCStr(uid));
            }
    }
}

void ProcessRun::queryMapBaseUID(uint32_t mapID, std::function<void(uint64_t)> op) const
{
    fflassert(mapID);

    CMQueryMapBaseUID cmQMBUID;
    std::memset(&cmQMBUID, 0, sizeof(cmQMBUID));

    cmQMBUID.mapID = mapID;
    g_client->send({CM_QUERYMAPBASEUID, cmQMBUID}, [op = std::move(op)](uint8_t headCode, const uint8_t *buf, size_t bufSize)
    {
        switch(headCode){
            case SM_UID:
                {
                    const auto smUID = ServerMsg::conv<SMUID>(buf, bufSize);
                    if(op){
                        op(smUID.uid);
                    }
                    break;
                }
            default:
                {
                    if(op){
                        op(0);
                    }
                    break;
                }
        }
    });
}

void ProcessRun::queryPlayerWLDesp(uint64_t uid) const
{
    if(uidf::getUIDType(uid) != UID_PLY){
        throw fflerror("invalid uid: %llu, type: %s", to_llu(uid), uidf::getUIDTypeCStr(uid));
    }

    CMQueryPlayerWLDesp cmQPWLD;
    std::memset(&cmQPWLD, 0, sizeof(cmQPWLD));

    cmQPWLD.uid = uid;
    g_client->send({CM_QUERYPLAYERWLDESP, cmQPWLD});
}

void ProcessRun::requestBuy(uint64_t npcUID, uint32_t itemID, uint32_t seqID, size_t count)
{
    fflassert(uidf::getUIDType(npcUID) == UID_NPC);
    fflassert((SDItem
    {
        .itemID = itemID,
        .seqID = seqID,
        .count = 1, // can buy more than SYS_INVGRIDMAXHOLD
    }));

    if(count <= 0){
        throw fflerror("invalid buy count: %zu", count);
    }

    CMBuy cmB;
    std::memset(&cmB, 0, sizeof(cmB));

    cmB.npcUID = npcUID;
    cmB.itemID = itemID;
    cmB. seqID =  seqID;
    cmB. count =  count;
    g_client->send({CM_BUY, cmB});
}


void ProcessRun::requestConsumeItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    CMConsumeItem cmCI;
    std::memset(&cmCI, 0, sizeof(cmCI));

    cmCI.itemID = itemID;
    cmCI.seqID  =  seqID;
    cmCI.count  =  count;
    g_client->send({CM_CONSUMEITEM, cmCI});
}

void ProcessRun::requestMakeItem(uint32_t itemID, size_t count)
{
    CMMakeItem cmMI;
    std::memset(&cmMI, 0, sizeof(cmMI));

    cmMI.itemID = itemID;
    cmMI.count  = count;
    g_client->send({CM_MAKEITEM, cmMI});
}

void ProcessRun::requestEquipWear(uint32_t itemID, uint32_t seqID, int wltype)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("invalid wltype: %d", wltype);
    }

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        throw fflerror("invalid itemID: %llu", to_llu(itemID));
    }

    if(!seqID){
        throw fflerror("invalid seqID: %llu", to_llu(seqID));
    }

    if(!ir.wearable(wltype)){
        throw fflerror("can't equip %s to wltype %d", to_cstr(ir.name), wltype);
    }

    if(!getMyHero()->canWear(itemID, wltype)){
        return;
    }

    CMRequestEquipWear cmREW;
    std::memset(&cmREW, 0, sizeof(cmREW));

    cmREW.itemID = itemID;
    cmREW.seqID  = seqID;
    cmREW.wltype = wltype;
    g_client->send({CM_REQUESTEQUIPWEAR, cmREW});
}

void ProcessRun::requestGrabWear(int wltype)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("invalid wltype: %d", wltype);
    }

    if(!getMyHero()->getWLItem(wltype)){
        return;
    }

    CMRequestGrabWear cmRGW;
    std::memset(&cmRGW, 0, sizeof(cmRGW));
    cmRGW.wltype = wltype;
    g_client->send({CM_REQUESTGRABWEAR, cmRGW});
}

void ProcessRun::requestEquipBelt(uint32_t itemID, uint32_t seqID, int slot)
{
    if(!(slot >= 0 && slot < 6)){
        throw fflerror("invalid slot: %d", slot);
    }

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        throw fflerror("invalid itemID: %llu", to_llu(itemID));
    }

    if(!seqID){
        throw fflerror("invalid seqID: %llu", to_llu(seqID));
    }

    if(!ir.beltable()){
        throw fflerror("can't equip %s to slot %d", to_cstr(ir.name), slot);
    }

    CMRequestEquipBelt cmREB;
    std::memset(&cmREB, 0, sizeof(cmREB));

    cmREB.itemID = itemID;
    cmREB.seqID = seqID;
    cmREB.slot = slot;
    g_client->send({CM_REQUESTEQUIPBELT, cmREB});
}

void ProcessRun::requestGrabBelt(int slot)
{
    if(!(slot >= 0 && slot < 6)){
        throw fflerror("invalid slot: %d", slot);
    }

    if(!getMyHero()->getBelt(slot)){
        return;
    }

    CMRequestGrabBelt cmRGB;
    std::memset(&cmRGB, 0, sizeof(cmRGB));
    cmRGB.slot = slot;
    g_client->send({CM_REQUESTGRABBELT, cmRGB});
}

void ProcessRun::requestDropItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    fflassert((SDItem
    {
        .itemID = itemID,
        .seqID = seqID,
        .count = count,
    }));

    CMDropItem cmDI;
    std::memset(&cmDI, 0, sizeof(cmDI));

    cmDI.itemID = itemID;
    cmDI.seqID = seqID;
    cmDI.count = count;
    g_client->send({CM_DROPITEM, cmDI});
}

bool ProcessRun::hasGroundItemID(uint32_t itemID, int x, int y) const
{
    for(const auto id: getGroundItemIDList(x, y)){
        if(id == itemID){
            return true;
        }
    }
    return false;
}

bool ProcessRun::addGroundItemID(uint32_t itemID, int x, int y)
{
    if(!m_mir2xMapData.validC(x, y)){
        return false;
    }

    if(!DBCOM_ITEMRECORD(itemID)){
        throw fflerror("invalid itemID: %llu", to_llu(itemID));
    }

    m_groundItemIDList[{x, y}].push_back(itemID);
    return true;
}

bool ProcessRun::removeGroundItemID(uint32_t itemID, int x, int y)
{
    auto p = m_groundItemIDList.find({x, y});
    if(p == m_groundItemIDList.end()){
        return false;
    }

    for(auto i = to_lld(p->second.size()) - 1; i >= 0; --i){
        if(p->second[i] == itemID){
            p->second.erase(p->second.begin() + i);
            return true;
        }
    }

    if(p->second.empty()){
        m_groundItemIDList.erase(p);
    }
    return false;
}

int ProcessRun::getAimDirection(const ActionNode &action, int defDir) const
{
    fflassert(action.aimUID);
    if(const auto coPtr = findUID(action.aimUID)){
        if(const auto box = coPtr->getTargetBox()){ // just ot check if still a good target
            const auto [aimX, aimY] = coPtr->location();
            const auto dir = pathf::getDir8(aimX - action.x, aimY - action.y);
            if(dir >= 0 && pathf::dirValid(dir + DIR_BEGIN)){
                return dir + DIR_BEGIN;
            }
        }
    }
    return defDir;
}

std::shared_ptr<SDLSoundEffectChannel> ProcessRun::playSoundEffectAt(uint32_t seffID, int locGX, int locGY, size_t repeats) const
{
    if(seffID == SYS_U32NIL){
        return {};
    }

    fflassert(getMyHero());
    const auto [heroGX, heroGY] = getMyHero()->location();

    const auto dGX = locGX - heroGX;
    const auto dGY = locGY - heroGY;

    const auto [distance, angle] = [dGX, dGY]() -> std::tuple<long, long>
    {
        if(dGX == 0 && dGY == 0){
            return {0, 0};
        }

        return
        {
            std::lround(mathf::LDistance<double>(dGX, dGY, 0, 0)),
            std::lround(90.0 - 180.0 * std::atan2(-dGY, dGX) / mathf::pi),
        };
    }();

    return g_sdlDevice->playSoundEffect(g_seffDB->retrieve(seffID), distance, angle, repeats);
}

void ProcessRun::setCursor(int cursorState)
{
    switch(m_cursorState = cursorState){
        case CURSOR_NONE:
            {
                SDL_ShowCursor(SDL_DISABLE);
                break;
            }
        case CURSOR_DEFAULT:
            {
                SDL_ShowCursor(SDL_ENABLE);
                break;
            }
        case CURSOR_TEAMFLAG:
            {
                SDL_ShowCursor(SDL_DISABLE);
                break;
            }
        default:
            {
                break;
            }
    }
}
