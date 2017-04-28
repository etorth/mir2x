/*
 * =====================================================================================
 *
 *       Filename: session.cpp
 *        Created: 09/03/2015 03:48:41 AM
 *  Last Modified: 04/28/2017 00:16:45
 *
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

#include "session.hpp"
#include "memorypn.hpp"
#include "compress.hpp"
#include "monoserver.hpp"

Session::SendTask::SendTask(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnOnDone)
    : HC(nHC)
    , Data(pData)
    , DataLen(nDataLen)
    , OnDone(std::move(fnOnDone))
{
    auto fnReportAndExit = [this](){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid server message: (%d, %p, %d)", (int)(HC), Data, (int)(DataLen));
        g_MonoServer->Restart();
    };

    SMSGParam stSMSG(HC);
    switch(stSMSG.Type()){
        case 0:
            {
                // empty message, shouldn't contain anything
                if(Data || DataLen){ fnReportAndExit(); return; }
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
                if(!(Data && (DataLen == stSMSG.DataLen()))){ fnReportAndExit(); return; }
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
    : SyncDriver()
    , m_ID(nSessionID)
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
{}

Session::~Session()
{
    Shutdown();
}

bool Session::ForwardActorMessage(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    // we won't exit if invalid argument providen
    auto fnReportError = [nHC, pData, nDataLen](){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid argument : ForwardActorMessage(HC = %d, Data = %p, DataLen = %d)", (int)(nHC), pData, (int)(nDataLen));
    };

    CMSGParam stCMSG(nHC);
    switch(stCMSG.Type()){
        case 0:
            {
                // no data allowed
                if(pData || nDataLen){ fnReportError(); return false; }
                break;
            }
        case 1:
        case 2:
            {
                if(!(pData && (nDataLen == stCMSG.DataLen()))){ fnReportError(); return false; }
                break;
            }
        case 3:
            {
                if(pData){
                    if((nDataLen == 0) || (nDataLen > 0XFFFFFFFF)){ fnReportError(); return false; }
                }else{
                    if(nDataLen){ fnReportError(); return false; }
                }
                break;
            }
        default:
            {
                return false;
            }
    }

    AMNetPackage stAMNP;
    stAMNP.SessionID = m_ID;
    stAMNP.Type      = nHC;
    stAMNP.Data      = pData;
    stAMNP.DataLen   = nDataLen;

    // didn't register any response for net package
    // session is a sync-driver, means if we request a response
    // we should do busy-polling for it and this hurts performance

    // if we support response
    // we can free the memory allocated by memory pool
    // currently I free it in the destination actor handling logic
    return Forward({MPK_NETPACKAGE, stAMNP}, m_BindAddress);
}

void Session::DoReadHC()
{
    auto fnReportCurrentMessage = [this](){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Current CMSGParam::HC      = %d", (int)(m_ReadHC));
        g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::Type    = %d", (int)(CMSGParam(m_ReadHC).Type()));
        g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::MaskLen = %d", (int)(CMSGParam(m_ReadHC).MaskLen()));
        g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::DataLen = %d", (int)(CMSGParam(m_ReadHC).DataLen()));
    };

    auto fnOnNetError = [this, fnReportCurrentMessage](std::error_code stEC){
        if(stEC){
            // 1. close the asio socket
            Shutdown();

            // 2. record the error code to log
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Network error: %s", stEC.message().c_str());
            fnReportCurrentMessage();
        }
    };

    auto fnDoneReadHC = [this, fnOnNetError, fnReportCurrentMessage](std::error_code stEC, size_t){
        if(stEC){ fnOnNetError(stEC); }
        else{
            CMSGParam stCMSG(m_ReadHC);
            switch(stCMSG.Type()){
                case 0:
                    {
                        ForwardActorMessage(m_ReadHC, nullptr, 0);
                        DoReadHC();
                        return;
                    }
                case 1:
                    {
                        // not empty, fixed size, compressed
                        auto fnDoneReadLen0 = [this, stCMSG, fnOnNetError, fnReportCurrentMessage](std::error_code stEC, size_t){
                            if(stEC){ fnOnNetError(stEC); }
                            else{
                                if(m_ReadLen[0] != 255){
                                    if((size_t)(m_ReadLen[0]) > stCMSG.DataLen()){
                                        // 1. close the asio socket
                                        Shutdown();

                                        // 2. record the error code but not exit?
                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(m_ReadLen[0]));
                                        fnReportCurrentMessage();

                                        // 3. we stop here
                                        //    should we have some method to save it?
                                        return;
                                    }else{ DoReadBody(stCMSG.MaskLen(), m_ReadLen[0]); }
                                }else{
                                    // oooops, bytes[0] is 255
                                    // we got a long message and need to read bytes[1]
                                    auto fnDoneReadLen1 = [this, stCMSG, fnReportCurrentMessage, fnOnNetError](std::error_code stEC, size_t){
                                        if(stEC){ fnOnNetError(stEC); }
                                        else{
                                            assert(m_ReadLen[0] == 255);
                                            auto nCompLen = (size_t)(m_ReadLen[1]) + 255;

                                            if(nCompLen > stCMSG.DataLen()){
                                                // 1. close the asio socket
                                                Shutdown();

                                                // 2. record the error code but not exit?
                                                extern MonoServer *g_MonoServer;
                                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(nCompLen));
                                                fnReportCurrentMessage();

                                                // 3. we stop here
                                                //    should we have some method to save it?
                                                return;
                                            }else{ DoReadBody(stCMSG.MaskLen(), nCompLen); }
                                        }
                                    };
                                    asio::async_read(m_Socket, asio::buffer(m_ReadLen + 1, 1), fnDoneReadLen1);
                                }
                            }
                        };
                        asio::async_read(m_Socket, asio::buffer(m_ReadLen, 1), fnDoneReadLen0);
                        return;
                    }
                case 2:
                    {
                        // not empty, fixed size, not compressed

                        // it has no overhead, fast
                        // this mode should be used for small messages
                        DoReadBody(0, stCMSG.DataLen());
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
                        auto fnDoneReadLen = [this, fnOnNetError](std::error_code stEC, size_t){
                            if(stEC){ fnOnNetError(stEC); }
                            else{
                                uint32_t nDataLenU32 = 0;
                                std::memcpy(&nDataLenU32, m_ReadLen, 4);
                                DoReadBody(0, (size_t)(nDataLenU32));
                            }
                        };
                        asio::async_read(m_Socket, asio::buffer(m_ReadLen, 4), fnDoneReadLen);
                        return;
                    }
                default:
                    {
                        // impossible type
                        // should abort at construction of CMSGParam
                        Shutdown();
                        fnReportCurrentMessage();
                        return;
                    }
            }
        }
    };
    asio::async_read(m_Socket, asio::buffer(&m_ReadHC, 1), fnDoneReadHC);
}

bool Session::DoReadBody(size_t nMaskLen, size_t nBodyLen)
{
    auto fnReportCurrentMessage = [this](){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Current SMSGParam::HC      = %d", (int)(m_ReadHC));
        g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::Type    = %d", (int)(CMSGParam(m_ReadHC).Type()));
        g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::MaskLen = %d", (int)(CMSGParam(m_ReadHC).MaskLen()));
        g_MonoServer->AddLog(LOGTYPE_WARNING, "                 ::DataLen = %d", (int)(CMSGParam(m_ReadHC).DataLen()));
    };

    auto fnOnNetError = [this, fnReportCurrentMessage](std::error_code stEC){
        if(stEC){
            // 1. close the asio socket
            Shutdown();

            // 2. record the error code to log
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Network error: %s", stEC.message().c_str());
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
                return false;
            }
        case 1:
            {
                if(!((nMaskLen == stCMSG.MaskLen()) && (nBodyLen <= stCMSG.DataLen()))){
                    fnReportInvalidArg();
                    return false;
                }
                break;
            }
        case 2:
            {
                if(nMaskLen || (nBodyLen != stCMSG.DataLen())){
                    fnReportInvalidArg();
                    return false;
                }
                break;
            }
        case 3:
            {
                if(nMaskLen || (nBodyLen > 0XFFFFFFFF)){
                    fnReportInvalidArg();
                    return false;
                }
                break;
            }
        default:
            {
                fnReportInvalidArg();
                return false;
            }
    }

    if(auto nDataLen = nMaskLen + nBodyLen){
        // we use global memory pool for read
        // since the allocated buffer will be passed to actor
        // and it's de-allocated by actor message handler, not here
        extern MemoryPN *g_MemoryPN;
        auto pMem = (uint8_t *)(g_MemoryPN->Get(nDataLen));

        auto fnDoneReadData = [this, nMaskLen, nBodyLen, pMem, stCMSG, fnReportCurrentMessage, fnOnNetError](std::error_code stEC, size_t){
            if(stEC){ fnOnNetError(stEC); }
            else{
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
                ForwardActorMessage(m_ReadHC, pDecodeMem ? pDecodeMem : pMem, nMaskLen ? stCMSG.DataLen() : nBodyLen);
                DoReadHC();
            }
        };
        asio::async_read(m_Socket, asio::buffer(pMem, nDataLen), fnDoneReadData);
        return true;
    }

    // possibilities to reach here
    // 1. call DoReadBody() with m_ReadHC as empty message type
    // 2. read a body with empty body in mode 3
    ForwardActorMessage(m_ReadHC, nullptr, 0);
    DoReadHC();
    return true;
}

void Session::DoSendNext()
{
    assert(m_FlushFlag);
    assert(!m_CurrSendQ->empty());

    if(m_CurrSendQ->front().OnDone){
        m_CurrSendQ->front().OnDone();
    }

    if(m_CurrSendQ->front().Data && m_CurrSendQ->front().DataLen){
        m_MemoryPN.Free(const_cast<uint8_t *>(m_CurrSendQ->front().Data));
    }

    m_CurrSendQ->pop();
    DoSendHC();
}

void Session::DoSendBuf()
{
    assert(m_FlushFlag);
    assert(!m_CurrSendQ->empty());
    if(m_CurrSendQ->front().Data && m_CurrSendQ->front().DataLen){
        auto fnDoneSend = [this](std::error_code stEC, size_t){
            if(stEC){
                // 1. shutdown current connection
                Shutdown();

                // 2. report message and abort current process
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_FATAL, "Network error: %s", stEC.message().c_str());
                g_MonoServer->Restart();
                return;
            }else{
                // don't do buffer release and callback invocation here
                // we put it in DoSendNext()
                DoSendNext();
            }
        };

        // the Data field should contains all needed size info
        // when call Session::Send() it should be compressed if necessary and put it there
        asio::async_write(m_Socket, asio::buffer(m_CurrSendQ->front().Data, m_CurrSendQ->front().DataLen), fnDoneSend);
    }else{ DoSendNext(); }
}

void Session::DoSendHC()
{
    // when we are here
    // we should already have m_FlushFlag set as true
    assert(m_FlushFlag);

    // we check m_CurrSendQ and if it empty we swap with the pending queue
    // we move this swap thing to the handler in FlushSendQ()
    // means we only handle m_CurrSendQ in DoSendHC()
    // but which means after we done m_CurrSendQ we have to wait next FlushSendQ() to drive the send
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

    assert(!m_CurrSendQ->empty());
    auto fnDoSendBuf = [this](std::error_code stEC, size_t){
        if(stEC){
            // 1. shutdown current connection
            Shutdown();

            // 2. report message and abort current process
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_FATAL, "Network error: %s", stEC.message().c_str());
            g_MonoServer->Restart();
            return;
        }else{ DoSendBuf(); }
    };
    asio::async_write(m_Socket, asio::buffer(&(m_CurrSendQ->front().HC), 1), fnDoSendBuf);
}

bool Session::FlushSendQ()
{
    auto fnFlushSendQ = [this](){
        // m_CurrSendQ assessing should always be in the asio main loop
        // the Session::Send() should only access m_NextSendQ
        // 
        // then we don't have to put lock to protect m_CurrSendQ
        // but we need lock for m_NextSendQ, in child threads, in asio main loop
        //
        // but we need to make sure there is only one procedure in asio main loop accessing m_CurrSendQ
        // because packages in m_CurrSendQ are divided into  two parts: HC / Data 
        // one package only get erased after Data is sent
        // then  multiple procesdure in asio main loop may send HC / Data more than one time
        if(!m_FlushFlag){
            //  mark as current some one is accessing it
            //  we don't even need to make m_FlushFlag atomic since it's in one thread
            m_FlushFlag = true;
            DoSendHC();
        }
    };

    // FlushSendQ() is called by child threads only
    // so we prevent it from access m_CurrSendQ
    // instead everytime we post a handler to asio main loop
    //
    // this hurts the performance but make a better logic framework
    // we can also make m_FlushFlag atomic and use it to protect m_CurrSendQ
    m_Socket.get_io_service().post(fnFlushSendQ);
    return true;
}

bool Session::Send(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnDone)
{
    uint8_t *pEncodeData = nullptr;
    size_t   nEncodeSize = 0;

    auto fnReportError = [nHC, pData, nDataLen](){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid message: (%d, %p, %d)", (int)(nHC), pData, (int)(nDataLen));
    };

    SMSGParam stSMSG(nHC);
    switch(stSMSG.Type()){
        case 0:
            {
                if(pData || nDataLen){ fnReportError(); return false; }
                break;
            }
        case 1:
            {
                // not empty, fixed size, comperssed
                if(!(pData && (nDataLen == stSMSG.DataLen()))){ fnReportError(); return false; }

                // do compression, two solutions
                //    1. we can allocate a buffer of size DataLen + (DataLen + 7) / 8
                //    2. we can count data before compression to know buffer size we needed
                // currently I'm using the second method, which helps to avoid re-allocate buffer if compressed size > 254

                // length encoding:
                // [0 - 254]          : length in 0 ~ 254
                // [    255][0 ~ 255] : length as 0 ~ 255 + 255
                //
                // 1. most likely we are using 0 ~ 254
                // 2. if compressed length more than 254 we need to bytes
                // 3. we support range in [0, 255 + 255]

                auto nCountData = Compress::CountData(pData, nDataLen);
                if(nCountData < 0){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Count data failed: HC = %d, Data = %p, DataLen = %d", (int)(nHC), pData, (int)(nDataLen));
                    return false;
                }else if(nCountData <= 254){
                    // we need only one byte for length info
                    pEncodeData = (uint8_t *)(m_MemoryPN.Get(stSMSG.MaskLen() + (size_t)(nCountData) + 1));
                    if(Compress::Encode(pEncodeData + 1, pData, nDataLen) != nCountData){
                        // 1. keep a record for the failure
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Compress failed: HC = %d, Data = %p, DataLen = %d", (int)(nHC), pData, (int)(nDataLen));

                        // free memory allocated and return immediately
                        m_MemoryPN.Free(pEncodeData);
                        return false;
                    }

                    pEncodeData[0] = (uint8_t)(nCountData);
                    nEncodeSize    = 1 + stSMSG.MaskLen() + (size_t)(nCountData);
                }else if(nCountData <= (255 + 255)){
                    // we need two byte for length info
                    pEncodeData = (uint8_t *)(m_MemoryPN.Get(stSMSG.MaskLen() + (size_t)(nCountData) + 2));
                    if(!Compress::Encode(pEncodeData + 2, pData, nDataLen)){
                        // 1. keep a record for the failure
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Compress failed: HC = %d, Data = %p, DataLen = %d", (int)(nHC), pData, (int)(nDataLen));

                        // free memory allocated and return immediately
                        m_MemoryPN.Free(pEncodeData);
                        return false;
                    }

                    pEncodeData[0] = 255;
                    pEncodeData[1] = (uint8_t)(nCountData - 255);
                    nEncodeSize    = 2 + stSMSG.MaskLen() + (size_t)(nCountData);
                }else{
                    // compressed message is tooo long
                    // should use another mode to send this message type

                    // 1. keep a record for the failure
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Compressed data too long: HC = %d, Data = %p, DataLen = %d", (int)(nHC), pData, (int)(nDataLen));

                    // free memory allocated and return immediately
                    m_MemoryPN.Free(pEncodeData);
                    return false;
                }
                break;
            }
        case 2:
            {
                // not empty, fixed size, not compressed
                if(!(pData && (nDataLen == stSMSG.DataLen()))){ fnReportError(); return false; }

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
                    if((nDataLen == 0) || (nDataLen > 0XFFFFFFFF)){ fnReportError(); return false; }
                }else{
                    if(nDataLen){ fnReportError(); return false; }
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
                fnReportError();
                return false;
            }
    }

    // ready to send
    {
        std::lock_guard<std::mutex> stLockGuard(m_NextQLock);
        m_NextSendQ->emplace(nHC, pEncodeData, nEncodeSize, std::move(fnDone));
    }

    // 3. notify asio main loop
    return FlushSendQ();
}
