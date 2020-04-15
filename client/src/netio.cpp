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
#include "toll.hpp"
#include "netio.hpp"
#include "message.hpp"
#include "compress.hpp"
#include "fflerror.hpp"
#include "condcheck.hpp"

extern Log *g_Log;

NetIO::SendPack::SendPack(uint8_t hc, const uint8_t *dataBuf, size_t dataLength)
    : headCode(hc)
    , data(dataBuf)
    , dataLen(dataLength)
{
    const ClientMsg stCMSG(headCode);
    switch(stCMSG.type()){
        case 0:
            {
                if(data || dataLen){
                    throw fflerror("invalid client message: (%lld, %p, %lld)", toLLD(headCode), data, toLLD(dataLen));
                }
                break;
            }
        case 1:
            {
                int nSizeLen = 0;
                int nCompLen = 0;
                int nMaskCnt = 0;

                if(data[0] == 255){
                    nSizeLen = 2;
                    nCompLen = 255 + (int)(data[1]);
                    nMaskCnt = Compress::CountMask(data + 2, stCMSG.maskLen());
                }else{
                    nSizeLen = 1;
                    nCompLen = (int)(data[0]);
                    nMaskCnt = Compress::CountMask(data + 1, stCMSG.maskLen());
                }

                if(false
                        || (nMaskCnt < 0)
                        || (nCompLen != nMaskCnt)
                        || (dataLen != ((size_t)(nSizeLen) + stCMSG.maskLen() + (size_t)(nCompLen)))){
                    throw fflerror("invalid client message: (%lld, %p, %lld)", toLLD(headCode), data, toLLD(dataLen));
                }
                break;
            }
        case 2:
            {
                // not empty, fixed size and not compressed
                if(!(data && (dataLen == stCMSG.dataLen()))){
                    throw fflerror("invalid client message: (%lld, %p, %lld)", toLLD(headCode), data, toLLD(dataLen));
                }
                break;
            }
        case 3:
            {
                // not empty, not fixed size and not compressed
                if(!(data && (dataLen >= 4))){
                    throw fflerror("invalid client message: (%lld, %p, %lld)", toLLD(headCode), data, toLLD(dataLen));
                }

                uint32_t nDataLenU32 = 0;
                std::memcpy(&nDataLenU32, data, 4);
                if(((size_t)(nDataLenU32) + 4) != dataLen){
                    throw fflerror("invalid client message: (%lld, %p, %lld)", toLLD(headCode), data, toLLD(dataLen));
                }
                break;
            }
        default:
            {
                throw fflerror("invalid client message: (%lld, %p, %lld)", toLLD(headCode), data, toLLD(dataLen));
            }
    }
}

NetIO::NetIO()
    : m_io()
    , m_resolver(m_io)
    , m_socket(m_io)
    , m_ReadHC(0)
    , m_ReadLen {0, 0, 0, 0}
    , m_ReadBuf(1024)
    , m_msgHandler()
    , m_sendQueue()
    , m_MemoryPN()
{}

NetIO::~NetIO()
{
    m_io.stop();
}

void NetIO::readHeadCode()
{
    auto fnReportCurrentMessage = [this](){
        g_Log->addLog(LOGTYPE_WARNING, "Current ServerMsg::headCode = %d", (int)(m_ReadHC));
        g_Log->addLog(LOGTYPE_WARNING, "                 ::type     = %d", (int)(ServerMsg(m_ReadHC).type()));
        g_Log->addLog(LOGTYPE_WARNING, "                 ::daskLen  = %d", (int)(ServerMsg(m_ReadHC).maskLen()));
        g_Log->addLog(LOGTYPE_WARNING, "                 ::dataLen  = %d", (int)(ServerMsg(m_ReadHC).dataLen()));
    };

    auto fnOnNetError = [this, fnReportCurrentMessage](std::error_code errCode)
    {
        if(errCode){
            // 1. close the asio socket
            shutdown();

            // 2. record the error code to log
            g_Log->addLog(LOGTYPE_WARNING, "Network error: %s", errCode.message().c_str());
            fnReportCurrentMessage();
        }
    };

    auto fnOnReadHC = [this, fnOnNetError, fnReportCurrentMessage](std::error_code errCode, size_t)
    {
        if(errCode){
            fnOnNetError(errCode);
        }

        ServerMsg stSMSG(m_ReadHC);
        switch(stSMSG.type()){
            case 0:
                {
                    m_msgHandler(m_ReadHC, nullptr, 0);
                    readHeadCode();
                    return;
                }
            case 1:
                {
                    auto fnOnReadLen0 = [this, stSMSG, fnOnNetError, fnReportCurrentMessage](std::error_code errCode, size_t){
                        if(errCode){ fnOnNetError(errCode); }
                        else{
                            if(m_ReadLen[0] != 255){
                                if((size_t)(m_ReadLen[0]) > stSMSG.dataLen()){
                                    // 1. close the asio socket
                                    shutdown();

                                    // 2. record the error code but not exit?
                                    g_Log->addLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(m_ReadLen[0]));
                                    fnReportCurrentMessage();

                                    // 3. we stop here
                                    //    should we have some method to save it?
                                    return;
                                }else{ readBody(stSMSG.maskLen(), m_ReadLen[0]); }
                            }else{
                                auto fnOnReadLen1 = [this, stSMSG, fnReportCurrentMessage, fnOnNetError](std::error_code errCode, size_t){
                                    if(errCode){ fnOnNetError(errCode); }
                                    else{
                                        condcheck(m_ReadLen[0] == 255);
                                        auto nCompLen = (size_t)(m_ReadLen[1]) + 255;

                                        if(nCompLen > stSMSG.dataLen()){
                                            // 1. close the asio socket
                                            shutdown();

                                            // 2. record the error code but not exit?
                                            g_Log->addLog(LOGTYPE_WARNING, "Invalid package: CompLen = %d", (int)(nCompLen));
                                            fnReportCurrentMessage();

                                            // 3. we stop here
                                            //    should we have some method to save it?
                                            return;
                                        }else{ readBody(stSMSG.maskLen(), nCompLen); }
                                    }
                                };
                                asio::async_read(m_socket, asio::buffer(m_ReadLen + 1, 1), fnOnReadLen1);
                            }
                        }
                    };
                    asio::async_read(m_socket, asio::buffer(m_ReadLen, 1), fnOnReadLen0);
                    return;
                }
            case 2:
                {
                    readBody(0, stSMSG.dataLen());
                    return;
                }
            case 3:
                {
                    auto fnOnReadLen = [this, fnOnNetError](std::error_code errCode, size_t){
                        if(errCode){ fnOnNetError(errCode); }
                        else{
                            uint32_t nDataLenU32 = 0;
                            std::memcpy(&nDataLenU32, m_ReadLen, 4);
                            readBody(0, nDataLenU32);
                        }
                    };
                    asio::async_read(m_socket, asio::buffer(m_ReadLen, 4), fnOnReadLen);
                    return;
                }
            default:
                {
                    shutdown();
                    fnReportCurrentMessage();
                    return;
                }
        }
    };
    asio::async_read(m_socket, asio::buffer(&m_ReadHC, 1), fnOnReadHC);
}

bool NetIO::readBody(size_t nMaskLen, size_t nBodyLen)
{
    auto fnReportCurrentMessage = [this](){
        g_Log->addLog(LOGTYPE_WARNING, "Current ServerMsg::headCode = %d", (int)(m_ReadHC));
        g_Log->addLog(LOGTYPE_WARNING, "                 ::Type     = %d", (int)(ServerMsg(m_ReadHC).type()));
        g_Log->addLog(LOGTYPE_WARNING, "                 ::MaskLen  = %d", (int)(ServerMsg(m_ReadHC).maskLen()));
        g_Log->addLog(LOGTYPE_WARNING, "                 ::DataLen  = %d", (int)(ServerMsg(m_ReadHC).dataLen()));
    };

    auto fnOnNetError = [this, fnReportCurrentMessage](std::error_code errCode){
        if(errCode){
            // 1. close the asio socket
            shutdown();

            // 2. record the error code to log
            g_Log->addLog(LOGTYPE_WARNING, "Network error: %s", errCode.message().c_str());
            fnReportCurrentMessage();
        }
    };

    auto fnReportInvalidArg = [nMaskLen, nBodyLen, fnReportCurrentMessage]()
    {
        g_Log->addLog(LOGTYPE_WARNING, "Invalid argument to readBody(MaskLen = %d, BodyLen = %d)", (int)(nMaskLen), (int)(nBodyLen));
        fnReportCurrentMessage();
    };

    ServerMsg stSMSG(m_ReadHC);
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
        auto fnDoneReadData = [this, nMaskLen, nBodyLen, stSMSG, fnReportCurrentMessage, fnOnNetError](std::error_code errCode, size_t){
            if(errCode){ fnOnNetError(errCode); }
            else{
                if(nMaskLen){
                    auto nMaskCount = Compress::CountMask(&(m_ReadBuf[0]), nMaskLen);
                    if(nMaskCount != (int)(nBodyLen)){
                        // we get corrupted data
                        // should we ignore current package or kill the process?

                        // 1. keep a log for the corrupted message
                        g_Log->addLog(LOGTYPE_WARNING, "Corrupted data: MaskCount = %d, CompLen = %d", nMaskCount, (int)(nBodyLen));
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
                            g_Log->addLog(LOGTYPE_WARNING, "Decode failed: MaskCount = %d, CompLen = %d", nMaskCount, (int)(nBodyLen));
                            fnReportCurrentMessage();
                            return;
                        }
                    }else{
                        g_Log->addLog(LOGTYPE_WARNING, "Corrupted data: DataLen = %d, CompLen = %d", (int)(stSMSG.dataLen()), (int)(nBodyLen));
                        fnReportCurrentMessage();
                        return;
                    }
                }

                // no matter decoding is needed or not
                // we should call the completion handler here

                // 1. call completion on decompressed data
                m_msgHandler(m_ReadHC, &(m_ReadBuf[nMaskLen ? ((nMaskLen + nBodyLen + 7) / 8 * 8) : 0]), nMaskLen ? stSMSG.dataLen() : nBodyLen);

                // 2. read next readHeadCode()
                readHeadCode();
            }
        };
        asio::async_read(m_socket, asio::buffer(&(m_ReadBuf[0]), nDataLen), fnDoneReadData);
        return true;
    }

    // I report error when read boyd at mode-0 
    // but still if in mode3 and we're transfering empty message we can reach here
    m_msgHandler(m_ReadHC, nullptr, 0);
    readHeadCode();
    return true;
}

void NetIO::sendNext()
{
    condcheck(!m_sendQueue.empty());
    if(m_sendQueue.front().data){
        m_MemoryPN.Free(const_cast<uint8_t *>(m_sendQueue.front().data));
    }

    m_sendQueue.pop();
    if(!m_sendQueue.empty()){
        sendHeadCode();
    }
}

void NetIO::sendBuf()
{
    condcheck(!m_sendQueue.empty());
    if(m_sendQueue.front().data && m_sendQueue.front().dataLen){
        auto fnDoSendValidBuf = [this](std::error_code errCode, size_t)
        {
            if(errCode){
                // 1. close the asio socket
                shutdown();

                // 2. record the error code and exit
                g_Log->addLog(LOGTYPE_WARNING, "Network error: %s", errCode.message().c_str());
            }else{ sendNext(); }
        };
        asio::async_write(m_socket, asio::buffer(m_sendQueue.front().data, m_sendQueue.front().dataLen), fnDoSendValidBuf);
    }else{ sendNext(); }
}

void NetIO::sendHeadCode()
{
    asio::async_write(m_socket, asio::buffer(&(m_sendQueue.front().headCode), 1),
        [this](std::error_code errCode, size_t){
            if(errCode){
                // 1. close the asio socket
                shutdown();

                // 2. record the error code and exit
                g_Log->addLog(LOGTYPE_WARNING, "Network error: %s", errCode.message().c_str());
            }else{ sendBuf(); }
        }
    );
}

bool NetIO::send(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    uint8_t *pEncodeData = nullptr;
    size_t   nEncodeSize = 0;

    auto fnReportError = [nHC, pData, nDataLen]()
    {
        g_Log->addLog(LOGTYPE_WARNING, "Invalid message to send: headCode = %d, Data = %p, DataLen = %d", (int)(nHC), pData, (int)(nDataLen));
    };

    ClientMsg stCMSG(nHC);
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
                    g_Log->addLog(LOGTYPE_WARNING, "Count failed: headCode = %d, DataLen = %d, CountData = %d", (int)(nHC), (int)(nDataLen), nCountData);
                    return false;
                }else if(nCountData <= 254){
                    pEncodeData = (uint8_t *)(m_MemoryPN.Get(stCMSG.maskLen() + (size_t)(nCountData) + 1));
                    if(Compress::Encode(pEncodeData + 1, pData, nDataLen) != nCountData){
                        g_Log->addLog(LOGTYPE_WARNING, "Count failed: headCode = %d, DataLen = %d, CountData = %d", (int)(nHC), (int)(nDataLen), nCountData);

                        m_MemoryPN.Free(pEncodeData);
                        return false;
                    }

                    pEncodeData[0] = (uint8_t)(nCountData);
                    nEncodeSize    = 1 + stCMSG.maskLen() + (size_t)(nCountData);
                }else if(nCountData <= (255 + 255)){
                    pEncodeData = (uint8_t *)(m_MemoryPN.Get(stCMSG.maskLen() + (size_t)(nCountData) + 2));
                    if(Compress::Encode(pEncodeData + 2, pData, nDataLen) != nCountData){
                        g_Log->addLog(LOGTYPE_WARNING, "Count failed: headCode = %d, DataLen = %d, CountData = %d", (int)(nHC), (int)(nDataLen), nCountData);

                        m_MemoryPN.Free(pEncodeData);
                        return false;
                    }

                    pEncodeData[0] = 255;
                    pEncodeData[1] = (uint8_t)(nCountData - 255);
                    nEncodeSize    = 2 + stCMSG.maskLen() + (size_t)(nCountData);
                }else{
                    g_Log->addLog(LOGTYPE_WARNING, "Count overflows: headCode = %d, DataLen = %d, CountData = %d", (int)(nHC), (int)(nDataLen), nCountData);

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

    m_io.post([this, nHC, pEncodeData, nEncodeSize]() mutable
    {
        bool bEmpty = m_sendQueue.empty();
        m_sendQueue.emplace(nHC, pEncodeData, nEncodeSize);

        //  if this is the only task
        //  we should start the flush procedure
        if(bEmpty){
            sendHeadCode();
        }
    });
    return true;
}

void NetIO::start(const char *IPStr, const char * portStr, const std::function<void(uint8_t, const uint8_t *, size_t)> &msgHandler)
{
    if(!(IPStr && portStr)){
        throw fflerror("invalid endpoint providen: (%s:%s)", IPStr ? IPStr : "(null)", portStr ? portStr : "(null)");
    }

    // 1. register the completion handler
    if(!msgHandler){
        throw fflerror("invalid message handler");
    }
    m_msgHandler = msgHandler;
 
    // 2. try to connect to server
    //    this just put an handler in the event pool, should pool to drive it
    asio::async_connect(m_socket, m_resolver.resolve({IPStr, portStr}), [this](std::error_code errCode, asio::ip::tcp::resolver::iterator)
    {
        if(errCode){
            // 1. close the asio socket
            shutdown();

            // 2. record the error code and exit
            g_Log->addLog(LOGTYPE_WARNING, "Network error: %s", errCode.message().c_str());

            // 3. the main loop can check socket::is_open()
            //    to inform the user that current socket is working or run into errors

            // 4. else call readHeadCode() to get the first headCode
            //    all readHeadCode() should be called after the invocation of completion handler
        }
        else{
            readHeadCode();
        }
    });

    // 3. we won't call asio::io_service::run() here
    //    instead we'll explicitly call asio::io_service::poll() in Client::MainLoop()
}
