/*
 * =====================================================================================
 *
 *       Filename: session.cpp
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

#include "session.hpp"
#include "memorypn.hpp"
#include "compress.hpp"
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "messagepack.hpp"

Session::SendTask::SendTask(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnOnDone)
    : HC(nHC)
    , Data(pData)
    , DataLen(nDataLen)
    , OnDone(std::move(fnOnDone))
{
    auto fnReportAndExit = [this]()
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid server message: (%d, %p, %d)", (int)(HC), Data, (int)(DataLen));
        g_MonoServer->Restart();
    };

    SMSGParam stSMSG(HC);
    switch(stSMSG.Type()){
        case 0:
            {
                // empty message, shouldn't contain anything
                if(Data || DataLen){
                    fnReportAndExit();
                    return;
                }
                break;
            }
        case 1:
            {
                // not empty, fixed size and compressed
                // data stream structure: [DataLen, Mask, Data]
                int nSizeLen = 0;
                int nCompLen = 0;
                int nMaskCnt = 0;

                if(Data[0] == 255){
                    nSizeLen = 2;
                    nCompLen = 255 + (int)(Data[1]);
                    nMaskCnt = Compress::CountMask(Data + 2, stSMSG.MaskLen());
                }else{
                    nSizeLen = 1;
                    nCompLen = (int)(Data[0]);
                    nMaskCnt = Compress::CountMask(Data + 1, stSMSG.MaskLen());
                }

                if(false
                        || (nMaskCnt < 0)
                        || (nCompLen != nMaskCnt)
                        || (DataLen != ((size_t)(nSizeLen) + stSMSG.MaskLen() + (size_t)(nCompLen)))){
                    fnReportAndExit();
                    return;
                }
                break;
            }
        case 2:
            {
                // not empty, fixed size and not compressed
                if(!(Data && (DataLen == stSMSG.DataLen()))){
                    fnReportAndExit();
                    return;
                }
                break;
            }
        case 3:
            {
                // not empty, not fixed size and not compressed
                if(!(Data && (DataLen >= 4))){
                    fnReportAndExit();
                    return;
                }else{
                    uint32_t nDataLenU32 = 0;
                    std::memcpy(&nDataLenU32, Data, 4);
                    if(((size_t)(nDataLenU32) + 4) != DataLen){
                        fnReportAndExit();
                        return;
                    }
                }
                break;
            }
        default:
            {
                fnReportAndExit();
                break;
            }
    }
}

Session::Session(uint32_t nSessionID, asio::ip::tcp::socket stSocket)
    : m_ID(nSessionID)
    , m_Dispatcher()
    , m_Socket(std::move(stSocket))
    , m_IP(m_Socket.remote_endpoint().address().to_string())
    , m_Port(m_Socket.remote_endpoint().port())
    , m_ReadHC(0)
    , m_ReadLen {0, 0, 0, 0}
    , m_Delay(0)
    , m_BindAddress(Theron::Address::Null())
    , m_FlushFlag(false)
    , m_NextQLock()
    , m_SendQBuf0()
    , m_SendQBuf1()
    , m_CurrSendQ(&(m_SendQBuf0))
    , m_NextSendQ(&(m_SendQBuf1))
    , m_MemoryPN()
    , m_State(SESSTYPE_NONE)
{}

Session::~Session()
{
    // handlers posted to the asio main loop will refer to *this
    // when calling destructor make sure no outstanding handlers posted by it

    // don't use shared_from_this() in constructor or destructor
    // so Shutdown(true) should accessing raw this pointer
    Shutdown(true);
}

void Session::DoReadHC()
{
    switch(auto nCurrState = m_State.load()){
        case SESSTYPE_STOPPED:
            {
                return;
            }
        case SESSTYPE_RUNNING:
            {
                auto fnReportCurrentMessage = [pThis = shared_from_this()]()
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Current CMSGParam::HC      = %s", (CMSGParam(pThis->m_ReadHC).Name().c_str()));
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::Type    = %d", (int)(CMSGParam(pThis->m_ReadHC).Type()));
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::MaskLen = %d", (int)(CMSGParam(pThis->m_ReadHC).MaskLen()));
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::DataLen = %d", (int)(CMSGParam(pThis->m_ReadHC).DataLen()));
                };

                auto fnOnNetError = [pThis = shared_from_this(), fnReportCurrentMessage](std::error_code stEC)
                {
                    if(stEC){
                        // 1. close the asio socket
                        pThis->Shutdown(true);

                        // 2. record the error code to log
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Network error on session %d: %s", (int)(pThis->ID()), stEC.message().c_str());
                        fnReportCurrentMessage();
                    }
                };

                auto fnDoneReadHC = [pThis = shared_from_this(), fnOnNetError, fnReportCurrentMessage](std::error_code stEC, size_t)
                {
                    if(stEC){
                        fnOnNetError(stEC);
                    }else{
                        CMSGParam stCMSG(pThis->m_ReadHC);
                        switch(stCMSG.Type()){
                            case 0:
                                {
                                    pThis->ForwardActorMessage(pThis->m_ReadHC, nullptr, 0);
                                    pThis->DoReadHC();
                                    return;
                                }
                            case 1:
                                {
                                    // not empty, fixed size, compressed
                                    auto fnDoneReadLen0 = [pThis, stCMSG, fnOnNetError, fnReportCurrentMessage](std::error_code stEC, size_t)
                                    {
                                        if(stEC){
                                            fnOnNetError(stEC);
                                        }else{
                                            if(pThis->m_ReadLen[0] != 255){
                                                if((size_t)(pThis->m_ReadLen[0]) > stCMSG.DataLen()){
                                                    // 1. close the asio socket
                                                    pThis->Shutdown(true);

                                                    // 2. record the error code but not exit?
                                                    extern MonoServer *g_MonoServer;
                                                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(pThis->m_ReadLen[0]));
                                                    fnReportCurrentMessage();

                                                    // 3. we stop here
                                                    //    should we have some method to save it?
                                                    return;
                                                }else{
                                                    pThis->DoReadBody(stCMSG.MaskLen(), pThis->m_ReadLen[0]);
                                                }
                                            }else{
                                                // oooops, bytes[0] is 255
                                                // we got a long message and need to read bytes[1]
                                                auto fnDoneReadLen1 = [pThis, stCMSG, fnReportCurrentMessage, fnOnNetError](std::error_code stEC, size_t)
                                                {
                                                    if(stEC){
                                                        fnOnNetError(stEC);
                                                    }else{
                                                        condcheck(pThis->m_ReadLen[0] == 255);
                                                        auto nCompLen = (size_t)(pThis->m_ReadLen[1]) + 255;

                                                        if(nCompLen > stCMSG.DataLen()){
                                                            // 1. close the asio socket
                                                            pThis->Shutdown(true);

                                                            // 2. record the error code but not exit?
                                                            extern MonoServer *g_MonoServer;
                                                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(nCompLen));
                                                            fnReportCurrentMessage();

                                                            // 3. we stop here
                                                            //    should we have some method to save it?
                                                            return;
                                                        }else{
                                                            pThis->DoReadBody(stCMSG.MaskLen(), nCompLen);
                                                        }
                                                    }
                                                };
                                                asio::async_read(pThis->m_Socket, asio::buffer(pThis->m_ReadLen + 1, 1), fnDoneReadLen1);
                                            }
                                        }
                                    };
                                    asio::async_read(pThis->m_Socket, asio::buffer(pThis->m_ReadLen, 1), fnDoneReadLen0);
                                    return;
                                }
                            case 2:
                                {
                                    // not empty, fixed size, not compressed

                                    // it has no overhead, fast
                                    // this mode should be used for small messages
                                    pThis->DoReadBody(0, stCMSG.DataLen());
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
                                            std::memcpy(&nDataLenU32, pThis->m_ReadLen, 4);
                                            pThis->DoReadBody(0, (size_t)(nDataLenU32));
                                        }
                                    };
                                    asio::async_read(pThis->m_Socket, asio::buffer(pThis->m_ReadLen, 4), fnDoneReadLen);
                                    return;
                                }
                            default:
                                {
                                    // impossible type
                                    // should abort at construction of CMSGParam
                                    pThis->Shutdown(true);
                                    fnReportCurrentMessage();
                                    return;
                                }
                        }
                    }
                };
                asio::async_read(m_Socket, asio::buffer(&m_ReadHC, 1), fnDoneReadHC);
                return;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Calling DoReadHC() with invalid state: %d", nCurrState);
                return;
            }
    }
}

void Session::DoReadBody(size_t nMaskLen, size_t nBodyLen)
{
    switch(auto nCurrState = m_State.load()){
        case SESSTYPE_STOPPED:
            {
                return;
            }
        case SESSTYPE_RUNNING:
            {
                auto fnReportCurrentMessage = [pThis = shared_from_this()]()
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Current SMSGParam::HC      = %d", (int)(pThis->m_ReadHC));
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::Type    = %d", (int)(CMSGParam(pThis->m_ReadHC).Type()));
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::MaskLen = %d", (int)(CMSGParam(pThis->m_ReadHC).MaskLen()));
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::DataLen = %d", (int)(CMSGParam(pThis->m_ReadHC).DataLen()));
                };

                auto fnOnNetError = [pThis = shared_from_this(), fnReportCurrentMessage](std::error_code stEC)
                {
                    if(stEC){
                        // 1. close the asio socket
                        pThis->Shutdown(true);

                        // 2. record the error code to log
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Network error on session %d: %s", (int)(pThis->ID()), stEC.message().c_str());
                        fnReportCurrentMessage();
                    }
                };

                auto fnReportInvalidArg = [nMaskLen, nBodyLen, fnReportCurrentMessage]()
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid argument to DoReadBody(MaskLen = %d, BodyLen = %d)", (int)(nMaskLen), (int)(nBodyLen));
                    fnReportCurrentMessage();
                };

                // argument check
                // check if (nMaskLen, nBodyLen) is proper based on current m_ReadHC
                CMSGParam stCMSG(m_ReadHC);
                switch(stCMSG.Type()){
                    case 0:
                        {
                            fnReportInvalidArg();
                            return;
                        }
                    case 1:
                        {
                            if(!((nMaskLen == stCMSG.MaskLen()) && (nBodyLen <= stCMSG.DataLen()))){
                                fnReportInvalidArg();
                                return;
                            }
                            break;
                        }
                    case 2:
                        {
                            if(nMaskLen || (nBodyLen != stCMSG.DataLen())){
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
                    // we use global memory pool for read
                    // since the allocated buffer will be passed to actor
                    // and it's de-allocated by actor message handler, not here
                    extern MemoryPN *g_MemoryPN;
                    auto pMem = (uint8_t *)(g_MemoryPN->Get(nDataLen));

                    auto fnDoneReadData = [pThis = shared_from_this(), nMaskLen, nBodyLen, pMem, stCMSG, fnReportCurrentMessage, fnOnNetError](std::error_code stEC, size_t)
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
                                    extern MonoServer *g_MonoServer;
                                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Corrupted data: MaskCount = %d, CompLen = %d", nMaskCount, (int)(nBodyLen));

                                    // 2. we ignore this message
                                    //    won't shutdown current session, just return?
                                    return;
                                }

                                // we need to decode it
                                // we do have a compressed version of data
                                if(nBodyLen <= stCMSG.DataLen()){
                                    extern MemoryPN *g_MemoryPN;
                                    pDecodeMem = (uint8_t *)(g_MemoryPN->Get(stCMSG.DataLen()));
                                    if(Compress::Decode(pDecodeMem, stCMSG.DataLen(), pMem, pMem + nMaskLen) != (int)(nBodyLen)){
                                        // 1. keep a record for this failure
                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Decode failed: MaskCount = %d, CompLen = %d", nMaskCount, (int)(nBodyLen));
                                        fnReportCurrentMessage();

                                        // 2. free memory
                                        g_MemoryPN->Free(pMem);
                                        g_MemoryPN->Free(pDecodeMem);

                                        // 3. do nothing but return directly
                                        return;
                                    }else{
                                        // ok decoding succeed, free pMem
                                        g_MemoryPN->Free(pMem);
                                    }
                                }else{
                                    extern MonoServer *g_MonoServer;
                                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Corrupted data: DataLen = %d, CompLen = %d", (int)(stCMSG.DataLen()), (int)(nBodyLen));
                                    fnReportCurrentMessage();
                                    return;
                                }
                            }

                            // decoding and verification done
                            // we pass the pointer to actor and it's released inside actor
                            pThis->ForwardActorMessage(pThis->m_ReadHC, pDecodeMem ? pDecodeMem : pMem, nMaskLen ? stCMSG.DataLen() : nBodyLen);
                            pThis->DoReadHC();
                        }
                    };
                    asio::async_read(m_Socket, asio::buffer(pMem, nDataLen), fnDoneReadData);
                    return;
                }

                // possibilities to reach here
                // 1. call DoReadBody() with m_ReadHC as empty message type
                // 2. read a body with empty body in mode 3
                ForwardActorMessage(m_ReadHC, nullptr, 0);
                DoReadHC();
                return;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Calling DoReadBody() with invalid state: %d", nCurrState);
                return;
            }
    }
}

void Session::DoSendNext()
{
    switch(auto nCurrState = m_State.load()){
        case SESSTYPE_STOPPED:
            {
                return;
            }
        case SESSTYPE_RUNNING:
            {
                condcheck(m_FlushFlag);
                condcheck(!m_CurrSendQ->empty());

                if(m_CurrSendQ->front().OnDone){
                    m_CurrSendQ->front().OnDone();
                }

                if(m_CurrSendQ->front().Data && m_CurrSendQ->front().DataLen){
                    m_MemoryPN.Free(const_cast<uint8_t *>(m_CurrSendQ->front().Data));
                }

                m_CurrSendQ->pop();
                DoSendHC();

                return;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Calling DoSendNext() with invalid state: %d", nCurrState);
                return;
            }
    }
}

void Session::DoSendBuf()
{
    switch(auto nCurrState = m_State.load()){
        case SESSTYPE_STOPPED:
            {
                return;
            }
        case SESSTYPE_RUNNING:
            {
                condcheck(m_FlushFlag);
                condcheck(!m_CurrSendQ->empty());

                if(m_CurrSendQ->front().Data && m_CurrSendQ->front().DataLen){
                    auto fnDoneSend = [pThis = shared_from_this()](std::error_code stEC, size_t)
                    {
                        if(stEC){
                            // 1. shutdown current connection
                            pThis->Shutdown(true);

                            // 2. report message and abort current process
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Network error on session %d: %s", (int)(pThis->ID()), stEC.message().c_str());
                            return;
                        }else{
                            // don't do buffer release and callback invocation here
                            // we put it in DoSendNext()
                            pThis->DoSendNext();
                        }
                    };

                    // the Data field should contains all needed size info
                    // when call Session::Send() it should be compressed if necessary and put it there
                    asio::async_write(m_Socket, asio::buffer(m_CurrSendQ->front().Data, m_CurrSendQ->front().DataLen), fnDoneSend);
                }else{
                    DoSendNext();
                }
                return;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Calling DoSendBuf() with invalid state: %d", nCurrState);
                return;
            }
    }
}

void Session::DoSendHC()
{
    // 1. only called in asio main loop thread
    // 2. only called in RUNNING / STOPPED state

    switch(auto nCurrState = m_State.load()){
        case SESSTYPE_STOPPED:
            {
                // asio can't cancel a handler after post
                // so we have to check it here and exit directly if session stopped
                // nothing is important now, we just return and won't take care of m_FlushFlag
                return;
            }
        case SESSTYPE_RUNNING:
            {
                // will send this header code
                // session state could switch to STOPPED during sending

                // when we are here
                // we should already have m_FlushFlag set as true
                condcheck(m_FlushFlag);

                // we check m_CurrSendQ and if it's empty we swap with the pending queue
                // then for server threads calling Send() we only dealing with m_NextSendQ

                // but in asio main loop thread calling DoSendHC()
                // when we finished all tasks in m_CurrSendQ (ro swapped into m_CurrSendQ) we just stopped
                // all posted tasks after have to wait for next post to call FlushSendQ() to drive then send

                // we may put a self check for each session
                // for every x ms we automatically call FlushSendQ()

                if(m_CurrSendQ->empty()){
                    std::lock_guard<std::mutex> stLockGuard(m_NextQLock);
                    if(m_NextSendQ->empty()){
                        // neither queue contains pending packages
                        // mark m_FlushFlag as no one accessing m_CurrSendQ and return
                        m_FlushFlag = false;
                        return;
                    }else{
                        // else we still need to access m_CurrSendQ 
                        // keep m_FlushFlag to pervent other thread to call DoSendHC()
                        std::swap(m_CurrSendQ, m_NextSendQ);
                    }
                }

                condcheck(!m_CurrSendQ->empty());
                auto fnDoSendBuf = [pThis = shared_from_this()](std::error_code stEC, size_t)
                {
                    // we always use pThis here
                    // but actually we know since DoSendHC() is invoked by pThis always
                    // then here using this or pThis should be both OK

                    if(stEC){
                        // 1. shutdown current connection
                        pThis->Shutdown(true);

                        // 2. report message and abort current process
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Network error on session %d: %s", (int)(pThis->ID()), stEC.message().c_str());
                        return;
                    }else{
                        pThis->DoSendBuf();
                    }
                };

                asio::async_write(m_Socket, asio::buffer(&(m_CurrSendQ->front().HC), 1), fnDoSendBuf);
                return;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Calling DoSendHC() with invalid state: %d", nCurrState);
                return;
            }
    }
}

bool Session::FlushSendQ()
{
    auto fnFlushSendQ = [pThis = shared_from_this()]()
    {
        // m_CurrSendQ assessing should always be in the asio main loop
        // the Session::Send() should only access m_NextSendQ

        // then we don't have to put lock to protect m_CurrSendQ
        // but we need lock for m_NextSendQ, in child threads, in asio main loop

        // but we need to make sure there is only one procedure in asio main loop accessing m_CurrSendQ
        // because packages in m_CurrSendQ are divided into two parts: HC / Data 
        // one package only get erased after Data is sent
        // then multiple procesdure in asio main loop may send HC / Data more than one time

        // use shared_ptr<Session>() instead of raw this
        // then outside of asio main loop we use shared_ptr::reset()

        if(!pThis->m_FlushFlag){
            //  mark as current some one is accessing it
            //  we don't even need to make m_FlushFlag atomic since it's in one thread
            pThis->m_FlushFlag = true;
            pThis->DoSendHC();
        }
    };

    // FlushSendQ() is called by server threads only
    // we post handler fnFlushSendQ to prevent from direct access to m_CurrSendQ
    m_Socket.get_io_service().post(fnFlushSendQ);
    return true;
}

bool Session::Send(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnDone)
{
    // BuildTask should be thread-safe
    // it's using the internal memory pool to build the task block

    if(auto stTask = BuildTask(nHC, pData, nDataLen, std::move(fnDone))){
        // ready to send
        {
            std::lock_guard<std::mutex> stLockGuard(m_NextQLock);
            m_NextSendQ->emplace(std::move(stTask));
        }

        // 3. notify asio main loop
        return FlushSendQ();
    }
    return false;
}

Session::SendTask Session::BuildTask(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnDone)
{
    size_t   nEncodeSize = 0;
    uint8_t *pEncodeData = nullptr;

    auto fnReportError = [nHC, pData, nDataLen](const char *pErrorMessage)
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "%s: (%d, %p, %d)", (pErrorMessage ? pErrorMessage : "Empty message"), (int)(nHC), pData, (int)(nDataLen));
    };

    SMSGParam stSMSG(nHC);
    switch(stSMSG.Type()){
        case 0:
            {
                if(pData || nDataLen){
                    fnReportError("Invalid argument");
                    return Session::SendTask::Null();
                }
                break;
            }
        case 1:
            {
                // not empty, fixed size, comperssed
                if(!(pData && (nDataLen == stSMSG.DataLen()))){
                    fnReportError("Invalid argument");
                    return Session::SendTask::Null();
                }

                // do compression, two solutions
                //   1. we can allocate a buffer of size DataLen + (DataLen + 7) / 8
                //   2. we can count data before compression to know buffer size we needed
                // currently I'm using the second method, which helps to avoid re-allocate buffer if compressed size > 254

                // length encoding:
                // [0 - 254]          : length in 0 ~ 254
                // [    255][0 ~ 255] : length as 0 ~ 255 + 255

                // 1. most likely we are using 0 ~ 254
                // 2. if compressed length more than 254 we need to bytes
                // 3. we support range in [0, 255 + 255]

                auto nCountData = Compress::CountData(pData, nDataLen);
                if(nCountData < 0){
                    fnReportError("Count data failed");
                    return Session::SendTask::Null();
                }else if(nCountData <= 254){
                    // we need only one byte for length info
                    pEncodeData = (uint8_t *)(m_MemoryPN.Get(stSMSG.MaskLen() + (size_t)(nCountData) + 1));
                    if(Compress::Encode(pEncodeData + 1, pData, nDataLen) != nCountData){
                        // 1. keep a record for the failure
                        fnReportError("Compression failed");

                        // 2. free memory allocated and return immediately
                        m_MemoryPN.Free(pEncodeData);
                        return Session::SendTask::Null();
                    }

                    pEncodeData[0] = (uint8_t)(nCountData);
                    nEncodeSize    = 1 + stSMSG.MaskLen() + (size_t)(nCountData);
                }else if(nCountData <= (255 + 255)){
                    // we need two byte for length info
                    pEncodeData = (uint8_t *)(m_MemoryPN.Get(stSMSG.MaskLen() + (size_t)(nCountData) + 2));
                    if(!Compress::Encode(pEncodeData + 2, pData, nDataLen)){
                        // 1. keep a record for the failure
                        fnReportError("Compression failed");

                        // 2. free memory allocated and return immediately
                        m_MemoryPN.Free(pEncodeData);
                        return Session::SendTask::Null();
                    }

                    pEncodeData[0] = 255;
                    pEncodeData[1] = (uint8_t)(nCountData - 255);
                    nEncodeSize    = 2 + stSMSG.MaskLen() + (size_t)(nCountData);
                }else{
                    // compressed message is toooooo long
                    // should use another mode to send this message type

                    // 1. keep a record for the failure
                    fnReportError("Compressed data too long");

                    // 2. free memory allocated and return immediately
                    m_MemoryPN.Free(pEncodeData);
                    return Session::SendTask::Null();
                }
                break;
            }
        case 2:
            {
                // not empty, fixed size, not compressed
                if(!(pData && (nDataLen == stSMSG.DataLen()))){
                    fnReportError("Invalid argument");
                    return Session::SendTask::Null();
                }

                pEncodeData = (uint8_t *)(m_MemoryPN.Get(nDataLen));
                nEncodeSize = stSMSG.DataLen();

                // for fixed size and uncompressed message
                // we don't need to send the length info since it's public known
                std::memcpy(pEncodeData, pData, nDataLen);
                break;
            }
        case 3:
            {
                // not empty, not fixed size, not compressed
                if(pData){
                    if((nDataLen == 0) || (nDataLen > 0XFFFFFFFF)){
                        fnReportError("Invalid argument");
                        return Session::SendTask::Null();
                    }
                }else{
                    if(nDataLen){
                        fnReportError("Invalid argument");
                        return Session::SendTask::Null();
                    }
                }

                pEncodeData = (uint8_t *)(m_MemoryPN.Get(nDataLen + 4));
                nEncodeSize = nDataLen + 4;

                // 1. setup the message length encoding
                {
                    auto nDataLenU32 = (uint32_t)(nDataLen);
                    std::memcpy(pEncodeData, &nDataLenU32, sizeof(nDataLenU32));
                }

                // 2. copy data if there is
                if(pData){ std::memcpy(pEncodeData + 4, pData, nDataLen); }
                break;
            }
        default:
            {
                fnReportError("Invalid argument");
                return Session::SendTask::Null();
            }
    }

    return {nHC, pEncodeData, nEncodeSize, std::move(fnDone)};
}

bool Session::ForwardActorMessage(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    auto fnReportError = [nHC, pData, nDataLen]()
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid argument: (%d, %p, %d)", (int)(nHC), pData, (int)(nDataLen));
    };

    CMSGParam stCMSG(nHC);
    switch(stCMSG.Type()){
        case 0:
            {
                if(pData || nDataLen){
                    fnReportError();
                    return false;
                }
                break;
            }
        case 1:
        case 2:
            {
                if(!(pData && (nDataLen == stCMSG.DataLen()))){
                    fnReportError();
                    return false;
                }
                break;
            }
        case 3:
            {
                if(pData){
                    if((nDataLen == 0) || (nDataLen > 0XFFFFFFFF)){
                        fnReportError();
                        return false;
                    }
                }else{
                    if(nDataLen){
                        fnReportError();
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
    stAMNP.SessionID = ID();
    stAMNP.Type      = nHC;
    stAMNP.Data      = pData;
    stAMNP.DataLen   = nDataLen;

    // didn't register any response for net package
    // session is a sync-driver, means if we request a response
    // we should do busy-polling for it and this hurts performance

    // if we support response
    // we can free the memory allocated by memory pool
    // currently I free it in the destination actor handling logic
    return m_Dispatcher.Forward({MPK_NETPACKAGE, stAMNP}, m_BindAddress);
}

void Session::Shutdown(bool bForce)
{
    auto fnShutdown = [](auto pThis)
    {
        switch(auto nCurrState = pThis->m_State.exchange(SESSTYPE_STOPPED)){
            case SESSTYPE_STOPPED:
                {
                    return;
                }
            case SESSTYPE_RUNNING:
                {
                    AMBadSession stAMBS;
                    stAMBS.SessionID = pThis->ID();

                    pThis->m_Dispatcher.Forward({MPK_BADSESSION, stAMBS}, pThis->m_BindAddress);
                    pThis->m_BindAddress = Theron::Address::Null();

                    // if we call shutdown() here
                    // we need to use try-catch since if connection has already
                    // been broken, it throws exception

                    // try{
                    //     m_Socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                    // }catch(...){
                    // }

                    pThis->m_Socket.close();
                    return;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Calling Shutdown() with invalid state: %d", nCurrState);
                    return;
                }
        }
    };

    if(bForce){
        fnShutdown(this);
    }else{
        m_Socket.get_io_service().post([pThis = shared_from_this(), fnShutdown](){ fnShutdown(pThis); });
    }
}

bool Session::Launch(const Theron::Address &rstAddr)
{
    // Launch is not thread safe
    // because it's accessing m_BindAddress directly without protection

    // life circle of a session 
    // 1. create
    // 2. Launch(service_core_address)
    // 3. Bind(player_address)
    //
    // 4. multithread: accessing send / receive
    // 5. multithread: shutdown()
    // 6. multithread: delete

    if(rstAddr){
        m_BindAddress = rstAddr;
        switch(auto nCurrState = m_State.exchange(SESSTYPE_RUNNING)){
            case SESSTYPE_NONE:
                {
                    // make state RUNNING first
                    // otherwise all DoXXXXFunc() will exit directly

                    m_Socket.get_io_service().post([pThis = shared_from_this()](){ pThis->DoReadHC(); });
                    break;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Calling Launch() with invalid state: %d", nCurrState);
                    return false;
                }
        }
    }
    return false;
}
