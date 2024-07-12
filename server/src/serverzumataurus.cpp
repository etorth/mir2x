#include "pathf.hpp"
#include "servermsg.hpp"
#include "serverargparser.hpp"
#include "serverzumataurus.hpp"

extern ServerArgParser *g_serverArgParser;
corof::eval_poller ServerZumaTaurus::updateCoroFunc()
{
    const auto hellFireDC = DBCOM_MAGICID(u8"祖玛教主_地狱火");
    const auto fireWallDC = DBCOM_MAGICID(u8"祖玛教主_火墙");

    fflassert(hellFireDC);
    fflassert(fireWallDC);

    const auto &hellFireMR = DBCOM_MAGICRECORD(hellFireDC);
    const auto &fireWallMR = DBCOM_MAGICRECORD(fireWallDC);

    fflassert(hellFireMR);
    fflassert(fireWallMR);

    hres_timer lastFireWallTime;
    constexpr uint64_t fireWallCoolDownTime = 5;

    uint64_t targetUID = 0;
    while(m_sdHealth.hp > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
            m_inViewCOList.erase(targetUID);
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            setStandMode(true);
            if(co_await coro_inDCCastRange(targetUID, hellFireMR.castRange)){
                co_await coro_attackUID(targetUID, hellFireDC);
            }
            else if((lastFireWallTime.diff_sec() >= fireWallCoolDownTime) && (co_await coro_inDCCastRange(targetUID, fireWallMR.castRange))){
                co_await coro_attackUID(targetUID, fireWallDC);
                lastFireWallTime.reset();
            }
            else{
                co_await coro_trackUID(targetUID, {});
            }
        }

        else if(masterUID()){
            setStandMode(true);
            co_await coro_followMaster();
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

void ServerZumaTaurus::onAMAttack(const ActorMsgPack &mpk)
{
    if(m_standMode){
        Monster::onAMAttack(mpk);
    }
}

void ServerZumaTaurus::attackUID(uint64_t targetUID, int dcType, std::function<void()> onOK, std::function<void()> onError)
{
    fflassert(false
            || to_u32(dcType) == DBCOM_MAGICID(u8"祖玛教主_火墙")
            || to_u32(dcType) == DBCOM_MAGICID(u8"祖玛教主_地狱火"));

    if(!canAttack()){
        if(onError){
            onError();
        }
        return;
    }

    if(!dcValid(dcType, true)){
        if(onError){
            onError();
        }
        return;
    }

    m_attackLock = true;
    getCOLocation(targetUID, [this, dcType, targetUID, onOK, onError](const COLocation &coLoc)
    {
        fflassert(m_attackLock);
        m_attackLock = false;

        const auto &mr = DBCOM_MAGICRECORD(dcType);
        if(!pathf::inDCCastRange(mr.castRange, X(), Y(), coLoc.x, coLoc.y)){
            if(onError){
                onError();
            }
            return;
        }

        if(const auto newDir = pathf::getOffDir(X(), Y(), coLoc.x, coLoc.y); pathf::dirValid(newDir)){
            m_direction = newDir;
        }

        if(!canAttack()){
            if(onError){
                onError();
            }
            return;
        }

        dispatchAction(ActionAttack
        {
            .speed = attackSpeed(),
            .x = X(),
            .y = Y(),
            .aimUID = targetUID,
            .extParam
            {
                .magicID = to_u32(dcType),
            },
        });

        addDelay(550, [dcType, coLoc, this]()
        {
            switch(dcType){
                case DBCOM_MAGICID(u8"祖玛教主_火墙"):
                    {
                        AMCastFireWall amCFW;
                        std::memset(&amCFW, 0, sizeof(amCFW));

                        amCFW.minDC = 5;
                        amCFW.maxDC = 9;

                        amCFW.duration = 5 * 1000;
                        amCFW.dps      = 3;

                        for(const int dir: {DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT}){
                            if(dir == DIR_NONE){
                                amCFW.x = coLoc.x;
                                amCFW.y = coLoc.y;
                            }
                            else{
                                std::tie(amCFW.x, amCFW.y) = pathf::getFrontGLoc(coLoc.x, coLoc.y, dir, 1);
                            }

                            if(m_map->groundValid(amCFW.x, amCFW.y)){
                                m_actorPod->forward(m_map->UID(), {AM_CASTFIREWALL, amCFW});
                            }
                        }
                        break;
                    }
                case DBCOM_MAGICID(u8"祖玛教主_地狱火"):
                    {
                        if(const auto dirIndex = pathf::getDir8(coLoc.x - X(), coLoc.y - Y()); (dirIndex >= 0) && pathf::dirValid(dirIndex + DIR_BEGIN)){
                            m_direction = dirIndex + DIR_BEGIN;
                        }

                        std::set<std::tuple<int, int>> pathGridList;
                        switch(Direction()){
                            case DIR_UP:
                            case DIR_DOWN:
                            case DIR_LEFT:
                            case DIR_RIGHT:
                                {
                                    for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                        const auto [pathGX, pathGY] = pathf::getFrontGLoc(X(), Y(), Direction(), distance);
                                        pathGridList.insert({pathGX, pathGY});

                                        if(distance > 3){
                                            const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, Direction(), 1);
                                            pathGridList.insert({pathGX + sgnDY, pathGY + sgnDX}); // switch sgnDX and sgnDY and plus/minus
                                            pathGridList.insert({pathGX - sgnDY, pathGY - sgnDX});
                                        }
                                    }
                                    break;
                                }
                            case DIR_UPLEFT:
                            case DIR_UPRIGHT:
                            case DIR_DOWNLEFT:
                            case DIR_DOWNRIGHT:
                                {
                                    for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                        const auto [pathGX, pathGY] = pathf::getFrontGLoc(X(), Y(), Direction(), distance);
                                        pathGridList.insert({pathGX, pathGY});

                                        const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, Direction(), 1);
                                        pathGridList.insert({pathGX + sgnDX, pathGY        });
                                        pathGridList.insert({pathGX        , pathGY + sgnDY});
                                    }
                                    break;
                                }
                            default:
                                {
                                    throw fflreach();
                                }
                        }

                        AMStrikeFixedLocDamage amSFLD;
                        std::memset(&amSFLD, 0, sizeof(amSFLD));

                        for(const auto &[pathGX, pathGY]: pathGridList){
                            if(m_map->groundValid(pathGX, pathGY)){
                                amSFLD.x = pathGX;
                                amSFLD.y = pathGY;
                                amSFLD.damage = getAttackDamage(dcType, 0);
                                addDelay(10 + mathf::CDistance(X(), Y(), amSFLD.x, amSFLD.y) * 100, [amSFLD, castMapID = mapID(), this]()
                                {
                                    if(castMapID == mapID()){
                                        m_actorPod->forward(m_map->UID(), {AM_STRIKEFIXEDLOCDAMAGE, amSFLD});
                                        if(g_serverArgParser->showStrikeGrid){
                                            SMStrikeGrid smSG;
                                            std::memset(&smSG, 0, sizeof(smSG));

                                            smSG.x = amSFLD.x;
                                            smSG.y = amSFLD.y;
                                            dispatchInViewCONetPackage(SM_STRIKEGRID, smSG);
                                        }
                                    }
                                });
                            }
                        }
                        break;
                    }
                default:
                    {
                        throw fflreach();
                    }
            }
        });

        if(onOK){
            onOK();
        }
    },

    [this, targetUID, onError]()
    {
        m_attackLock = false;
        m_inViewCOList.erase(targetUID);

        if(onError){
            onError();
        }
    });
}
