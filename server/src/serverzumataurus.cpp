#include "pathf.hpp"
#include "servermsg.hpp"
#include "serverargparser.hpp"
#include "serverzumataurus.hpp"

extern ServerArgParser *g_serverArgParser;
corof::awaitable<> ServerZumaTaurus::runAICoro()
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
    while(!m_sdHealth.dead()){
        if(targetUID && !(co_await validTarget(targetUID))){
            m_inViewCOList.erase(targetUID);
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
        }

        if(targetUID){
            setStandMode(true);
            if(co_await inDCCastRange(targetUID, hellFireMR.castRange)){
                co_await attackUID(targetUID, hellFireDC);
            }
            else if((lastFireWallTime.diff_sec() >= fireWallCoolDownTime) && (co_await inDCCastRange(targetUID, fireWallMR.castRange))){
                co_await attackUID(targetUID, fireWallDC);
                lastFireWallTime.reset();
            }
            else{
                co_await trackUID(targetUID, {});
            }
        }

        else if(masterUID()){
            setStandMode(true);
            co_await followMaster();
        }

        co_await asyncIdleWait(1000);
    }
}

corof::awaitable<> ServerZumaTaurus::onAMAttack(const ActorMsgPack &mpk)
{
    if(m_standMode){
        co_await Monster::onAMAttack(mpk);
    }
}

corof::awaitable<bool> ServerZumaTaurus::attackUID(uint64_t targetUID, int dcType)
{
    fflassert(false
            || to_u32(dcType) == DBCOM_MAGICID(u8"祖玛教主_火墙")
            || to_u32(dcType) == DBCOM_MAGICID(u8"祖玛教主_地狱火"));

    if(!canAttack(true)){
        co_return false;
    }

    if(!dcValid(dcType, true)){
        co_return false;
    }

    m_attackLock = true;
    const auto attackLockSg = sgf::guard([this](){ m_attackLock = false; });

    const auto coLocOpt = co_await getCOLocation(targetUID);
    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto &coLoc = coLocOpt.value();
    const auto &mr = DBCOM_MAGICRECORD(dcType);

    if(!pathf::inDCCastRange(mr.castRange, X(), Y(), coLoc.x, coLoc.y)){
        co_return false;
    }

    if(const auto newDir = pathf::getOffDir(X(), Y(), coLoc.x, coLoc.y); pathf::dirValid(newDir)){
        m_direction = newDir;
    }

    if(!canAttack(false)){
        co_return false;
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

    addDelay(550, [dcType, coLoc, this](bool)
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

                        if(mapBin()->groundValid(amCFW.x, amCFW.y)){
                            m_actorPod->post(mapUID(), {AM_CASTFIREWALL, amCFW});
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
                        if(mapBin()->groundValid(pathGX, pathGY)){
                            amSFLD.x = pathGX;
                            amSFLD.y = pathGY;
                            amSFLD.damage = getAttackDamage(dcType, 0);
                            addDelay(10 + mathf::CDistance(X(), Y(), amSFLD.x, amSFLD.y) * 100, [amSFLD, castMapID = mapID(), this](bool)
                            {
                                if(castMapID == mapID()){
                                    m_actorPod->post(mapUID(), {AM_STRIKEFIXEDLOCDAMAGE, amSFLD});
                                    if(g_serverArgParser->sharedConfig().showStrikeGrid){
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
    co_return true;
}
