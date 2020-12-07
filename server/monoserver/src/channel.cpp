/*
 * =====================================================================================
 *
 *       Filename: channel.cpp
 *        Created: 09/03/2015 03:48:41 AM
 *    Description: for received messages we won't crash if get invalid ones
 *                 but for messages to send we take zero tolerance
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

#include "channel.hpp"
#include "compress.hpp"
#include "netdriver.hpp"
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "messagepack.hpp"

Channel::Channel(uint32_t nChannID, asio::ip::tcp::socket stSocket)
    : m_ID(nChannID)
    , m_state(CHANNTYPE_NONE)
    , m_dispatcher()
    , m_socket(std::move(stSocket))
    , m_IP(m_socket.remote_endpoint().address().to_string())
    , m_port(m_socket.remote_endpoint().port())
    , m_readHC(0)
    , m_readLen {0, 0, 0, 0}
    , m_bodyLen(0)
    , m_readBuf()
    , m_decodeBuf()
    , m_bindUID(0)
    , m_flushFlag(false)
    , m_nextQLock()
    , m_sendPackQ0()
    , m_sendPackQ1()
    , m_currSendQ(&(m_sendPackQ0))
    , m_nextSendQ(&(m_sendPackQ1))
{}

Channel::~Channel()
{
    // access raw pointer here
    // don't use shared_from_this() in constructor or destructor

    Shutdown(true);

    extern NetDriver *g_netDriver;
    g_netDriver->RecycleChannID(ID());
}

void Channel::DoReadPackHC()
{
    switch(auto nCurrState = m_state.load()){
        case CHANNTYPE_STOPPED:
            {
                return;
            }
        case CHANNTYPE_RUNNING:
            {
                auto fnReportLastPack = [pThis = shared_from_this()]()
                {
                    extern MonoServer *g_monoServer;
                    g_monoServer->addLog(LOGTYPE_WARNING, "Last ClientMsg::HC      = %s", (ClientMsg(pThis->m_readHC).name().c_str()));
                    g_monoServer->addLog(LOGTYPE_WARNING, "              ::Type    = %d", (int)(ClientMsg(pThis->m_readHC).type()));
                    g_monoServer->addLog(LOGTYPE_WARNING, "              ::MaskLen = %d", (int)(ClientMsg(pThis->m_readHC).maskLen()));
                    g_monoServer->addLog(LOGTYPE_WARNING, "              ::DataLen = %d", (int)(ClientMsg(pThis->m_readHC).dataLen()));
                };

                auto fnOnNetError = [pThis = shared_from_this(), fnReportLastPack](std::error_code stEC)
                {
                    if(stEC){
                        // 1. close the asio socket
                        pThis->Shutdown(true);

                        // 2. record the error code to log
                        extern MonoServer *g_monoServer;
                        g_monoServer->addLog(LOGTYPE_WARNING, "Network error on channel %d: %s", (int)(pThis->ID()), stEC.message().c_str());
                        fnReportLastPack();
                    }
                };

                auto fnDoneReadHC = [pThis = shared_from_this(), fnOnNetError, fnReportLastPack](std::error_code stEC, size_t)
                {
                    if(stEC){
                        fnOnNetError(stEC);
                    }else{
                        ClientMsg stCMSG(pThis->m_readHC);
                        switch(stCMSG.type()){
                            case 0:
                                {
                                    pThis->ForwardActorMessage(pThis->m_readHC, nullptr, 0);
                                    pThis->DoReadPackHC();
                                    return;
                                }
                            case 1:
                                {
                                    // not empty, fixed size, compressed
                                    auto fnDoneReadLen0 = [pThis, stCMSG, fnOnNetError, fnReportLastPack](std::error_code stEC, size_t)
                                    {
                                        if(stEC){
                                            fnOnNetError(stEC);
                                        }else{
                                            if(pThis->m_readLen[0] != 255){
                                                if((size_t)(pThis->m_readLen[0]) > stCMSG.dataLen()){
                                                    // 1. close the asio socket
                                                    pThis->Shutdown(true);

                                                    // 2. record the error code but not exit?
                                                    extern MonoServer *g_monoServer;
                                                    g_monoServer->addLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(pThis->m_readLen[0]));
                                                    fnReportLastPack();

                                                    // 3. we stop here
                                                    //    should we have some method to save it?
                                                    return;
                                                }else{
                                                    pThis->DoReadPackBody(stCMSG.maskLen(), pThis->m_readLen[0]);
                                                }
                                            }else{
                                                // oooops, bytes[0] is 255
                                                // we got a long message and need to read bytes[1]
                                                auto fnDoneReadLen1 = [pThis, stCMSG, fnReportLastPack, fnOnNetError](std::error_code stEC, size_t)
                                                {
                                                    if(stEC){
                                                        fnOnNetError(stEC);
                                                    }else{
                                                        condcheck(pThis->m_readLen[0] == 255);
                                                        auto nCompLen = (size_t)(pThis->m_readLen[1]) + 255;

                                                        if(nCompLen > stCMSG.dataLen()){
                                                            // 1. close the asio socket
                                                            pThis->Shutdown(true);

                                                            // 2. record the error code but not exit?
                                                            extern MonoServer *g_monoServer;
                                                            g_monoServer->addLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(nCompLen));
                                                            fnReportLastPack();

                                                            // 3. we stop here
                                                            //    should we have some method to save it?
                                                            return;
                                                        }else{
                                                            pThis->DoReadPackBody(stCMSG.maskLen(), nCompLen);
                                                        }
                                                    }
                                                };
                                                asio::async_read(pThis->m_socket, asio::buffer(pThis->m_readLen + 1, 1), fnDoneReadLen1);
                                            }
                                        }
                                    };
                                    asio::async_read(pThis->m_socket, asio::buffer(pThis->m_readLen, 1), fnDoneReadLen0);
                                    return;
                                }
                            case 2:
                                {
                                    // not empty, fixed size, not compressed

                                    // it has no overhead, fast
                                    // this mode should be used for small messages
                                    pThis->DoReadPackBody(0, stCMSG.dataLen());
                                    return;
                                }
                            case 3:
                                {
                                    // not empty, not fixed size, not compressed

                                    // directly read four bytes as length
                                    // this mode is designed for transfering big chunk
                                    // for even bigger chunk we should send more than one time

                                    // and if we want to send big chunk of compressed data
                                    // we should compress it by other method and send it via this channel
                                    auto fnDoneReadLen = [pThis, fnOnNetError](std::error_code stEC, size_t)
                                    {
                                        if(stEC){
                                            fnOnNetError(stEC);
                                        }else{
                                            uint32_t nDataLenU32 = 0;
                                            std::memcpy(&nDataLenU32, pThis->m_readLen, 4);
                                            pThis->DoReadPackBody(0, (size_t)(nDataLenU32));
                                        }
                                    };
                                    asio::async_read(pThis->m_socket, asio::buffer(pThis->m_readLen, 4), fnDoneReadLen);
                                    return;
                                }
                            default:
                                {
                                    // impossible type
                                    // should abort at construction of ClientMsg
                                    pThis->Shutdown(true);
                                    fnReportLastPack();
                                    return;
                                }
                        }
                    }
                };
                asio::async_read(m_socket, asio::buffer(&m_readHC, 1), fnDoneReadHC);
                return;
            }
        default:
            {
                extern MonoServer *g_monoServer;
                g_monoServer->addLog(LOGTYPE_WARNING, "Calling DoReadPackHC() with invalid state: %d", nCurrState);
                return;
            }
    }
}

void Channel::DoReadPackBody(size_t nMaskLen, size_t nBodyLen)
{
    switch(auto nCurrState = m_state.load()){
        case CHANNTYPE_STOPPED:
            {
                return;
            }
        case CHANNTYPE_RUNNING:
            {
                auto fnReportLastPack = [pThis = shared_from_this()]()
                {
                    extern MonoServer *g_monoServer;
                    g_monoServer->addLog(LOGTYPE_WARNING, "Current serverMsg::HC      = %d", (int)(pThis->m_readHC));
                    g_monoServer->addLog(LOGTYPE_WARNING, "                 ::Type    = %d", (int)(ClientMsg(pThis->m_readHC).type()));
                    g_monoServer->addLog(LOGTYPE_WARNING, "                 ::MaskLen = %d", (int)(ClientMsg(pThis->m_readHC).maskLen()));
                    g_monoServer->addLog(LOGTYPE_WARNING, "                 ::DataLen = %d", (int)(ClientMsg(pThis->m_readHC).dataLen()));
                };

                auto fnOnNetError = [pThis = shared_from_this(), fnReportLastPack](std::error_code stEC)
                {
                    if(stEC){
                        // 1. close the asio socket
                        pThis->Shutdown(true);

                        // 2. record the error code to log
                        extern MonoServer *g_monoServer;
                        g_monoServer->addLog(LOGTYPE_WARNING, "Network error on channel %d: %s", (int)(pThis->ID()), stEC.message().c_str());
                        fnReportLastPack();
                    }
                };

                auto fnReportInvalidArg = [nMaskLen, nBodyLen, fnReportLastPack]()
                {
                    extern MonoServer *g_monoServer;
                    g_monoServer->addLog(LOGTYPE_WARNING, "Invalid argument to DoReadPackBody(MaskLen = %d, BodyLen = %d)", (int)(nMaskLen), (int)(nBodyLen));
                    fnReportLastPack();
                };

                // argument check
                // check if (nMaskLen, nBodyLen) is proper based on current m_readHC
                ClientMsg stCMSG(m_readHC);
                switch(stCMSG.type()){
                    case 0:
                        {
                            fnReportInvalidArg();
                            return;
                        }
                    case 1:
                        {
                            if(!((nMaskLen == stCMSG.maskLen()) && (nBodyLen <= stCMSG.dataLen()))){
                                fnReportInvalidArg();
                                return;
                            }
                            break;
                        }
                    case 2:
                        {
                            if(nMaskLen || (nBodyLen != stCMSG.dataLen())){
                                fnReportInvalidArg();
                                return;
                            }
                            break;
                        }
                    case 3:
                        {
                            if(nMaskLen || (nBodyLen > 0XFFFFFFFF)){
                                fnReportInvalidArg();
                                return;
                            }
                            break;
                        }
                    default:
                        {
                            fnReportInvalidArg();
                            return;
                        }
                }

                if(auto nDataLen = nMaskLen + nBodyLen){
                    auto pMem = GetReadBuf(nDataLen);
                    auto fnDoneReadData = [pThis = shared_from_this(), nMaskLen, nBodyLen, pMem, stCMSG, fnReportLastPack, fnOnNetError](std::error_code stEC, size_t)
                    {
                        if(stEC){
                            fnOnNetError(stEC);
                        }else{
                            uint8_t *pDecodeMem = nullptr;
                            if(nMaskLen){
                                auto nMaskCount = Compress::CountMask(pMem, nMaskLen);
                                if(nMaskCount != (int)(nBodyLen)){
                                    // we get corrupted data
                                    // should we ignore current package or kill the process?

                                    // 1. keep a log for the corrupted message
                                    extern MonoServer *g_monoServer;
                                    g_monoServer->addLog(LOGTYPE_WARNING, "Corrupted data: MaskCount = %d, CompLen = %d", nMaskCount, (int)(nBodyLen));

                                    // 2. we ignore this message
                                    //    won't shutdown current channel, just return?
                                    return;
                                }

                                // we need to decode it
                                // we do have a compressed version of data
                                if(nBodyLen <= stCMSG.dataLen()){
                                    pDecodeMem = pThis->GetDecodeBuf(stCMSG.dataLen());
                                    if(Compress::Decode(pDecodeMem, stCMSG.dataLen(), pMem, pMem + nMaskLen) != (int)(nBodyLen)){
                                        extern MonoServer *g_monoServer;
                                        g_monoServer->addLog(LOGTYPE_WARNING, "Decode failed: MaskCount = %d, CompLen = %d", nMaskCount, (int)(nBodyLen));
                                        fnReportLastPack();
                                        return;
                                    }
                                }else{
                                    extern MonoServer *g_monoServer;
                                    g_monoServer->addLog(LOGTYPE_WARNING, "Corrupted data: DataLen = %d, CompLen = %d", (int)(stCMSG.dataLen()), (int)(nBodyLen));
                                    fnReportLastPack();
                                    return;
                                }
                            }

                            // decoding and verification done
                            // we forward the (decoded/origin) data to the bind actor
                            pThis->ForwardActorMessage(pThis->m_readHC, pDecodeMem ? pDecodeMem : pMem, nMaskLen ? stCMSG.dataLen() : nBodyLen);
                            pThis->DoReadPackHC();
                        }
                    };
                    asio::async_read(m_socket, asio::buffer(pMem, nDataLen), fnDoneReadData);
                    return;
                }

                // possibilities to reach here
                // 1. call DoReadPackBody() with m_readHC as empty message type
                // 2. read a body with empty body in mode 3
                ForwardActorMessage(m_readHC, nullptr, 0);
                DoReadPackHC();
                return;
            }
        default:
            {
                extern MonoServer *g_monoServer;
                g_monoServer->addLog(LOGTYPE_WARNING, "Calling DoReadPackBody() with invalid state: %d", nCurrState);
                return;
            }
    }
}

void Channel::DoSendPack()
{
    // 1. only called in asio main loop thread
    // 2. only called in RUNNING / STOPPED state

    switch(auto nCurrState = m_state.load()){
        case CHANNTYPE_STOPPED:
            {
                // asio can't cancel a handler after post
                // so we have to check it here and exit directly if channel stopped
                // nothing is important now, we just return and won't take care of m_flushFlag
                return;
            }
        case CHANNTYPE_RUNNING:
            {
                // will send the first chann pack
                // channel state could switch to STOPPED during sending

                // when we are here
                // we should already have m_flushFlag set as true
                condcheck(m_flushFlag);

                // check m_currSendQ and if it's empty we swap with the pending queue
                // then for server threads calling Post() we only dealing with m_nextSendQ

                if(m_currSendQ->Empty()){
                    std::lock_guard<std::mutex> stLockGuard(m_nextQLock);
                    if(m_nextSendQ->Empty()){
                        // neither queue contains pending packages
                        // mark m_flushFlag as no one accessing m_currSendQ and return
                        m_flushFlag = false;
                        return;
                    }else{
                        // else we still need to access m_currSendQ
                        // keep m_flushFlag to pervent other thread to call DoSendHC()
                        std::swap(m_currSendQ, m_nextSendQ);
                    }
                }

                condcheck(!m_currSendQ->Empty());
                auto fnDoSendBuf = [pThis = shared_from_this()](std::error_code stEC, size_t)
                {
                    if(stEC){
                        // immediately shutdown the channel
                        pThis->Shutdown(true);

                        // report message and abort current process
                        extern MonoServer *g_monoServer;
                        g_monoServer->addLog(LOGTYPE_WARNING, "Network error on channel %d: %s", (int)(pThis->ID()), stEC.message().c_str());
                        return;
                    }

                    // send the first pack without error
                    // invoke the callback and register the next round

                    auto stCurrPack = pThis->m_currSendQ->GetChannPack();
                    if(stCurrPack.DoneCB){
                        stCurrPack.DoneCB();
                    }

                    pThis->m_currSendQ->RemoveChannPack();
                    pThis->DoSendPack();
                };

                auto stCurrPack = m_currSendQ->GetChannPack();
                asio::async_write(m_socket, asio::buffer(stCurrPack.Data, stCurrPack.DataLen), fnDoSendBuf);
                return;
            }
        default:
            {
                extern MonoServer *g_monoServer;
                g_monoServer->addLog(LOGTYPE_WARNING, "Calling DoSendPack() with invalid channel state: %d", nCurrState);
                return;
            }
    }
}

bool Channel::FlushSendQ()
{
    auto fnFlushSendQ = [pThis = shared_from_this()]()
    {
        // m_currSendQ assessing should always be in the asio main loop
        // the Channel::Post() should only access m_nextSendQ

        // then we don't have to put lock to protect m_currSendQ
        // but we need lock for m_nextSendQ, in child threads, in asio main loop

        // but we need to make sure there is only one procedure in asio main loop accessing m_currSendQ
        // because packages in m_currSendQ are divided into two parts: HC / Data
        // one package only get erased after Data is sent
        // then multiple procesdure in asio main loop may send HC / Data more than one time

        // use shared_ptr<Channel>() instead of raw this
        // then outside of asio main loop we use shared_ptr::reset()

        if(!pThis->m_flushFlag){
            //  mark as current some one is accessing it
            //  we don't even need to make m_flushFlag atomic since it's in one thread
            pThis->m_flushFlag = true;
            pThis->DoSendPack();
        }
    };

    // FlushSendQ() is called by server threads only
    // we post handler fnFlushSendQ to prevent from direct access to m_currSendQ
    m_socket.get_io_service().post(fnFlushSendQ);
    return true;
}

bool Channel::Post(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnDone)
{
    // post current message to NextSendQ
    // this function is called by one server thread
    {
        std::lock_guard<std::mutex> stLockGuard(m_nextQLock);
        m_nextSendQ->AddChannPack(nHC, pData, nDataLen, std::move(fnDone));
    }

    return FlushSendQ();
}

bool Channel::ForwardActorMessage(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    auto fnReportBadArgs = [nHC, pData, nDataLen]()
    {
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid argument: (%d, %p, %d)", (int)(nHC), pData, (int)(nDataLen));
    };

    ClientMsg stCMSG(nHC);
    switch(stCMSG.type()){
        case 0:
            {
                if(pData || nDataLen){
                    fnReportBadArgs();
                    return false;
                }
                break;
            }
        case 1:
        case 2:
            {
                if(!(pData && (nDataLen == stCMSG.dataLen()))){
                    fnReportBadArgs();
                    return false;
                }
                break;
            }
        case 3:
            {
                if(pData){
                    if((nDataLen == 0) || (nDataLen > 0XFFFFFFFF)){
                        fnReportBadArgs();
                        return false;
                    }
                }else{
                    if(nDataLen){
                        fnReportBadArgs();
                        return false;
                    }
                }
                break;
            }
        default:
            {
                return false;
            }
    }

    AMNetPackage stAMNP;
    std::memset(&stAMNP, 0, sizeof(stAMNP));

    stAMNP.ChannID = ID();
    stAMNP.Type    = nHC;
    stAMNP.DataLen = nDataLen;

    if(pData){
        if(nDataLen <= std::extent<decltype(stAMNP.DataBuf)>::value){
            stAMNP.Data = nullptr;
            std::memcpy(stAMNP.DataBuf, pData, nDataLen);
        }else{
            stAMNP.Data = new uint8_t[nDataLen];
            std::memcpy(stAMNP.Data, pData, nDataLen);
        }
    }

    return m_dispatcher.forward(m_bindUID, {MPK_NETPACKAGE, stAMNP});
}

void Channel::Shutdown(bool bForce)
{
    auto fnShutdown = [](auto pThis)
    {
        switch(auto nCurrState = pThis->m_state.exchange(CHANNTYPE_STOPPED)){
            case CHANNTYPE_STOPPED:
                {
                    return;
                }
            case CHANNTYPE_RUNNING:
                {
                    AMBadChannel stAMBC;
                    std::memset(&stAMBC, 0, sizeof(stAMBC));

                    // can forward to servicecore or player
                    // servicecore won't keep pointer *this* then we need to report it
                    stAMBC.ChannID = pThis->ID();

                    pThis->m_dispatcher.forward(pThis->m_bindUID, {MPK_BADCHANNEL, stAMBC});
                    pThis->m_bindUID = 0;

                    // if we call shutdown() here
                    // we need to use try-catch since if connection has already
                    // been broken, it throws exception

                    // try{
                    //     m_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                    // }catch(...){
                    // }

                    pThis->m_socket.close();
                    return;
                }
            default:
                {
                    extern MonoServer *g_monoServer;
                    g_monoServer->addLog(LOGTYPE_WARNING, "Calling Shutdown() with invalid state: %d", nCurrState);
                    return;
                }
        }
    };

    if(bForce){
        fnShutdown(this);
    }else{
        m_socket.get_io_service().post([pThis = shared_from_this(), fnShutdown]()
        {
            fnShutdown(pThis);
        });
    }
}

bool Channel::Launch(uint64_t rstAddr)
{
    // Launch is not thread safe
    // because it's accessing m_bindUID directly without protection

    if(rstAddr){
        m_bindUID = rstAddr;
        switch(auto nCurrState = m_state.exchange(CHANNTYPE_RUNNING)){
            case CHANNTYPE_NONE:
                {
                    // make state RUNNING first
                    // otherwise all DoXXXXFunc() will exit directly

                    m_socket.get_io_service().post([pThis = shared_from_this()]()
                    {
                        pThis->DoReadPackHC();
                    });
                    return true;
                }
            default:
                {
                    extern MonoServer *g_monoServer;
                    g_monoServer->addLog(LOGTYPE_WARNING, "Invalid channel state to launch: %d", nCurrState);
                    return false;
                }
        }
    }
    return false;
}
