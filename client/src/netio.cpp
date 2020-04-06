/*
 * =====================================================================================
 *
 *       Filename: netio.cpp
 *        Created: 06/29/2015 07:18:27 PM
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

#include "log.hpp"
#include "netio.hpp"
#include "message.hpp"
#include "compress.hpp"
#include "condcheck.hpp"

NetIO::SendPack::SendPack(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnOnDone)
    : HC(nHC)
    , Data(pData)
    , DataLen(nDataLen)
    , OnDone(std::move(fnOnDone))
{
    auto fnReportAndExit = [this](){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Invalid client message: (%d, %p, %d)", (int)(HC), Data, (int)(DataLen));
        std::abort();
    };

    clientMsg stCMSG(HC);
    switch(stCMSG.type()){
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
                    nMaskCnt = Compress::CountMask(Data + 2, stCMSG.maskLen());
                }else{
                    nSizeLen = 1;
                    nCompLen = (int)(Data[0]);
                    nMaskCnt = Compress::CountMask(Data + 1, stCMSG.maskLen());
                }

                if(false
                        || (nMaskCnt < 0)
                        || (nCompLen != nMaskCnt)
                        || (DataLen != ((size_t)(nSizeLen) + stCMSG.maskLen() + (size_t)(nCompLen)))){
                    fnReportAndExit();
                    return;
                }
                break;
            }
        case 2:
            {
                // not empty, fixed size and not compressed
                if(!(Data && (DataLen == stCMSG.dataLen()))){ fnReportAndExit(); return; }
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

NetIO::NetIO()
    : m_IO()
    , m_Resolver(m_IO)
    , m_Socket(m_IO)
    , m_ReadHC(0)
    , m_ReadLen {0, 0, 0, 0}
    , m_ReadBuf(1024)
    , m_OnReadDone()
    , m_SendQueue()
    , m_MemoryPN()
{}

NetIO::~NetIO()
{
    m_IO.stop();
}

void NetIO::Shutdown()
{
    m_Socket.close();
}

void NetIO::DoReadHC()
{
    auto fnReportCurrentMessage = [this](){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING, "Current serverMsg::HC      = %d", (int)(m_ReadHC));
        g_Log->AddLog(LOGTYPE_WARNING, "                 ::Type    = %d", (int)(serverMsg(m_ReadHC).type()));
        g_Log->AddLog(LOGTYPE_WARNING, "                 ::MaskLen = %d", (int)(serverMsg(m_ReadHC).maskLen()));
        g_Log->AddLog(LOGTYPE_WARNING, "                 ::DataLen = %d", (int)(serverMsg(m_ReadHC).dataLen()));
    };

    auto fnOnNetError = [this, fnReportCurrentMessage](std::error_code stEC){
        if(stEC){
            // 1. close the asio socket
            Shutdown();

            // 2. record the error code to log
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_WARNING, "Network error: %s", stEC.message().c_str());
            fnReportCurrentMessage();
        }
    };

    auto fnOnReadHC = [this, fnOnNetError, fnReportCurrentMessage](std::error_code stEC, size_t){
        if(stEC){ fnOnNetError(stEC); }
        else{
            serverMsg stSMSG(m_ReadHC);
            switch(stSMSG.type()){
                case 0:
                    {
                        if(m_OnReadDone){ m_OnReadDone(m_ReadHC, nullptr, 0); }
                        DoReadHC();
                        return;
                    }
                case 1:
                    {
                        auto fnOnReadLen0 = [this, stSMSG, fnOnNetError, fnReportCurrentMessage](std::error_code stEC, size_t){
                            if(stEC){ fnOnNetError(stEC); }
                            else{
                                if(m_ReadLen[0] != 255){
                                    if((size_t)(m_ReadLen[0]) > stSMSG.dataLen()){
                                        // 1. close the asio socket
                                        Shutdown();

                                        // 2. record the error code but not exit?
                                        extern Log *g_Log;
                                        g_Log->AddLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(m_ReadLen[0]));
                                        fnReportCurrentMessage();

                                        // 3. we stop here
                                        //    should we have some method to save it?
                                        return;
                                    }else{ DoReadBody(stSMSG.maskLen(), m_ReadLen[0]); }
                                }else{
                                    auto fnOnReadLen1 = [this, stSMSG, fnReportCurrentMessage, fnOnNetError](std::error_code stEC, size_t){
                                        if(stEC){ fnOnNetError(stEC); }
                                        else{
                                            condcheck(m_ReadLen[0] == 255);
                                            auto nCompLen = (size_t)(m_ReadLen[1]) + 255;

                                            if(nCompLen > stSMSG.dataLen()){
                                                // 1. close the asio socket
                                                Shutdown();

                                                // 2. record the error code but not exit?
                                                extern Log *g_Log;
                                                g_Log->AddLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(nCompLen));
                                                fnReportCurrentMessage();

                                                // 3. we stop here
                                                //    should we have some method to save it?
                                                return;
                                            }else{ DoReadBody(stSMSG.maskLen(), nCompLen); }
                                        }
                                    };
                                    asio::async_read(m_Socket, asio::buffer(m_ReadLen + 1, 1), fnOnReadLen1);
                                }
                            }
                        };
                        asio::async_read(m_Socket, asio::buffer(m_ReadLen, 1), fnOnReadLen0);
                        return;
                    }
                case 2:
                    {
                        DoReadBody(0, stSMSG.dataLen());
                        return;
                    }
                case 3:
                    {
                        auto fnOnReadLen = [this, fnOnNetError](std::error_code stEC, size_t){
                            if(stEC){ fnOnNetError(stEC); }
                            else{
                                uint32_t nDataLenU32 = 0;
                                std::memcpy(&nDataLenU32, m_ReadLen, 4);
                                DoReadBody(0, nDataLenU32);
                            }
                        };
                        asio::async_read(m_Socket, asio::buffer(m_ReadLen, 4), fnOnReadLen);
                        return;
                    }
                default:
                    {
                        Shutdown();
                        fnReportCurrentMessage();
                        return;
                    }
            }
        }
    };
    asio::async_read(m_Socket, asio::buffer(&m_ReadHC, 1), fnOnReadHC);
}

bool NetIO::DoReadBody(size_t nMaskLen, size_t nBodyLen)
{
    auto fnReportCurrentMessage = [this](){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING, "Current serverMsg::HC      = %d", (int)(m_ReadHC));
        g_Log->AddLog(LOGTYPE_WARNING, "                 ::Type    = %d", (int)(serverMsg(m_ReadHC).type()));
        g_Log->AddLog(LOGTYPE_WARNING, "                 ::MaskLen = %d", (int)(serverMsg(m_ReadHC).maskLen()));
        g_Log->AddLog(LOGTYPE_WARNING, "                 ::DataLen = %d", (int)(serverMsg(m_ReadHC).dataLen()));
    };

    auto fnOnNetError = [this, fnReportCurrentMessage](std::error_code stEC){
        if(stEC){
            // 1. close the asio socket
            Shutdown();

            // 2. record the error code to log
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_WARNING, "Network error: %s", stEC.message().c_str());
            fnReportCurrentMessage();
        }
    };

    auto fnReportInvalidArg = [nMaskLen, nBodyLen, fnReportCurrentMessage]()
    {
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING, "Invalid argument to DoReadBody(MaskLen = %d, BodyLen = %d)", (int)(nMaskLen), (int)(nBodyLen));
        fnReportCurrentMessage();
    };

    serverMsg stSMSG(m_ReadHC);
    switch(stSMSG.type()){
        case 0:
            {
                fnReportInvalidArg();
                return false;
            }
        case 1:
            {
                if(!((nMaskLen == stSMSG.maskLen()) && (nBodyLen <= stSMSG.dataLen()))){
                    fnReportInvalidArg();
                    return false;
                }
                m_ReadBuf.resize(nMaskLen + nBodyLen + 8 + stSMSG.dataLen());
                break;
            }
        case 2:
            {
                if(nMaskLen || (nBodyLen != stSMSG.dataLen())){
                    fnReportInvalidArg();
                    return false;
                }
                m_ReadBuf.resize(stSMSG.dataLen());
                break;
            }
        case 3:
            {
                if(nMaskLen || (nBodyLen > 0XFFFFFFFF)){
                    fnReportInvalidArg();
                    return false;
                }
                m_ReadBuf.resize(stSMSG.dataLen());
                break;
            }
        default:
            {
                fnReportInvalidArg();
                return false;
            }
    }

    if(auto nDataLen = nMaskLen + nBodyLen){
        auto fnDoneReadData = [this, nMaskLen, nBodyLen, stSMSG, fnReportCurrentMessage, fnOnNetError](std::error_code stEC, size_t){
            if(stEC){ fnOnNetError(stEC); }
            else{
                if(nMaskLen){
                    auto nMaskCount = Compress::CountMask(&(m_ReadBuf[0]), nMaskLen);
                    if(nMaskCount != (int)(nBodyLen)){
                        // we get corrupted data
                        // should we ignore current package or kill the process?

                        // 1. keep a log for the corrupted message
                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_WARNING, "Corrupted data: MaskCount = %d, CompLen = %d", nMaskCount, (int)(nBodyLen));
                        fnReportCurrentMessage();

                        // 2. we ignore this message
                        //    won't shutdown current session, just return?
                        return;
                    }

                    if(nBodyLen <= stSMSG.dataLen()){
                        auto pMaskData = &(m_ReadBuf[0]);
                        auto pCompData = &(m_ReadBuf[nMaskLen]);
                        auto pOrigData = &(m_ReadBuf[((nMaskLen + nBodyLen + 7) / 8) * 8]);
                        if(Compress::Decode(pOrigData, stSMSG.dataLen(), pMaskData, pCompData) != (int)(nBodyLen)){
                            extern Log *g_Log;
                            g_Log->AddLog(LOGTYPE_WARNING, "Decode failed: MaskCount = %d, CompLen = %d", nMaskCount, (int)(nBodyLen));
                            fnReportCurrentMessage();
                            return;
                        }
                    }else{
                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_WARNING, "Corrupted data: DataLen = %d, CompLen = %d", (int)(stSMSG.dataLen()), (int)(nBodyLen));
                        fnReportCurrentMessage();
                        return;
                    }
                }

                // no matter decoding is needed or not
                // we should call the completion handler here

                // 1. call completion on decompressed data
                if(m_OnReadDone){
                    m_OnReadDone(m_ReadHC, &(m_ReadBuf[nMaskLen ? ((nMaskLen + nBodyLen + 7) / 8 * 8) : 0]), nMaskLen ? stSMSG.dataLen() : nBodyLen);
                }

                // 2. read next DoReadHC()
                DoReadHC();
            }
        };
        asio::async_read(m_Socket, asio::buffer(&(m_ReadBuf[0]), nDataLen), fnDoneReadData);
        return true;
    }

    // I report error when read boyd at mode-0 
    // but still if in mode3 and we're transfering empty message we can reach here
    if(m_OnReadDone){ m_OnReadDone(m_ReadHC, nullptr, 0); }
    DoReadHC();
    return true;
}

void NetIO::DoSendNext()
{
    condcheck(!m_SendQueue.empty());

    if(m_SendQueue.front().OnDone){
        m_SendQueue.front().OnDone();
    }

    if(m_SendQueue.front().Data){
        m_MemoryPN.Free(const_cast<uint8_t *>(m_SendQueue.front().Data));
    }

    m_SendQueue.pop();
    if(!m_SendQueue.empty()){ DoSendHC(); }
}

void NetIO::DoSendBuf()
{
    condcheck(!m_SendQueue.empty());
    if(m_SendQueue.front().Data && m_SendQueue.front().DataLen){
        auto fnDoSendValidBuf = [this](std::error_code stEC, size_t){
            if(stEC){
                // 1. close the asio socket
                Shutdown();

                // 2. record the error code and exit
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "Network error: %s", stEC.message().c_str());
            }else{ DoSendNext(); }
        };
        asio::async_write(m_Socket, asio::buffer(m_SendQueue.front().Data, m_SendQueue.front().DataLen), fnDoSendValidBuf);
    }else{ DoSendNext(); }
}

void NetIO::DoSendHC()
{
    asio::async_write(m_Socket, asio::buffer(&(m_SendQueue.front().HC), 1),
        [this](std::error_code stEC, size_t){
            if(stEC){
                // 1. close the asio socket
                Shutdown();

                // 2. record the error code and exit
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "Network error: %s", stEC.message().c_str());
            }else{ DoSendBuf(); }
        }
    );
}

bool NetIO::Send(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&fnDone)
{
    uint8_t *pEncodeData = nullptr;
    size_t   nEncodeSize = 0;

    auto fnReportError = [nHC, pData, nDataLen](){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING, "Invalid message to send: HC = %d, Data = %p, DataLen = %d", (int)(nHC), pData, (int)(nDataLen));
    };

    clientMsg stCMSG(nHC);
    switch(stCMSG.type()){
        case 0:
            {
                if(pData || nDataLen){
                    fnReportError();
                    return false;
                }
                break;
            }
        case 1:
            {
                if(!(pData && (stCMSG.dataLen() == nDataLen))){
                    fnReportError();
                    return false;
                }

                auto nCountData = Compress::CountData(pData, nDataLen);
                if((nCountData < 0) || (nCountData > (int)(stCMSG.dataLen()))){
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_WARNING, "Count failed: HC = %d, DataLen = %d, CountData = %d", (int)(nHC), (int)(nDataLen), nCountData);
                    return false;
                }else if(nCountData <= 254){
                    pEncodeData = (uint8_t *)(m_MemoryPN.Get(stCMSG.maskLen() + (size_t)(nCountData) + 1));
                    if(Compress::Encode(pEncodeData + 1, pData, nDataLen) != nCountData){
                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_WARNING, "Count failed: HC = %d, DataLen = %d, CountData = %d", (int)(nHC), (int)(nDataLen), nCountData);

                        m_MemoryPN.Free(pEncodeData);
                        return false;
                    }

                    pEncodeData[0] = (uint8_t)(nCountData);
                    nEncodeSize    = 1 + stCMSG.maskLen() + (size_t)(nCountData);
                }else if(nCountData <= (255 + 255)){
                    pEncodeData = (uint8_t *)(m_MemoryPN.Get(stCMSG.maskLen() + (size_t)(nCountData) + 2));
                    if(Compress::Encode(pEncodeData + 2, pData, nDataLen) != nCountData){
                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_WARNING, "Count failed: HC = %d, DataLen = %d, CountData = %d", (int)(nHC), (int)(nDataLen), nCountData);

                        m_MemoryPN.Free(pEncodeData);
                        return false;
                    }

                    pEncodeData[0] = 255;
                    pEncodeData[1] = (uint8_t)(nCountData - 255);
                    nEncodeSize    = 2 + stCMSG.maskLen() + (size_t)(nCountData);
                }else{
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_WARNING, "Count overflows: HC = %d, DataLen = %d, CountData = %d", (int)(nHC), (int)(nDataLen), nCountData);

                    m_MemoryPN.Free(pEncodeData);
                    return false;
                }
                break;
            }
        case 2:
            {
                if(!(pData && (nDataLen == stCMSG.dataLen()))){
                    fnReportError();
                    return false;
                }

                pEncodeData = (uint8_t *)(m_MemoryPN.Get(nDataLen));
                nEncodeSize = nDataLen;
                std::memcpy(pEncodeData, pData, nDataLen);
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

    // post the handler to the main loop
    // we'll change fnDone in lambda so have to make it mutable
    m_IO.post([this, nHC, pEncodeData, nEncodeSize, fnDone = std::move(fnDone)]() mutable {
        bool bEmpty = m_SendQueue.empty();
        m_SendQueue.emplace(nHC, pEncodeData, nEncodeSize, std::move(fnDone));

        //  if this is the only task
        //  we should start the flush procedure
        if(bEmpty){ DoSendHC(); }
    });
    return true;
}

bool NetIO::InitIO(const char *szIP, const char * szPort, const std::function<void(uint8_t, const uint8_t *, size_t)> &fnOnDone)
{
    // check arguments, should do further check
    // like IP address is valid and Port > 1024 etc.
    if(!(szIP && szPort)){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING, "Invalid endpoint providen: (%s:%s)", szIP ? szIP : "", szPort ? szPort : "");
        return false;
    }

    // 1. register the completion handler
    m_OnReadDone = fnOnDone;
    if(!m_OnReadDone){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_WARNING, "Register completion handler not executable");
        return false;
    }

    // 2. try to connect to server
    //    this just put an handler in the event pool, should pool to drive it
    asio::async_connect(m_Socket, m_Resolver.resolve({szIP, szPort}),
        [this](std::error_code stEC, asio::ip::tcp::resolver::iterator){
            if(stEC){
                // 1. close the asio socket
                Shutdown();

                // 2. record the error code and exit
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "Network error: %s", stEC.message().c_str());

                // 3. the main loop can check socket::is_open()
                //    to inform the user that current socket is working or run into errors

                // 4. else call DoReadHC() to get the first HC
                //    all DoReadHC() should be called after the invocation of completion handler
            }else{ DoReadHC(); }
        }
    );

    // 3. we won't call asio::io_service::run() here
    //    instead we'll explicitly call asio::io_service::poll() in Client::MainLoop()
    return true;
}

void NetIO::PollIO()
{
    m_IO.poll();
}

void NetIO::StopIO()
{
    m_IO.post([this](){ Shutdown(); });
}
