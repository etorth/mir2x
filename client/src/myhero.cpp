/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
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

#include <algorithm>
#include "log.hpp"
#include "client.hpp"
#include "myhero.hpp"
#include "message.hpp"
#include "fflerror.hpp"
#include "mathf.hpp"
#include "clientargparser.hpp"
#include "processrun.hpp"
#include "clientpathfinder.hpp"

extern Log *g_log;
extern Client *g_client;
extern ClientArgParser *g_clientArgParser;

MyHero::MyHero(uint64_t nUID, ProcessRun *pRun, const ActionNode &action)
	: Hero(nUID, pRun, action)
{}

bool MyHero::moveNextMotion()
{
    if(!m_forceMotionQueue.empty()){
        m_currMotion = std::move(m_forceMotionQueue.front());
        m_forceMotionQueue.pop_front();
        return true;
    }

    if(m_motionQueue.empty()){
        if(m_actionQueue.empty()){
            m_currMotion = makeIdleMotion();
            return true;
        }
        else{
            // there is pending action in the queue
            // try to present it and return false if failed
            return parseActionQueue();
        }
    }

    if(motionQueueValid()){
        m_currMotion = std::move(m_motionQueue.front());
        m_motionQueue.pop_front();
        return true;
    }

    // oops we get invalid motion queue
    g_log->addLog(LOGTYPE_INFO, "Invalid motion queue:");

    m_currMotion->print();
    for(auto &m: m_motionQueue){
        m->print();
    }
    throw fflerror("Current motion is not valid");
}

bool MyHero::decompMove(bool bCheckGround, int nCheckCreature, bool bCheckMove, int nX0, int nY0, int nX1, int nY1, int *pXm, int *pYm)
{
    const auto stvPathNode = parseMovePath(nX0, nY0, nX1, nY1, bCheckGround, nCheckCreature);
    switch(stvPathNode.size()){
        case 0:
        case 1:
            {
                return false;
            }
        default:
            {
                if(bCheckMove){

                    // request to check if the first step is possible
                    // to avoid creature to move into invalid or occupied grids

                    auto nXt = stvPathNode[1].X;
                    auto nYt = stvPathNode[1].Y;

                    int nDX = (nXt > nX0) - (nXt < nX0);
                    int nDY = (nYt > nY0) - (nYt < nY0);

                    int nIndexMax = 0;
                    switch(mathf::LDistance2(nX0, nY0, nXt, nYt)){
                        case 1:
                        case 2:
                            {
                                nIndexMax = 1;
                                break;
                            }
                        case 4:
                        case 8:
                            {
                                nIndexMax = 2;
                                break;
                            }
                        case  9:
                        case 18:
                            {
                                nIndexMax = 3;
                                break;
                            }
                        default:
                            {
                                return false;
                            }
                    }

                    int nReachIndexMax = 0;
                    for(int nIndex = 1; nIndex <= nIndexMax; ++nIndex){
                        if(m_processRun->canMove(true, 2, nX0 + nDX * nIndex, nY0 + nDY * nIndex)){
                            nReachIndexMax = nIndex;
                        }else{ break; }
                    }

                    switch(nReachIndexMax){
                        case 0:
                            {
                                // we find an impossible step
                                // need to reject and report failure
                                return false;
                            }
                        default:
                            {
                                // we allow step size as 1, 2, 3
                                // every creature has two possible step size 1, MaxStep

                                // so if it failed to reach grid with MaxStep size
                                // it should use sizeStep as 1 only

                                // like if human on horse failed for a MOTION_HORSERUN
                                // then it should only use MOTION_HORSEWALK, rather than MOTION_RUN

                                if(pXm){
                                    *pXm = (nReachIndexMax == nIndexMax) ? nXt : (nX0 + nDX);
                                }

                                if(pYm){
                                    *pYm = (nReachIndexMax == nIndexMax) ? nYt : (nY0 + nDY);
                                }

                                return true;
                            }
                    }

                }else{

                    // won't check if srcLoc -> midLoc is possible
                    // 1. could contain invalid grids if not set bCheckGround
                    // 2. could contain occuped grids if not set nCheckCreature

                    if(pXm){
                        *pXm = stvPathNode[1].X;
                    }

                    if(pYm){
                        *pYm = stvPathNode[1].Y;
                    }

                    return true;
                }
            }
    }
}

bool MyHero::decompActionMove()
{
    if(m_actionQueue.empty() || m_actionQueue.front().type != ACTION_MOVE){
        throw bad_reach();
    }

    const auto currAction = m_actionQueue.front();
    m_actionQueue.pop_front();

    int nX0 = currAction.x;
    int nY0 = currAction.y;
    int nX1 = currAction.aimX;
    int nY1 = currAction.aimY;

    if(!m_processRun->canMove(true, 0, nX0, nY0)){
        g_log->addLog(LOGTYPE_WARNING, "Motion start from invalid grid (%d, %d)", nX0, nY0);
        m_actionQueue.clear();
        return false;
    }

    switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
        case 0:
            {
                // pickup is done in Hero::parseAction()
                g_log->addLog(LOGTYPE_WARNING, "Motion invalid (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);

                // I have to clear all pending actions
                // because if I return true the next step treats the front action as basic one

                m_actionQueue.clear();
                return false;
            }
        default:
            {
                const auto fnaddHop = [this, currAction](int nXm, int nYm) -> bool
                {
                    switch(mathf::LDistance2<int>(currAction.aimX, currAction.aimY, nXm, nYm)){
                        case 0:
                            {
                                m_actionQueue.emplace_front(currAction);
                                return true;
                            }
                        default:
                            {
                                m_actionQueue.emplace_front(ActionMove
                                {
                                    .speed = currAction.speed,
                                    .x = nXm,
                                    .y = nYm,
                                    .aimX = currAction.aimX,
                                    .aimY = currAction.aimY,
                                    .pickUp = (bool)(currAction.extParam.move.pickUp),
                                    .onHorse = (bool)(currAction.extParam.move.onHorse),
                                });

                                m_actionQueue.emplace_front(ActionMove
                                {
                                    .speed = currAction.speed,
                                    .x = currAction.x,
                                    .y = currAction.y,
                                    .aimX = nXm,
                                    .aimY = nYm,
                                    .pickUp = false,
                                    .onHorse = (bool)(currAction.extParam.move.onHorse),
                                });
                                return true;
                            }
                    }
                };

                int nXm = -1;
                int nYm = -1;

                bool bCheckGround = m_processRun->canMove(true, 0, nX1, nY1);
                if(decompMove(bCheckGround, 1, true, nX0, nY0, nX1, nY1, &nXm, &nYm)){
                    return fnaddHop(nXm, nYm);
                }
                else{
                    if(bCheckGround){
                        // means there is no such way to there
                        // move as much as possible
                        if(decompMove(false, 1, true, nX0, nY0, nX1, nY1, &nXm, &nYm)){
                            return fnaddHop(nXm, nYm);
                        }
                        else{
                            // won't check the ground but failed
                            // only one possibility: the first step is not legal
                            return false;
                        }
                    }
                    else{
                        return false;
                    }
                }
            }
            break;
    }
}

bool MyHero::decompActionAttack()
{
    if(m_actionQueue.empty() || m_actionQueue.front().type != ACTION_ATTACK){
        throw bad_reach();
    }

    const auto currAction = m_actionQueue.front();
    m_actionQueue.pop_front();

    // when parsing ActionAttack
    // we don't use ActionAttack::X/Y

    // this X/Y is used to send to the server
    // for location verification only

    auto nX0 = m_currMotion->endX;
    auto nY0 = m_currMotion->endY;

    // use if need to keep the attack node
    // we use m_currMotion->endX/y instead of rstAction.x/y

    const ActionAttack attack
    {
        .speed = currAction.speed,
        .x = nX0,
        .y = nY0,
        .aimUID = currAction.aimUID,
        .damageID = currAction.extParam.attack.damageID,
    };

    if(auto coPtr = m_processRun->findUID(currAction.aimUID)){
        const auto nX1 = coPtr->x();
        const auto nY1 = coPtr->y();

        switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
            case 0:
                {
                    g_log->addLog(LOGTYPE_WARNING, "Invalid attack location (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);
                    m_actionQueue.clear();
                    return false;
                }
            case 1:
            case 2:
                {
                    m_actionQueue.emplace_front(attack);
                    return true;
                }
            default:
                {
                    // not close enough for the PLAIN_PHY_ATTACK
                    // need to schedule a path to move closer and then attack

                    int nXt = -1;
                    int nYt = -1;

                    if(decompMove(true, 1, true, nX0, nY0, nX1, nY1, &nXt, &nYt)){

                        // decompse the move
                        // but need to check if it's one step distance

                        switch(mathf::LDistance2(nXt, nYt, nX1, nY1)){
                            case 0:
                                {
                                    // one hop we can reach the attack location
                                    // but we know step size between (nX0, nY0) and (nX1, nY1) > 1

                                    int nXm  = -1;
                                    int nYm  = -1;
                                    int nDir = PathFind::GetDirection(nX0, nY0, nX1, nY1);
                                    PathFind::GetFrontLocation(&nXm, &nYm, nX0, nY0, nDir, 1);

                                    nXt = nXm;
                                    nYt = nYm;

                                    break;
                                }
                            default:
                                {
                                    break;
                                }
                        }

                        m_actionQueue.emplace_front(ActionAttack
                        {
                            .speed = currAction.speed,
                            .x = nXt,
                            .y = nYt,
                            .aimUID = currAction.aimUID,
                            .damageID = currAction.extParam.attack.damageID,
                        });

                        m_actionQueue.emplace_front(ActionMove
                        {
                            .speed = SYS_DEFSPEED,
                            .x = nX0,
                            .y = nY0,
                            .aimX = nXt,
                            .aimY = nYt,
                            .onHorse = OnHorse(),
                        });
                        return true;
                    }
                    else{

                        // decompse failed, why it failed?
                        // if can't reach we need to reject current action
                        // if caused by occupied grids of creatures, we need to keep it

                        if(decompMove(true, 0, false, nX0, nY0, nX1, nY1, nullptr, nullptr)){
                            // keep it
                            // can reach but not now
                            m_actionQueue.emplace_front(attack);
                        }
                        else{
                            m_actionQueue.clear();
                        }

                        // decompse failed
                        // the head of the action queue is not simple
                        return false;
                    }
                }
        }
    }

    // we can't get the UID
    // remove the attack node and return false
    return false;
}

bool MyHero::decompActionSpell()
{
    if(m_actionQueue.empty() || m_actionQueue.front().type != ACTION_SPELL){
        throw bad_reach();
    }

    const auto currAction = m_actionQueue.front();
    m_actionQueue.pop_front();

    const auto fnGetSpellDir = [this](int nX0, int nY0, int nX1, int nY1) -> int
    {
        switch(mathf::LDistance2<int>(nX0, nY0, nX1, nY1)){
            case 0:
                {
                    return m_currMotion->direction;
                }
            default:
                {
                    return PathFind::GetDirection(nX0, nY0, nX1, nY1);
                }
        }
    };

    const auto standDir = [&fnGetSpellDir, &currAction, this]() -> int
    {
        if(currAction.aimUID){
            if(auto coPtr = m_processRun->findUID(currAction.aimUID)){
                return fnGetSpellDir(currAction.x, currAction.y, coPtr->x(), coPtr->y());
            }
        }
        else if(m_processRun->canMove(true, 0, currAction.aimX, currAction.aimY)){
            return fnGetSpellDir(currAction.x, currAction.y, currAction.aimX, currAction.aimY);
        }
        return DIR_NONE;
    }();

    // adjust the direction and acknowledge server
    // otherwise server doesn't know client has made direction turn
    // when summon skeleton or dog it appears at wrong place, not in front

    if(directionValid(standDir) && m_currMotion->direction != standDir){
        m_actionQueue.emplace_front(ActionStand
        {
            .x = currAction.x,
            .y = currAction.y,
            .direction = standDir,
        });
    }

    // TODO like dual axe
    // some magic it has a attack distance

    m_actionQueue.emplace_front(currAction);
    return true;
}

bool MyHero::parseActionQueue()
{
    if(m_actionQueue.empty()){
        return true;
    }

    // trace message
    // trace move action before parsing
    if(g_clientArgParser->traceMove){
        if((!m_actionQueue.empty()) && (m_actionQueue.front().type == ACTION_MOVE)){
            const int motionX0 = m_currMotion->x;
            const int motionY0 = m_currMotion->y;
            const int motionX1 = m_currMotion->endX;
            const int motionY1 = m_currMotion->endY;

            const int actionX0 = m_actionQueue.front().x;
            const int actionY0 = m_actionQueue.front().y;
            const int actionX1 = m_actionQueue.front().aimX;
            const int actionY1 = m_actionQueue.front().aimY;

            g_log->addLog(LOGTYPE_INFO, "BF: CurrMotion: (%d, %d) -> (%d, %d)", motionX0, motionY0, motionX1, motionY1);
            g_log->addLog(LOGTYPE_INFO, "BF: CurrAction: (%d, %d) -> (%d, %d)", actionX0, actionY0, actionX1, actionY1);
        }
    }

    // parsing action
    // means pending motion queue must be empty
    if(!m_motionQueue.empty()){
        throw fflerror("motion queue is not empty");
    }

    // all actions in action queue is local and not verfified
    // present the action list immediately and sent it to server for verification
    // the server will keep silent if permitted, or send pull-back message if rejected

    // 1. decompose action into simple action if needed
    // 2. send the simple action to server for verification
    // 3. present the action simultaneously

    switch(m_actionQueue.front().type){
        case ACTION_MOVE:
            {
                if(!decompActionMove()){
                    return false;
                }
                break;
            }
        case ACTION_ATTACK:
            {
                if(!decompActionAttack()){
                    return false;
                }
                break;
            }
        case ACTION_SPELL:
            {
                if(!decompActionSpell()){
                    return false;
                }
                break;
            }
        default:
            {
                break;
            }
    }

    const auto currAction = m_actionQueue.front();
    m_actionQueue.pop_front();

    // present current *local* action without verification
    // later if server refused current action we'll do correction by pullback

    reportAction(currAction);
    if(parseAction(currAction)){
        // trace message
        // trace move action after parsing
        if(g_clientArgParser->traceMove){
            std::ranges::for_each(m_motionQueue, [](const auto &node)
            {
                const int motionX0 = node->x;
                const int motionY0 = node->y;
                const int motionX1 = node->endX;
                const int motionY1 = node->endY;
                g_log->addLog(LOGTYPE_INFO, "AF: CurrMotion: (%d, %d) -> (%d, %d)", motionX0, motionY0, motionX1, motionY1);
            });

            if(m_actionQueue.empty()){
                g_log->addLog(LOGTYPE_INFO, "AF: CurrAction: NONE");
            }
            else{
                std::ranges::for_each(m_actionQueue, [](const auto &node)
                {
                    const int actionX0 = node.x;
                    const int actionY0 = node.y;
                    const int actionX1 = node.aimX;
                    const int actionY1 = node.aimY;
                    g_log->addLog(LOGTYPE_INFO, "AF: CurrAction: (%d, %d) -> (%d, %d)", actionX0, actionY0, actionX1, actionY1);
                });
            }
        }

        // ParseAction() can make m_motionQueue empty
        // Like for ACTION_PICKUP

        if(!m_motionQueue.empty()){
            m_currMotion = std::move(m_motionQueue.front());
            m_motionQueue.pop_front();
        }
        return true;
    }
    return false;
}

bool MyHero::emplaceAction(const ActionNode &action)
{
    m_actionQueue.clear();
    m_actionQueue.push_back(action);
    return true;
}

void MyHero::reportAction(const ActionNode &action)
{
    CMAction cmA;
    std::memset(&cmA, 0, sizeof(cmA));

    cmA.UID = UID();
    cmA.mapID = m_processRun->mapID();
    cmA.action = action;

    g_client->send(CM_ACTION, cmA);
}

void MyHero::pullGold()
{
    g_client->send(CM_QUERYGOLD);
}

void MyHero::flushMotionPending()
{
    Hero::flushMotionPending();
    m_actionQueue.clear();
}

bool MyHero::canWear(uint32_t itemID, int wltype) const
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("invalid wltype: %d", wltype);
    }

    if(!itemID){
        throw fflerror("invalid itemID: %llu", to_llu(itemID));
    }

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        return false;
    }

    if(!ir.wearable(wltype)){
        return false;
    }

    if(wltype == WLG_DRESS && getClothGender(itemID) != gender()){
        return false;
    }

    // TODO
    // check item requirement

    return true;
}
