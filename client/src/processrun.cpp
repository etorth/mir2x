/*
 * =====================================================================================
 *
 *       Filename: processrun.cpp
 *        Created: 08/31/2015 03:43:46
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include <memory>
#include <cstring>
#include "dbcomid.hpp"
#include "monster.hpp"
#include "mathf.hpp"
#include "sysconst.hpp"
#include "mapbindb.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "clientargparser.hpp"
#include "pathfinder.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "clientluamodule.hpp"
#include "clientpathfinder.hpp"
#include "debugboard.hpp"
#include "npcchatboard.hpp"
#include "notifyboard.hpp"
#include "fflerror.hpp"
#include "toll.hpp"
#include "lochashtable.hpp"

extern Log *g_log;
extern Client *g_client;
extern PNGTexDB *g_mapDB;
extern MapBinDB *g_mapBinDB;
extern SDLDevice *g_SDLDevice;
extern PNGTexDB *g_groundItemDB;
extern NotifyBoard *g_notifyBoard;
extern debugBoard *g_debugBoard;
extern ClientArgParser *g_clientArgParser;

ProcessRun::ProcessRun()
    : Process()
    , m_mapID(0)
    , m_myHeroUID(0)
    , m_viewX(0)
    , m_viewY(0)
    , m_mapScrolling(false)
    , m_luaModule(this, OUTPORT_CONTROLBOARD)
    , m_NPCChatBoard(this)
    , m_controlBoard
      {
          g_SDLDevice->WindowH(false) - 133,
          g_SDLDevice->WindowW(false),
          this,
      }
    , m_inventoryBoard(0, 0, this)
    , m_quickAccessBoard
      {
          0,
          std::get<1>(g_SDLDevice->getRendererSize()) - m_controlBoard.h(),
          this,
      }
    , m_mousePixlLoc(0, 0, "", 0, 15, 0, colorf::RGBA(0XFF, 0X00, 0X00, 0X00))
    , m_mouseGridLoc(0, 0, "", 0, 15, 0, colorf::RGBA(0XFF, 0X00, 0X00, 0X00))
    , m_ascendStrList()
{
    m_focusUIDTable.fill(0);
    RegisterUserCommand();

    std::memset(m_aniSaveTick, 0, sizeof(m_aniSaveTick));
    std::memset(m_aniTileFrame, 0, sizeof(m_aniTileFrame));
}

void ProcessRun::scrollMap()
{
    const auto [rendererW, rendererH] = g_SDLDevice->getRendererSize();
    const auto showWindowW = rendererW;
    const auto showWindowH = rendererH - m_controlBoard.h();

    const int nViewX = getMyHero()->x() * SYS_MAPGRIDXP - showWindowW / 2;
    const int nViewY = getMyHero()->y() * SYS_MAPGRIDYP - showWindowH / 2;

    const int nDViewX = nViewX - m_viewX;
    const int nDViewY = nViewY - m_viewY;

    if(m_mapScrolling
            ||  (std::abs(nDViewX) > showWindowW / 6)
            ||  (std::abs(nDViewY) > showWindowH / 6)){

        m_mapScrolling = true;

        m_viewX += (int)(std::lround(std::copysign((std::min<int>)(3, std::abs(nDViewX)), nDViewX)));
        m_viewY += (int)(std::lround(std::copysign((std::min<int>)(2, std::abs(nDViewY)), nDViewY)));

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
    constexpr uint32_t delayTicks[]
    {
        150, 200, 250, 300, 350, 400, 420, 450,
    };

    for(int i = 0; i < 8; ++i){
        m_aniSaveTick[i] += std::lround(fUpdateTime);
        if(m_aniSaveTick[i] > delayTicks[i]){
            for(int frame = 0; frame < 16; ++frame){
                m_aniTileFrame[i][frame]++;
                if(m_aniTileFrame[i][frame] >= frame){
                    m_aniTileFrame[i][frame] = 0;
                }
            }
            m_aniSaveTick[i] = 0;
        }
    }

    scrollMap();
    m_controlBoard.update(fUpdateTime);
    m_NPCChatBoard.update(fUpdateTime);
    m_inventoryBoard.update(fUpdateTime);

    getMyHero()->update(fUpdateTime);
    const int myHeroX = getMyHero()->x();
    const int myHeroY = getMyHero()->y();

    for(auto p = m_creatureList.begin(); p != m_creatureList.end();){
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
            p = m_creatureList.erase(p);
        }
    }

    for(auto p = m_indepMagicList.begin(); p != m_indepMagicList.end();){
        if((*p)->Done()){
            p = m_indepMagicList.erase(p);
        }else{
            (*p)->Update(fUpdateTime);
            ++p;
        }
    }

    for(auto p = m_ascendStrList.begin(); p != m_ascendStrList.end();){
        if((*p)->Ratio() < 1.00){
            (*p)->Update(fUpdateTime);
            ++p;
        }else{
            p = m_ascendStrList.erase(p);
        }
    }

    if(auto p = findUID(m_focusUIDTable[FOCUS_ATTACK])){
        if(p->alive()){
            trackAttack(false, m_focusUIDTable[FOCUS_ATTACK]);
        }else{
            m_focusUIDTable[FOCUS_ATTACK] = 0;
        }
    }else{
        m_focusUIDTable[FOCUS_ATTACK] = 0;
    }

    if(true){
        centerMyHero();
    }

    m_starRatio += 0.05;
    if(m_starRatio >= 2.50){
        m_starRatio = 0.00;
    }
}

uint64_t ProcessRun::FocusUID(int nFocusType)
{
    if(nFocusType < (int)(m_focusUIDTable.size())){
        switch(nFocusType){
            case FOCUS_NONE:
                {
                    return 0;
                }
            case FOCUS_MOUSE:
                {
                    // use the cached mouse focus first
                    // if can't get it then scan the whole creature list

                    const auto fnCheckFocus = [this](uint64_t uid, int px, int py) -> bool
                    {
                        if(auto creaturePtr = findUID(uid)){
                            if(uid != getMyHeroUID()){
                                if(creaturePtr->canFocus(px, py)){
                                    return true;
                                }
                            }
                        }
                        return false;
                    };
                    
                    int mousePX = -1;
                    int mousePY = -1;
                    SDL_GetMouseState(&mousePX, &mousePY);

                    mousePX += m_viewX;
                    mousePY += m_viewY;

                    if(fnCheckFocus(m_focusUIDTable[FOCUS_MOUSE], mousePX, mousePY)){
                        return m_focusUIDTable[FOCUS_MOUSE];
                    }

                    ClientCreature *focusCreaturePtr = nullptr;
                    for(auto p = m_creatureList.begin();;){
                        auto pnext = std::next(p);
                        if(fnCheckFocus(p->second->UID(), mousePX, mousePY)){
                            if(false
                                    || !focusCreaturePtr
                                    ||  focusCreaturePtr->y() < p->second->y()){
                                // 1. currently we have no candidate yet
                                // 2. we have candidate but it's not at more front location
                                focusCreaturePtr = p->second.get();
                            }
                        }

                        if(pnext == m_creatureList.end()){
                            break;
                        }
                        p = pnext;
                    }

                    m_focusUIDTable[FOCUS_MOUSE] = focusCreaturePtr ? focusCreaturePtr->UID() : 0;
                    return m_focusUIDTable[FOCUS_MOUSE];
                }
            default:
                {
                    return m_focusUIDTable[nFocusType];
                }
        }
    }
    
    return 0;
}

void ProcessRun::draw()
{
    SDLDevice::RenderNewFrame newFrame;

    const int x0 = -SYS_OBJMAXW + (m_viewX - 2 * SYS_MAPGRIDXP) / SYS_MAPGRIDXP;
    const int y0 = -SYS_OBJMAXH + (m_viewY - 2 * SYS_MAPGRIDYP) / SYS_MAPGRIDYP;
    const int x1 = +SYS_OBJMAXW + (m_viewX + 2 * SYS_MAPGRIDXP + g_SDLDevice->WindowW(false)) / SYS_MAPGRIDXP;
    const int y1 = +SYS_OBJMAXH + (m_viewY + 2 * SYS_MAPGRIDYP + g_SDLDevice->WindowH(false)) / SYS_MAPGRIDYP;

    drawTile(x0, y0, x1, y1);

    // ground objects
    for(int y = y0; y <= y1; ++y){
        for(int x = x0; x <= x1; ++x){
            drawGroundObject(x, y, true);
        }
    }

    if(g_clientArgParser->drawMapGrid){
        const int gridX0 = m_viewX / SYS_MAPGRIDXP;
        const int gridY0 = m_viewY / SYS_MAPGRIDYP;

        const int gridX1 = (m_viewX + g_SDLDevice->WindowW(false)) / SYS_MAPGRIDXP;
        const int gridY1 = (m_viewY + g_SDLDevice->WindowH(false)) / SYS_MAPGRIDYP;

        SDLDevice::EnableDrawColor drawColor(colorf::RGBA(0, 255, 0, 128));
        for(int x = gridX0; x <= gridX1; ++x){
            g_SDLDevice->DrawLine(x * SYS_MAPGRIDXP - m_viewX, 0, x * SYS_MAPGRIDXP - m_viewX, g_SDLDevice->WindowH(false));
        }

        for(int y = gridY0; y <= gridY1; ++y){
            g_SDLDevice->DrawLine(0, y * SYS_MAPGRIDYP - m_viewY, g_SDLDevice->WindowW(false), y * SYS_MAPGRIDYP - m_viewY);
        }
    }

    // draw dead actors
    // dead actors are shown before all active actors
    for(auto &p: m_creatureList){
        p.second->draw(m_viewX, m_viewY, 0);
    }

    drawGroundItem(x0, y0, x1, y1);

    const auto locHashTable = [this]()
    {
        LocHashTable<std::vector<ClientCreature *>> table;
        for(auto &p: m_creatureList){
            table[p.second->location()].push_back(p.second.get());
        }
        return table;
    }();

    // over ground objects
    for(int y = y0; y <= y1; ++y){
        for(int x = x0; x <= x1; ++x){
            drawGroundObject(x, y, false);
        }

        for(int x = x0; x <= x1; ++x){
            if(auto p = locHashTable.find({x, y}); p != locHashTable.end()){
                for(auto creaturePtr: p->second){
                    if(!(creaturePtr && creaturePtr->location() == std::make_tuple(x, y))){
                        throw fflerror("invalid creature location table");
                    }

                    if(!creaturePtr->alive()){
                        continue;
                    }

                    int focusMask = 0;
                    for(auto focusType = 0; focusType < FOCUS_MAX; ++focusType){
                        if(FocusUID(focusType) == creaturePtr->UID()){
                            focusMask |= (1 << focusType);
                        }
                    }
                    creaturePtr->draw(m_viewX, m_viewY, focusMask);
                }

                if(g_clientArgParser->drawCreatureCover){
                    SDLDevice::EnableDrawColor enableColor(colorf::RGBA(0, 0, 255, 128));
                    SDLDevice::EnableDrawBlendMode enableBlendMode(SDL_BLENDMODE_BLEND);
                    g_SDLDevice->fillRectangle(x * SYS_MAPGRIDXP - m_viewX, y * SYS_MAPGRIDYP - m_viewY, SYS_MAPGRIDXP, SYS_MAPGRIDYP);
                }
            }
        }
    }

    // draw all rotating stars
    // notify players that there is somethig to check
    drawRotateStar(x0, y0, x1, y1);
    
    // draw magics
    for(auto p: m_indepMagicList){
        if(!p->Done()){
            p->Draw(m_viewX, m_viewY);
        }
    }

    // draw underlay at the bottom
    // there is one pixel transparent rectangle
    {
        const auto [winW, winH] = g_SDLDevice->getRendererSize();
        SDLDevice::EnableDrawColor color(colorf::RGBA(0, 0, 0, 0));
        g_SDLDevice->fillRectangle(0, winH - 4, winW, 4);
    }

    for(auto p: m_ascendStrList){
        p->Draw(m_viewX, m_viewY);
    }

    m_controlBoard    .draw();
    m_inventoryBoard  .draw();
    m_NPCChatBoard    .draw();
    m_quickAccessBoard.draw();

    // draw notifyBoard
    {
        const int w = g_notifyBoard->w();
        const int h = g_notifyBoard->h();
        g_notifyBoard->drawEx(0, 0, 0, 0, w, h);
    }

    // draw debugBoard
    {
        const int w = std::max<int>(g_debugBoard->pw() + 10, 160);
        const int h = g_debugBoard->h();
        const int x = 0;
        const int y = std::get<1>(g_SDLDevice->getRendererSize()) - h - 133;

        {
            SDLDevice::EnableDrawColor enableColor(colorf::GREEN + 200);
            SDLDevice::EnableDrawBlendMode enableBlend(SDL_BLENDMODE_BLEND);
            g_SDLDevice->fillRectangle(x, y, w, h);
        }

        g_debugBoard->drawEx(x, y, 0, 0, w, h);
        {
            SDLDevice::EnableDrawColor enableColor(colorf::BLUE + 100);
            g_SDLDevice->DrawRectangle(x, y, w, h);
        }
    }

    if(g_clientArgParser->drawMouseLocation){
        SDLDevice::EnableDrawColor enableColor(colorf::RGBA(0, 0, 0, 230));
        SDLDevice::EnableDrawBlendMode enableBlendMode(SDL_BLENDMODE_BLEND);
        g_SDLDevice->fillRectangle(0, 0, 200, 60);

        int mouseX = -1;
        int mouseY = -1;
        SDL_GetMouseState(&mouseX, &mouseY);

        m_mousePixlLoc.setText("Pix_Loc: %d, %d", mouseX, mouseY);
        m_mouseGridLoc.setText("Til_Loc: %d, %d", (mouseX + m_viewX) / SYS_MAPGRIDXP, (mouseY + m_viewY) / SYS_MAPGRIDYP);

        m_mouseGridLoc.drawEx(10, 10, 0, 0, m_mouseGridLoc.w(), m_mouseGridLoc.h());
        m_mousePixlLoc.drawEx(10, 30, 0, 0, m_mousePixlLoc.w(), m_mousePixlLoc.h());
    }

    if(g_clientArgParser->drawFPS){
        drawFPS();
    }
}

void ProcessRun::processEvent(const SDL_Event &event)
{
    bool takeEvent = false;

    takeEvent |= m_inventoryBoard  .processEvent(event, !takeEvent);
    takeEvent |= m_controlBoard    .processEvent(event, !takeEvent);
    takeEvent |= m_NPCChatBoard    .processEvent(event, !takeEvent);
    takeEvent |= m_quickAccessBoard.processEvent(event, !takeEvent);

    if(takeEvent){
        return;
    }

    switch(event.type){
        case SDL_WINDOWEVENT:
            {
                switch(event.window.event){
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            const auto [w, h] = g_SDLDevice->getRendererSize();
                            m_controlBoard.resizeWidth(w);
                            m_controlBoard.moveTo(0, h - 133);
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                int mouseGridX = -1;
                int mouseGridY = -1;
                ScreenPoint2Grid(event.button.x, event.button.y, &mouseGridX, &mouseGridY);

                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            if(const auto uid = FocusUID(FOCUS_MOUSE)){
                                switch(uidf::getUIDType(uid)){
                                    case UID_MON:
                                        {
                                            m_focusUIDTable[FOCUS_ATTACK] = uid;
                                            trackAttack(true, uid);
                                            break;
                                        }
                                    case UID_NPC:
                                        {
                                            sendNPCEvent(uid, SYS_NPCINIT);
                                        }
                                    default:
                                        {
                                            break;
                                        }
                                }
                            }
                            
                            else if(const auto &itemList = getGroundItemList(mouseGridX, mouseGridY); !itemList.empty()){
                                getMyHero()->emplaceAction(ActionPickUp(mouseGridX, mouseGridY, itemList.back().ID()));
                            }
                            break;
                        }
                    case SDL_BUTTON_RIGHT:
                        {
                            // in mir2ei how human moves
                            // 1. client send motion request to server
                            // 2. client put motion lock to human
                            // 3. server response with "+GOOD" or "+FAIL" to client
                            // 4. if "+GOOD" client will release the motion lock
                            // 5. if "+FAIL" client will use the backup position and direction

                            m_focusUIDTable[FOCUS_ATTACK] = 0;
                            m_focusUIDTable[FOCUS_FOLLOW] = 0;

                            if(auto nUID = FocusUID(FOCUS_MOUSE)){
                                m_focusUIDTable[FOCUS_FOLLOW] = nUID;
                            }else{
                                int nX = -1;
                                int nY = -1;
                                if(true
                                        && ScreenPoint2Grid(event.button.x, event.button.y, &nX, &nY)
                                        && mathf::LDistance2(getMyHero()->currMotion().endX, getMyHero()->currMotion().endY, nX, nY)){

                                    // we get a valid dst to go
                                    // provide myHero with new move action command

                                    // when post move action don't use X() and Y()
                                    // since if clicks during hero moving then X() may not equal to EndX

                                    getMyHero()->emplaceAction(ActionMove
                                    {
                                        getMyHero()->currMotion().endX,    // don't use X()
                                        getMyHero()->currMotion().endY,    // don't use Y()
                                        nX,
                                        nY,
                                        SYS_DEFSPEED,
                                        getMyHero()->OnHorse() ? 1 : 0
                                    });
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
                    case SDLK_e:
                        {
                            std::exit(0);
                            break;
                        }
                    case SDLK_ESCAPE:
                        {
                            centerMyHero();
                            break;
                        }
                    case SDLK_t:
                        {
                            if(auto nMouseFocusUID = FocusUID(FOCUS_MOUSE)){
                                m_focusUIDTable[FOCUS_MAGIC] = nMouseFocusUID;
                            }else{
                                if(!findUID(m_focusUIDTable[FOCUS_MAGIC])){
                                    m_focusUIDTable[FOCUS_MAGIC] = 0;
                                }
                            }

                            if(auto nFocusUID = FocusUID(FOCUS_MAGIC)){
                                getMyHero()->emplaceAction(ActionSpell
                                {
                                    getMyHero()->currMotion().endX,
                                    getMyHero()->currMotion().endY,
                                    nFocusUID,
                                    DBCOM_MAGICID(u8"雷电术"),
                                });
                            }else{
                                int nAimX   = -1;
                                int nAimY   = -1;
                                int nMouseX = -1;
                                int nMouseY = -1;
                                SDL_GetMouseState(&nMouseX, &nMouseY);
                                ScreenPoint2Grid(nMouseX, nMouseY, &nAimX, &nAimY);
                                getMyHero()->emplaceAction(ActionSpell
                                {
                                    getMyHero()->currMotion().endX,
                                    getMyHero()->currMotion().endY,
                                    nAimX,
                                    nAimY,
                                    DBCOM_MAGICID(u8"雷电术"),
                                });
                            }
                            break;
                        }
                    case SDLK_p:
                        {
                            getMyHero()->PickUp();
                            break;
                        }
                    case SDLK_y:
                        {
                            getMyHero()->emplaceAction(ActionSpell(getMyHero()->x(), getMyHero()->y(), getMyHero()->UID(), DBCOM_MAGICID(u8"魔法盾")));
                            break;
                        }
                    case SDLK_u:
                        {
                            getMyHero()->emplaceAction(ActionSpell(getMyHero()->x(), getMyHero()->y(), getMyHero()->UID(), DBCOM_MAGICID(u8"召唤骷髅")));
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case SDL_TEXTEDITING:
            {
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessRun::loadMap(uint32_t mapID)
{
    if(!mapID){
        throw fflerror("mapID is zero");
    }

    const auto mapBinPtr = g_mapBinDB->Retrieve(mapID);
    if(!mapBinPtr){
        throw fflerror("can't find map: mapID = %llu", toLLU(mapID));
    }

    m_mapID = mapID;
    m_mir2xMapData = *mapBinPtr;
    m_groundItemList.clear();
}

bool ProcessRun::CanMove(bool bCheckGround, int nCheckCreature, int nX, int nY)
{
    switch(auto nGrid = CheckPathGrid(nX, nY)){
        case PathFind::FREE:
            {
                return true;
            }
        case PathFind::OBSTACLE:
        case PathFind::INVALID:
            {
                return bCheckGround ? false : true;
            }
        case PathFind::OCCUPIED:
        case PathFind::LOCKED:
            {
                switch(nCheckCreature){
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
                            throw fflerror("invalid CheckCreature provided: %d, should be (0, 1, 2)", nCheckCreature);
                        }
                }
            }
        default:
            {
                throw fflerror("invalid grid provided: %d at (%d, %d)", nGrid, nX, nY);
            }
    }
}

int ProcessRun::CheckPathGrid(int nX, int nY) const
{
    if(!m_mir2xMapData.ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_mir2xMapData.Cell(nX, nY).CanThrough()){
        return PathFind::OBSTACLE;
    }

    // we should take EndX/EndY, not X()/Y() as occupied
    // because server only checks EndX/EndY, if we use X()/Y() to request move it just fails

    bool bLocked = false;
    for(auto &p: m_creatureList){
        if(true
                && (p.second)
                && (p.second->currMotion().endX == nX)
                && (p.second->currMotion().endY == nY)){
            return PathFind::OCCUPIED;
        }

        if(!bLocked
                && p.second->x() == nX
                && p.second->y() == nY){
            bLocked = true;
        }
    }
    return bLocked ? PathFind::LOCKED : PathFind::FREE;
}

bool ProcessRun::CanMove(bool bCheckGround, int nCheckCreature, int nX0, int nY0, int nX1, int nY1)
{
    return OneStepCost(nullptr, bCheckGround, nCheckCreature, nX0, nY0, nX1, nY1) >= 0.00;
}

double ProcessRun::OneStepCost(const ClientPathFinder *pFinder, bool bCheckGround, int nCheckCreature, int nX0, int nY0, int nX1, int nY1) const
{
    switch(nCheckCreature){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid CheckCreature provided: %d, should be (0, 1, 2)", nCheckCreature);
            }
    }

    int nMaxIndex = -1;
    switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
        case 0:
            {
                nMaxIndex = 0;
                break;
            }
        case 1:
        case 2:
            {
                nMaxIndex = 1;
                break;
            }
        case 4:
        case 8:
            {
                nMaxIndex = 2;
                break;
            }
        case  9:
        case 18:
            {
                nMaxIndex = 3;
                break;
            }
        default:
            {
                return -1.00;
            }
    }

    int nDX = (nX1 > nX0) - (nX1 < nX0);
    int nDY = (nY1 > nY0) - (nY1 < nY0);

    double fExtraPen = 0.00;
    for(int nIndex = 0; nIndex <= nMaxIndex; ++nIndex){
        int nCurrX = nX0 + nDX * nIndex;
        int nCurrY = nY0 + nDY * nIndex;
        switch(auto nGrid = pFinder ? pFinder->GetGrid(nCurrX, nCurrY) : this->CheckPathGrid(nCurrX, nCurrY)){
            case PathFind::FREE:
                {
                    break;
                }
            case PathFind::LOCKED:
            case PathFind::OCCUPIED:
                {
                    switch(nCheckCreature){
                        case 1:
                            {
                                fExtraPen += 100.00;
                                break;
                            }
                        case 2:
                            {
                                return -1.00;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    break;
                }
            case PathFind::INVALID:
            case PathFind::OBSTACLE:
                {
                    if(bCheckGround){
                        return -1.00;
                    }

                    fExtraPen += 10000.00;
                    break;
                }
            default:
                {
                    throw fflerror("invalid grid provided: %d at (%d, %d)", nGrid, nCurrX, nCurrY);
                }
        }
    }

    return 1.00 + nMaxIndex * 0.10 + fExtraPen;
}

bool ProcessRun::ScreenPoint2Grid(int nPX, int nPY, int *pX, int *pY)
{
    if(pX){ *pX = (nPX + m_viewX) / SYS_MAPGRIDXP; }
    if(pY){ *pY = (nPY + m_viewY) / SYS_MAPGRIDYP; }

    return true;
}

bool ProcessRun::LuaCommand(const char *szLuaCommand)
{
    if(szLuaCommand){
        auto stCallResult = m_luaModule.GetLuaState().script(szLuaCommand, [](lua_State *, sol::protected_function_result stResult){
            // default handler
            // do nothing and let the call site handle the errors
            return stResult;
        });

        if(stCallResult.valid()){
            // default nothing printed
            // we can put information here to show call succeeds
            // or we can unlock the input widget to allow next command
        }else{
            sol::error stError = stCallResult;
            g_log->addLog(LOGTYPE_WARNING, "%s", stError.what());
        }

        // always return true if command get evaluated
        // return false only if provided is unhandled
        return true;
    }
    return false;
}

bool ProcessRun::UserCommand(const char *szUserCommand)
{
    if(szUserCommand){
        // 1. split commands into an array
        std::vector<std::string> stvToken;
        {
            auto pBegin = szUserCommand;
            auto pEnd   = szUserCommand + std::strlen(szUserCommand);

            while(true){
                pBegin = std::find_if_not(pBegin, pEnd, [](char chByte){ return chByte == ' '; });
                if(pBegin != pEnd){
                    auto pDone = std::find(pBegin, pEnd, ' ');
                    stvToken.emplace_back(pBegin, pDone);
                    pBegin = pDone;
                }else{ break; }
            }

            if(stvToken.empty()){
                return true;
            }
        }

        // 2. to find the command
        //    alert and return if can't find it
        UserCommandEntry *pEntry = nullptr;
        {
            int nCount = 0;
            for(auto &rstEntry : m_userCommandGroup){
                if(rstEntry.Command.substr(0, stvToken[0].size()) == stvToken[0]){
                    pEntry = &rstEntry;
                    nCount++;
                }
            }

            switch(nCount){
                case 0:
                    {
                        AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "Invalid command: %s", stvToken[0].c_str());
                        break;
                    }
                case 1:
                    {
                        if(pEntry->Callback){
                            pEntry->Callback(stvToken);
                        }
                        break;
                    }
                default:
                    {
                        AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "Ambiguous command: %s", stvToken[0].c_str());
                        for(auto &rstEntry : m_userCommandGroup){
                            if(rstEntry.Command.substr(0, stvToken[0].size()) == stvToken[0]){
                                AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "Possible command: %s", rstEntry.Command.c_str());
                            }
                        }
                        break;
                    }
            }
        }
    }
    return false;
}

std::vector<int> ProcessRun::GetPlayerList()
{
    std::vector<int> result;
    for(auto p = m_creatureList.begin(); p != m_creatureList.end();){
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
            p = m_creatureList.erase(p);
        }
    }
    return result;
}

bool ProcessRun::RegisterUserCommand()
{
    // 1. register command: moveTo
    //    usage:
    //              moveTo        120 250
    //              moveTo 某地图 120 250
    auto fnMoveTo = [this](const std::vector<std::string> &rstvParam) -> int
    {
        switch(rstvParam.size()){
            case 1 + 2:
                {
                    int nX = std::atoi(rstvParam[1].c_str());
                    int nY = std::atoi(rstvParam[2].c_str());
                    RequestSpaceMove(MapID(), nX, nY);
                    return 0;
                }
            case 1 + 3:
                {
                    int nX = std::atoi(rstvParam[2].c_str());
                    int nY = std::atoi(rstvParam[3].c_str());
                    auto nMapID = DBCOM_MAPID(rstvParam[1].c_str());
                    RequestSpaceMove(nMapID, nX, nY);
                    return 0;
                }
            default:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "Invalid argument to command: moveTo");
                    return 1;
                }
        }
    };
    m_userCommandGroup.emplace_back("moveTo", fnMoveTo);

    auto fnLuaEditor = [this](const std::vector<std::string> &) -> int
    {
        AddOPLog(OUTPORT_CONTROLBOARD, 1, "", "LuaEditor not implemented yet");
        return 0;
    };
    m_userCommandGroup.emplace_back("luaEditor", fnLuaEditor);

    auto fnMakeItem = [this](const std::vector<std::string> &rstvParam) -> int
    {
        switch(rstvParam.size()){
            case 0 + 1:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"@make 物品名字");
                    return 1;
                }
            case 1 + 1:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"获得%s", rstvParam[1].c_str());
                    return 0;
                }
            default:
                {
                    AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"Invalid argument for @make");
                    return 1;
                }
        }
    };
    m_userCommandGroup.emplace_back("makeItem", fnMakeItem);

    auto fnGetAttackUID = [this](const std::vector<std::string> &) -> int
    {
        AddOPLog(OUTPORT_CONTROLBOARD, 2, "", std::to_string(FocusUID(FOCUS_ATTACK)).c_str());
        return 0;
    };
    m_userCommandGroup.emplace_back("getAttackUID", fnGetAttackUID);

    auto fnKillPets = [this](const std::vector<std::string> &) -> int
    {
        RequestKillPets();
        AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"杀死所有宝宝");
        return 0;
    };
    m_userCommandGroup.emplace_back("killPets", fnKillPets);

    return true;
}

bool ProcessRun::RegisterLuaExport(ClientLuaModule *pModule, int nOutPort)
{
    if(pModule){

        // initialization before registration
        pModule->GetLuaState().script(R"()");

        // register command printLine
        // print one line to the out port allocated for the lua module
        pModule->GetLuaState().set_function("printLine", [this, nOutPort](sol::object stLogType, sol::object stPrompt, sol::object stLogInfo)
        {
            if(true
                    && stLogType.is<int>()
                    &&  stPrompt.is<std::string>()
                    && stLogInfo.is<std::string>()){
                AddOPLog(nOutPort, stLogType.as<int>(), stPrompt.as<std::string>().c_str(), "%s", stLogInfo.as<std::string>().c_str());
                return;
            }

            // invalid argument provided
            AddOPLog(nOutPort, 2, ">>> ", "%s", "printLine(LogType: int, Prompt: string, LogInfo: string)");
        });

        // register command addLog
        // add to client system log file only, same as g_log->AddLog(LOGTYPE_XXXX, LogInfo)
        pModule->GetLuaState().set_function("addLog", [this, nOutPort](sol::object stLogType, sol::object stLogInfo)
        {
            if(true
                    && stLogType.is<int>()
                    && stLogInfo.is<std::string>()){

                switch(stLogType.as<int>()){
                    case 0  : g_log->addLog(LOGTYPE_INFO   , "%s", stLogInfo.as<std::string>().c_str()); return;
                    case 1  : g_log->addLog(LOGTYPE_WARNING, "%s", stLogInfo.as<std::string>().c_str()); return;
                    case 2  : g_log->addLog(LOGTYPE_FATAL  , "%s", stLogInfo.as<std::string>().c_str()); return;
                    default : g_log->addLog(LOGTYPE_DEBUG  , "%s", stLogInfo.as<std::string>().c_str()); return;
                }
            }

            // invalid argument provided
            AddOPLog(nOutPort, 2, ">>> ", "%s", "addLog(LogType: int, LogInfo: string)");
        });

        // register command playerList
        // return a table (userData) to lua for ipairs() check
        pModule->GetLuaState().set_function("playerList", [this](sol::this_state stThisLua)
        {
            return sol::make_object(sol::state_view(stThisLua), GetPlayerList());
        });

        // register command moveTo(x, y)
        // wait for server to move player if possible
        pModule->GetLuaState().set_function("moveTo", [this, nOutPort](sol::object stLocX, sol::object stLocY)
        {
            if(true
                    && stLocX.is<int>()
                    && stLocY.is<int>()){

                int nX = stLocX.as<int>();
                int nY = stLocY.as<int>();

                RequestSpaceMove(MapID(), nX, nY);
                AddOPLog(nOutPort, 0, ">", "moveTo(%d, %d)", nX, nY);
            }
        });


        // register command ``listPlayerInfo"
        // this command call to get a player info table and print to out port
        pModule->GetLuaState().script(R"#(
            function listPlayerInfo ()
                for k, v in ipairs(playerList())
                do
                    printLine(0, "> ", tostring(v))
                end
            end
        )#");

        // register command ``help"
        // part-1: divide into two parts, part-1 create the table for help
        pModule->GetLuaState().script(R"#(
            helpInfoTable = {
                wear     = "put on different dress",
                moveTo   = "move to other position on current map",
                randMove = "random move on current map"
            }
        )#");

        // part-2: make up the function to print the table entry
        pModule->GetLuaState().script(R"#(
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
        pModule->GetLuaState().set_function("myHero_dress", [this](int nDress)
        {
            if(nDress >= 0){
                getMyHero()->Dress((uint32_t)(nDress));
            }
        });

        // register command ``myHero.xxx"
        // I need to insert a table to micmic a instance myHero in the future
        pModule->GetLuaState().set_function("myHero_weapon", [this](int nWeapon)
        {
            if(nWeapon >= 0){
                getMyHero()->Weapon((uint32_t)(nWeapon));
            }
        });


        // registration done
        return true;
    }
    return false;
}

void ProcessRun::AddOPLog(int nOutPort, int logType, const char *szPrompt, const char *szLogFormat, ...)
{
    std::string szLog;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szLogFormat);

        try{
            szLog = str_vprintf(szLogFormat, ap);
        }catch(const std::exception &e){
            bError = true;
            szLog = str_printf("Exception caught in ProcessRun::AddOPLog(\"%s\", ...): %s", szLogFormat, e.what());
        }

        va_end(ap);
    }

    if(bError){
        logType = Log::LOGTYPEV_WARNING;
    }

    if(nOutPort & OUTPORT_LOG){
        switch(logType){
            case Log::LOGTYPEV_INFO    : g_log->addLog(LOGTYPE_INFO   , u8"%s%s", szPrompt ? szPrompt : "", szLog.c_str()); break;
            case Log::LOGTYPEV_WARNING : g_log->addLog(LOGTYPE_WARNING, u8"%s%s", szPrompt ? szPrompt : "", szLog.c_str()); break;
            case Log::LOGTYPEV_DEBUG   : g_log->addLog(LOGTYPE_DEBUG,   u8"%s%s", szPrompt ? szPrompt : "", szLog.c_str()); break;
            default                    : g_log->addLog(LOGTYPE_FATAL  , u8"%s%s", szPrompt ? szPrompt : "", szLog.c_str()); break;
        }
    }

    if(nOutPort & OUTPORT_SCREEN){
    }

    if(nOutPort & OUTPORT_CONTROLBOARD){
        m_controlBoard.addLog(logType, szLog.c_str());
    }
}

void ProcessRun::addCBLog(int logType, const char *format, ...)
{
    std::string logStr;
    bool hasError = false;
    {
        va_list ap;
        va_start(ap, format);

        try{
            logStr = str_vprintf(format, ap);
        }
        catch(const std::exception &e){
            hasError = true;
            logStr = str_printf("Parsing failed in ProcessRun::addCBLog(\"%s\", ...): %s", format, e.what());
        }

        va_end(ap);
    }

    if(hasError){
        logType = CBLOG_ERR;
    }
    m_controlBoard.addLog(logType, logStr.c_str());
}

bool ProcessRun::OnMap(uint32_t nMapID, int nX, int nY) const
{
    return (MapID() == nMapID) && m_mir2xMapData.ValidC(nX, nY);
}

ClientCreature *ProcessRun::findUID(uint64_t uid, bool checkVisible) const
{
    if(!uid){
        return nullptr;
    }

    // TODO don't remove invisible creatures, this causes too much crash
    // let ProcessRun::update() loop clean it

    if(auto p = m_creatureList.find(uid); p != m_creatureList.end()){
        if(p->second->UID() != uid){
            throw fflerror("invalid creature: %p, UID = %llu", p->second.get(), toLLU(p->second->UID()));
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
        if(bForce || getMyHero()->StayIdle()){
            auto nEndX = getMyHero()->currMotion().endX;
            auto nEndY = getMyHero()->currMotion().endY;
            return getMyHero()->emplaceAction(ActionAttack(nEndX, nEndY, DC_PHY_PLAIN, SYS_DEFSPEED, nUID));
        }
    }
    return false;
}

uint32_t ProcessRun::GetFocusFaceKey()
{
    uint32_t nFaceKey = 0X02000000;
    if(auto nUID = FocusUID(FOCUS_MOUSE)){
        if(auto pCreature = findUID(nUID)){
            switch(pCreature->type()){
                case UID_PLY:
                    {
                        nFaceKey = 0X02000000;
                        break;
                    }
                case UID_MON:
                    {
                        auto nLookID = ((Monster*)(pCreature))->lookID();
                        if(nLookID >= 0){
                            nFaceKey = 0X01000000 + (nLookID - LID_MIN);
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }

    return nFaceKey;
}

void ProcessRun::addAscendStr(int nType, int nValue, int nX, int nY)
{
    m_ascendStrList.emplace_back(std::make_shared<AscendStr>(nType, nValue, nX, nY));
}

bool ProcessRun::GetUIDLocation(uint64_t nUID, bool bDrawLoc, int *pX, int *pY)
{
    if(auto pCreature = findUID(nUID)){
        if(bDrawLoc){
        }else{
            if(pX){ *pX = pCreature->x(); }
            if(pY){ *pY = pCreature->y(); }
        }
        return true;
    }
    return false;
}

void ProcessRun::centerMyHero()
{
    const auto nMotion     = getMyHero()->currMotion().motion;
    const auto nDirection  = getMyHero()->currMotion().direction;
    const auto nX          = getMyHero()->currMotion().x;
    const auto nY          = getMyHero()->currMotion().y;
    const auto currFrame   = getMyHero()->currMotion().frame;
    const auto frameCount = getMyHero()->motionFrameCount(nMotion, nDirection);

    if(frameCount <= 0){
        throw fflerror("invalid frame count: %d", frameCount);
    }

    const auto fnSetOff = [this, nX, nY, nDirection, currFrame, frameCount](int stepLen)
    {
        const auto showWindowW = g_SDLDevice->WindowW(false);
        const auto showWindowH = g_SDLDevice->WindowH(false) - m_controlBoard.h();

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
                    const auto [shiftX, shiftY] = getMyHero()->getShift();
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

bool ProcessRun::RequestSpaceMove(uint32_t nMapID, int nX, int nY)
{
    if(auto pMapBin = g_mapBinDB->Retrieve(nMapID)){
        if(pMapBin->ValidC(nX, nY) && pMapBin->Cell(nX, nY).CanThrough()){
            CMReqestSpaceMove stCMRSM;
            stCMRSM.MapID = nMapID;
            stCMRSM.X     = nX;
            stCMRSM.Y     = nY;
            g_client->send(CM_REQUESTSPACEMOVE, stCMRSM);
            return true;
        }
    }
    return false;
}

void ProcessRun::RequestKillPets()
{
    g_client->send(CM_REQUESTKILLPETS);
}

void ProcessRun::queryCORecord(uint64_t nUID) const
{
    CMQueryCORecord stCMQCOR;
    std::memset(&stCMQCOR, 0, sizeof(stCMQCOR));

    stCMQCOR.AimUID = nUID;
    g_client->send(CM_QUERYCORECORD, stCMQCOR);
}

void ProcessRun::OnActionSpawn(uint64_t nUID, const ActionNode &rstAction)
{
    condcheck(nUID);
    condcheck(rstAction.Action == ACTION_SPAWN);

    if(uidf::getUIDType(nUID) != UID_MON){
        queryCORecord(nUID);
        return;
    }

    switch(uidf::getMonsterID(nUID)){
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                // TODO how about make it as an action of skeleton
                // then we don't need to define the callback of a done magic

                AddOPLog(OUTPORT_CONTROLBOARD, 2, "", u8"使用魔法: 召唤骷髅"), 
                m_indepMagicList.emplace_back(std::make_shared<IndepMagic>
                (
                    rstAction.ActionParam,
                    DBCOM_MAGICID(u8"召唤骷髅"), 
                    0,
                    EGS_START,
                    rstAction.Direction,
                    rstAction.X,
                    rstAction.Y,
                    rstAction.X,
                    rstAction.Y,
                    nUID
                ));

                m_UIDPending.insert(nUID);
                m_indepMagicList.back()->AddFunc([this, nUID, rstAction, pMagic = m_indepMagicList.back()]() -> bool
                {
                    // if(!pMagic->Done()){
                    //     return false;
                    // }

                    if(pMagic->Frame() < 10){
                        return false;
                    }

                    ActionStand stActionStand
                    {
                        rstAction.X,
                        rstAction.Y,
                        DIR_DOWNLEFT,
                    };

                    if(auto pMonster = Monster::createMonster(nUID, this, stActionStand)){
                        m_creatureList[nUID].reset(pMonster);
                    }

                    m_UIDPending.erase(nUID);
                    queryCORecord(nUID);
                    return true;
                });

                return;
            }
        default:
            {
                ActionStand stActionStand
                {
                    rstAction.X,
                    rstAction.Y,
                    PathFind::ValidDir(rstAction.Direction) ? rstAction.Direction : DIR_UP,
                };

                if(auto pMonster = Monster::createMonster(nUID, this, stActionStand)){
                    m_creatureList[nUID].reset(pMonster);
                }

                queryCORecord(nUID);
                return;
            }
    }
}

Widget *ProcessRun::getWidget(const std::string &widgetName)
{
    if(widgetName == "InventoryBoard"){
        return &m_inventoryBoard;
    }

    if(widgetName == "QuickAccessBoard"){
        return &m_quickAccessBoard;
    }
    return nullptr;
}

void ProcessRun::sendNPCEvent(uint64_t uid, const char *event, const char *value)
{
    if(uidf::getUIDType(uid) != UID_NPC){
        throw fflerror("sending npc event to a non-npc UID");
    }

    if(!(event && std::strlen(event))){
        throw fflerror("invalid event name: [%p]%s", event, event ? event : "(null)");
    }

    CMNPCEvent cmNPCE;
    std::memset(&cmNPCE, 0, sizeof(cmNPCE));

    cmNPCE.uid = uid;
    if(event){
        std::strcpy(cmNPCE.event, event);
    }

    if(value){
        std::strcpy(cmNPCE.value, value);
    }
    g_client->send(CM_NPCEVENT, cmNPCE);
}

void ProcessRun::drawGroundItem(int x0, int y0, int x1, int y1)
{
    for(const auto &p: m_groundItemList){
        const auto [x, y] = p.first;
        if(!(x >= x0 && x < x1 && y >= y0 && y < y1)){
            continue;
        }

        for(auto &item: p.second){
            const auto &ir = DBCOM_ITEMRECORD(item.ID());
            if(!ir){
                throw fflerror("invalid item record");
            }

            if(ir.PkgGfxID < 0){
                continue;
            }

            auto texPtr = g_groundItemDB->Retrieve(ir.PkgGfxID);
            if(!texPtr){
                continue;
            }

            const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
            const int drawPX = x * SYS_MAPGRIDXP - m_viewX + SYS_MAPGRIDXP / 2 - texW / 2;
            const int drawPY = y * SYS_MAPGRIDYP - m_viewY + SYS_MAPGRIDYP / 2 - texH / 2;

            int mouseX = -1;
            int mouseY = -1;
            SDL_GetMouseState(&mouseX, &mouseY);

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
            g_SDLDevice->DrawTexture(texPtr, drawPX + 1, drawPY - 1);

            // draw item body
            SDL_SetTextureColorMod(texPtr, 255, 255, 255);
            SDL_SetTextureAlphaMod(texPtr, 255);
            g_SDLDevice->DrawTexture(texPtr, drawPX, drawPY);

            if(mouseOver){
                LabelBoard itemName(0, 0, ir.Name, 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0X00));
                const int boardW = itemName.w();
                const int boardH = itemName.h();

                const int drawNameX = x * SYS_MAPGRIDXP - m_viewX + SYS_MAPGRIDXP / 2 - itemName.w() / 2;
                const int drawNameY = y * SYS_MAPGRIDYP - m_viewY + SYS_MAPGRIDYP / 2 - itemName.h() / 2 - 20;
                itemName.drawEx(drawNameX, drawNameY, 0, 0, boardW, boardH);
            }
        }
    }
}

void ProcessRun::drawTile(int x0, int y0, int x1, int y1)
{
    for(int y = y0; y < y1; ++y){
        for(int x = x0; x <= x1; ++x){
            if(m_mir2xMapData.ValidC(x, y) && !(x % 2) && !(y % 2)){
                if(const auto &tile = m_mir2xMapData.Tile(x, y); tile.Valid()){
                    if(auto texPtr = g_mapDB->Retrieve(tile.Image())){
                        g_SDLDevice->DrawTexture(texPtr, x * SYS_MAPGRIDXP - m_viewX, y * SYS_MAPGRIDYP - m_viewY);
                    }
                }
            }
        }
    }
}

void ProcessRun::drawGroundObject(int x, int y, bool ground)
{
    if(!m_mir2xMapData.ValidC(x, y)){
        return;
    }

    for(const int i: {0, 1}){
        const auto objArr = m_mir2xMapData.Cell(x, y).ObjectArray(i);
        if(true
                && ((bool)(objArr[4] & 0X80))
                && ((bool)(objArr[4] & 0X01) == ground)){
            uint32_t imageId = 0
                | (((uint32_t)(objArr[2])) << 16)
                | (((uint32_t)(objArr[1])) <<  8)
                | (((uint32_t)(objArr[0])) <<  0);

            if(objArr[3] & 0X80){
                if(false
                        || objArr[2] == 11
                        || objArr[2] == 26
                        || objArr[2] == 41
                        || objArr[2] == 56
                        || objArr[2] == 71){
                    const int aniType = (objArr[3] & 0B01110000) >> 4;
                    const int aniFrameType = (objArr[3] & 0B00001111);
                    imageId += m_aniTileFrame[aniType][aniFrameType];
                }
            }

            const bool alphaRender = (objArr[4] & 0B00000010);
            if(auto texPtr = g_mapDB->Retrieve(imageId)){
                const int texH = SDLDevice::getTextureHeight(texPtr);
                if(alphaRender){
                    SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_BLEND);
                    SDL_SetTextureAlphaMod(texPtr, 128);
                }
                g_SDLDevice->DrawTexture(texPtr, x * SYS_MAPGRIDXP - m_viewX, (y + 1) * SYS_MAPGRIDYP - m_viewY - texH);
            }
        }
    }
}

void ProcessRun::drawRotateStar(int x0, int y0, int x1, int y1)
{
    if(m_starRatio > 1.0){
        return;
    }

    auto texPtr = g_groundItemDB->Retrieve(0X01000000);
    if(!texPtr){
        return;
    }

    const auto [texW, texH] = SDLDevice::getTextureSize(texPtr);
    const auto currSize = (int)(std::lround(m_starRatio * texW / 2.50));

    for(const auto &p: m_groundItemList){
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
        g_SDLDevice->drawTextureEx(texPtr, 0, 0, texW, texH, drawPX, drawPY, currSize, currSize, currSize / 2, currSize / 2, std::lround(m_starRatio * 360.0));
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

void ProcessRun::drawFPS()
{
    const auto fpsStr = std::to_string(g_SDLDevice->getFPS());
    LabelBoard fpsBoard(0, 0, fpsStr.c_str(), 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0X00));

    const int winWidth = g_SDLDevice->getRendererWidth();
    fpsBoard.moveTo(winWidth - fpsBoard.w(), 0);

    g_SDLDevice->fillRectangle(colorf::GREY + 255, fpsBoard.x() - 1, fpsBoard.y(), fpsBoard.w() + 1, fpsBoard.h());
    g_SDLDevice->DrawRectangle(colorf::BLUE + 255, fpsBoard.x() - 1, fpsBoard.y(), fpsBoard.w() + 1, fpsBoard.h());
    fpsBoard.draw();
}
