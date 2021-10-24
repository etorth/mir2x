#include "netio.hpp"
#include "totype.hpp"
#include "zcompf.hpp"
#include "message.hpp"
#include "fflerror.hpp"

void NetIO::doReadHeadCode()
{
    asio::async_read(m_socket, asio::buffer(&m_readHeadCode, 1), [this](std::error_code errCode, size_t)
    {
        if(errCode){
            throw fflerror("network error: %s", errCode.message().c_str());
        }

        const ServerMsg smsg(m_readHeadCode);
        switch(smsg.type()){
            case 0:
                {
                    m_msgHandler(m_readHeadCode, nullptr, 0);
                    doReadHeadCode();
                    return;
                }
            case 1:
                {
                    asio::async_read(m_socket, asio::buffer(m_readLen, 1), [this, smsg](std::error_code errCode, size_t)
                    {
                        if(errCode){
                            throw fflerror("network error: %s", errCode.message().c_str());
                        }

                        if(m_readLen[0] != 255){
                            fflassert(to_uz(m_readLen[0]) <= smsg.dataLen());
                            doReadBody(smsg.maskLen(), m_readLen[0]);
                        }
                        else{
                            asio::async_read(m_socket, asio::buffer(m_readLen + 1, 1), [this, smsg](std::error_code errCode, size_t)
                            {
                                if(errCode){
                                    throw fflerror("network error: %s", errCode.message().c_str());
                                }

                                fflassert(m_readLen[0] == 255);
                                const auto compSize = to_uz(m_readLen[1]) + 255;

                                fflassert(compSize <= smsg.dataLen());
                                doReadBody(smsg.maskLen(), compSize);
                            });
                        }
                    });
                    return;
                }
            case 2:
                {
                    doReadBody(0, smsg.dataLen());
                    return;
                }
            case 3:
                {
                    asio::async_read(m_socket, asio::buffer(m_readLen, 4), [this](std::error_code errCode, size_t)
                    {
                        if(errCode){
                            throw fflerror("network error: %s", errCode.message().c_str());
                        }
                        doReadBody(0, as_u32(m_readLen));
                    });
                    return;
                }
            default:
                {
                    throw bad_reach();
                }
        }
    });
}

void NetIO::doReadBody(size_t maskSize, size_t bodySize)
{
    const ServerMsg smsg(m_readHeadCode);
    fflassert(smsg.checkDataSize(maskSize, bodySize));

    switch(smsg.type()){
        case 1:
            {
                m_readBuf.resize(maskSize + bodySize + 64 + smsg.dataLen());
                break;
            }
        case 2:
            {
                m_readBuf.resize(smsg.dataLen());
                break;
            }
        case 3:
            {
                m_readBuf.resize(bodySize);
                break;
            }
        case 0:
        default:
            {
                throw bad_reach();
            }
    }

    if(const auto totalSize = maskSize + bodySize){
        asio::async_read(m_socket, asio::buffer(m_readBuf.data(), totalSize), [this, maskSize, bodySize, smsg](std::error_code errCode, size_t)
        {
            if(errCode){
                throw fflerror("network error: %s", errCode.message().c_str());
            }

            if(maskSize){
                const auto bitCount = zcompf::countMask(m_readBuf.data(), maskSize);
                fflassert(bitCount == bodySize);
                fflassert(bodySize <= smsg.dataLen());

                const auto maskDataPtr = m_readBuf.data();
                const auto compDataPtr = m_readBuf.data() + maskSize;
                /* */ auto origDataPtr = m_readBuf.data() + ((maskSize + bodySize + 7) / 8) * 8;

                const auto decodeSize = zcompf::xorDecode(origDataPtr, smsg.dataLen(), maskDataPtr, compDataPtr);
                fflassert(decodeSize == bodySize);
            }

            m_msgHandler(m_readHeadCode, m_readBuf.data() + (maskSize ? ((maskSize + bodySize + 7) / 8 * 8) : 0), maskSize ? smsg.dataLen() : bodySize);
            doReadHeadCode();
        });
    }
    else{
        m_msgHandler(m_readHeadCode, nullptr, 0);
        doReadHeadCode();
    }
}

void NetIO::doSendBuf()
{
    if(m_currSendBuf.empty()){
        return;
    }

    asio::async_write(m_socket, asio::buffer(m_currSendBuf.data(), m_currSendBuf.size()), [this](std::error_code errCode, size_t)
    {
        if(errCode){
            throw fflerror("network error: %s", errCode.message().c_str());
        }

        m_currSendBuf.clear();
        if(m_nextSendBuf.empty()){
            return;
        }

        std::swap(m_currSendBuf, m_nextSendBuf);
        doSendBuf();
    });
}

void NetIO::send(uint8_t headCode, const uint8_t *buf, size_t bufSize)
{
    const ClientMsg cmsg(headCode);
    fflassert(cmsg.checkData(buf, bufSize));

    const auto bodyStartOff = m_nextSendBuf.size() + 1; // after headCode
    m_nextSendBuf.push_back(headCode);
    switch(cmsg.type()){
        case 0:
            {
                break;
            }
        case 1:
            {
                const auto bitCount = zcompf::countData(buf, bufSize);
                fflassert(bitCount <= cmsg.dataLen());

                if(bitCount <= 254){
                    m_nextSendBuf.resize(m_nextSendBuf.size() + cmsg.maskLen() + to_uz(bitCount) + 1);
                    const auto encodeSize = zcompf::xorEncode(m_nextSendBuf.data() + bodyStartOff + 1, buf, bufSize);
                    fflassert(encodeSize == bitCount);
                    m_nextSendBuf[bodyStartOff] = bitCount;
                }
                else if(bitCount <= 255 + 255){
                    m_nextSendBuf.resize(m_nextSendBuf.size() + cmsg.maskLen() + to_uz(bitCount) + 2);
                    const auto encodeSize = zcompf::xorEncode(m_nextSendBuf.data() + bodyStartOff + 2, buf, bufSize);
                    fflassert(encodeSize == bitCount);
                    m_nextSendBuf[bodyStartOff] = 255;
                    m_nextSendBuf[bodyStartOff + 1] = to_u8(bitCount - 255);
                }
                else{
                    throw fflerror("message length after compression is too long: %zu", bitCount);
                }
                break;
            }
        case 2:
            {
                m_nextSendBuf.insert(m_nextSendBuf.end(), buf, buf + bufSize);
                break;
            }
        case 3:
            {
                m_nextSendBuf.resize(m_nextSendBuf.size() + bufSize + 4);
                const uint32_t bufSizeU32 = bufSize;
                std::memcpy(m_nextSendBuf.data() + bodyStartOff, &bufSizeU32, 4);
                std::memcpy(m_nextSendBuf.data() + bodyStartOff + 4, buf, bufSize);
                break;
            }
        default:
            {
                throw bad_reach();
            }
    }

    if(m_currSendBuf.empty()){
        std::swap(m_currSendBuf, m_nextSendBuf);
        doSendBuf();
    }
}

void NetIO::start(const char *ipStr, const char *portStr, std::function<void(uint8_t, const uint8_t *, size_t)> msgHandler)
{
    fflassert(str_haschar(  ipStr));
    fflassert(str_haschar(portStr));

    fflassert(msgHandler);
    m_msgHandler = std::move(msgHandler);

    asio::async_connect(m_socket, m_resolver.resolve({ipStr, portStr}), [this](std::error_code errCode, asio::ip::tcp::resolver::iterator)
    {
        if(errCode){
            throw fflerror("network error: %s", errCode.message().c_str());
        }
        else{
            doReadHeadCode();
        }
    });
}
