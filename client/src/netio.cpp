#include "netio.hpp"
#include "totype.hpp"
#include "zcompf.hpp"
#include "message.hpp"
#include "fflerror.hpp"

void NetIO::readHeadCode()
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
                    readHeadCode();
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
                            readBody(smsg.maskLen(), m_readLen[0]);
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
                                readBody(smsg.maskLen(), compSize);
                            });
                        }
                    });
                    return;
                }
            case 2:
                {
                    readBody(0, smsg.dataLen());
                    return;
                }
            case 3:
                {
                    asio::async_read(m_socket, asio::buffer(m_readLen, 4), [this](std::error_code errCode, size_t)
                    {
                        if(errCode){
                            throw fflerror("network error: %s", errCode.message().c_str());
                        }
                        readBody(0, as_u32(m_readLen));
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

void NetIO::readBody(size_t maskSize, size_t bodySize)
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
                fflassert(bitCount == to_d(bodySize));
                fflassert(bodySize <= smsg.dataLen());

                const auto maskDataPtr = m_readBuf.data();
                const auto compDataPtr = m_readBuf.data() + maskSize;
                /* */ auto origDataPtr = m_readBuf.data() + ((maskSize + bodySize + 7) / 8) * 8;

                const auto decodeSize = zcompf::xorDecode(origDataPtr, smsg.dataLen(), maskDataPtr, compDataPtr);
                fflassert(decodeSize == to_d(bodySize));
            }

            m_msgHandler(m_readHeadCode, m_readBuf.data() + (maskSize ? ((maskSize + bodySize + 7) / 8 * 8) : 0), maskSize ? smsg.dataLen() : bodySize);
            readHeadCode();
        });
    }
    else{
        m_msgHandler(m_readHeadCode, nullptr, 0);
        readHeadCode();
    }
}

void NetIO::send(uint8_t headCode, const uint8_t *buf, size_t bufSize)
{
    const ClientMsg cmsg(headCode);
    fflassert(cmsg.checkData(buf, bufSize));

    const bool needFlush = m_sendBuf.empty();
    const auto bodyStartOff = m_sendBuf.size() + 1; // after headCode

    m_sendBuf.push_back(headCode);
    switch(cmsg.type()){
        case 0:
            {
                break;
            }
        case 1:
            {
                const auto bitCount = zcompf::countData(buf, bufSize);
                fflassert(bitCount >= 0);
                fflassert(bitCount <= to_d(cmsg.dataLen()));

                if(bitCount <= 254){
                    m_sendBuf.resize(m_sendBuf.size() + cmsg.maskLen() + to_uz(bitCount) + 1);
                    const auto encodeSize = zcompf::xorEncode(m_sendBuf.data() + bodyStartOff + 1, buf, bufSize);
                    fflassert(encodeSize == bitCount);
                    m_sendBuf[bodyStartOff] = bitCount;
                }
                else if(bitCount <= 255 + 255){
                    m_sendBuf.resize(m_sendBuf.size() + cmsg.maskLen() + to_uz(bitCount) + 2);
                    const auto encodeSize = zcompf::xorEncode(m_sendBuf.data() + bodyStartOff + 2, buf, bufSize);
                    fflassert(encodeSize == bitCount);
                    m_sendBuf[bodyStartOff] = 255;
                    m_sendBuf[bodyStartOff + 1] = to_u8(bitCount - 255);
                }
                else{
                    throw fflerror("message length after compression is too long: %d", bitCount);
                }
                break;
            }
        case 2:
            {
                m_sendBuf.insert(m_sendBuf.end(), buf, buf + bufSize);
                break;
            }
        case 3:
            {
                m_sendBuf.resize(m_sendBuf.size() + bufSize + 4);
                const uint32_t bufSizeU32 = bufSize;
                std::memcpy(m_sendBuf.data() + bodyStartOff, &bufSizeU32, 4);
                std::memcpy(m_sendBuf.data() + bodyStartOff + 4, buf, bufSize);
                break;
            }
        default:
            {
                throw bad_reach();
            }
    }

    if(needFlush){
        m_io.post([this]()
        {
            asio::async_write(m_socket, asio::buffer(m_sendBuf.data(), m_sendBuf.size()), [this](std::error_code errCode, size_t)
            {
                if(errCode){
                    throw fflerror("network error: %s", errCode.message().c_str());
                }
                else{
                    m_sendBuf.clear();
                }
            });
        });
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
            readHeadCode();
        }
    });
}
