#include <cinttypes>
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "actorpool.hpp"
#include "netdriver.hpp"
#include "monoserver.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
static thread_local bool t_netThreadFlag = false; // use bool since only has 1 net thread

NetDriver::NetDriver()
    : m_channList(SYS_MAXPLAYERNUM + 1)
{
    // reserve all valid channal IDs
    // don't use the first slot with ID as zero
    for(size_t channID = 1; channID < m_channList.size(); ++channID){
        m_channIDQ.push(channID);
    }
}

NetDriver::~NetDriver()
{
    try{
        doRelease();
    }
    catch(const std::exception &e){
        g_monoServer->addLog(LOGTYPE_WARNING, "Failed when release net driver: %s", e.what());
    }
    catch(...){
        g_monoServer->addLog(LOGTYPE_WARNING, "Failed when release net driver: unknown error");
    }
}

bool NetDriver::isNetThread()
{
    return t_netThreadFlag;
}

void NetDriver::launch(uint32_t port)
{
    fflassert(port > 1024);
    fflassert(g_actorPool->checkUIDValid(uidf::getServiceCoreUID()));

    fflassert(!m_port);
    fflassert(!isNetThread());

    m_port = port;
    m_io = new asio::io_service();
    m_endPoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_port);
    m_acceptor = new asio::ip::tcp::acceptor(*m_io, *m_endPoint);

    fflassert(!m_channIDQ.empty());
    acceptNewConnection();

    m_thread = std::thread([this]()
    {
        t_netThreadFlag = true;

        // net driver shouldn't crash the main loop when exception get thrown in completion handlers
        // see: http://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/reference/io_service.html#boost_asio.reference.io_service.effect_of_exceptions_thrown_from_handlers
        while(true){
            try{
                m_io->run();
                break; // run() exited normally
            }
            catch(const ChannError &e){
                doClose(e.channID());
                g_monoServer->addLog(LOGTYPE_WARNING, "Channel %d disconnected.", to_d(e.channID()));
            }
            // only catch channError
            // let it crash when caught any other exceptions
        }
    });
}

void NetDriver::acceptNewConnection()
{
    if(m_channIDQ.empty()){
        g_monoServer->addLog(LOGTYPE_INFO, "No valid slot for new connection, request ignored.");
        return;
    }

    const auto channID = m_channIDQ.front();
    m_channIDQ.pop();

    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());

    auto &slotRef = m_channList[channID];

    slotRef.sendBuf.clear();
    slotRef.channPtr = std::make_shared<Channel>(m_io, channID, slotRef.lock, slotRef.sendBuf);
    m_acceptor->async_accept(slotRef.channPtr->socket(), [channID, this](std::error_code ec)
    {
        if(ec){
            throw ChannError(channID, "network error when accepting new connection: %s", ec.message().c_str());
        }

        auto channPtr = m_channList.at(channID).channPtr.get();
        g_monoServer->addLog(LOGTYPE_INFO, "Channel %d established for endpoint (%s:%d).", to_d(channPtr->id()), to_cstr(channPtr->ip()), to_d(channPtr->port()));

        channPtr->launch();
        acceptNewConnection();
    });
}

std::array<std::tuple<const uint8_t *, size_t>, 2> NetDriver::encodePostBuf(uint8_t headCode, const void *buf, size_t bufSize, std::vector<uint8_t> &encodeBuf)
{
    const ServerMsg smsg(headCode);
    fflassert(smsg.checkData(buf, bufSize));

    switch(smsg.type()){
        case 0:
            {
                return
                {
                    std::make_tuple(nullptr, 0),
                    std::make_tuple(nullptr, 0),
                };
            }
        case 1:
            {
                // not empty, fixed size, comperssed
                // length encoding:
                // [0 - 254]          : length in 0 ~ 254
                // [    255][0 ~ 255] : length as 0 ~ 255 + 255

                // use 1 or 2 bytes
                // variant data length, support range in [0, 255 + 255]

                encodeBuf.resize(smsg.maskLen() + bufSize + 64);
                /* */ auto compBuf = encodeBuf.data();
                const auto compCnt = zcompf::xorEncode(compBuf + 2, (const uint8_t *)(buf), bufSize);

                fflassert(compCnt >= 0);
                if(compCnt <= 254){
                    compBuf[1] = to_u8(compCnt);
                    return
                    {
                        std::make_tuple(compBuf + 1, 1 + smsg.maskLen() + to_uz(compCnt)),
                        std::make_tuple(nullptr, 0),
                    };
                }
                else if(compCnt <= (255 + 255)){
                    compBuf[0] = 255;
                    compBuf[1] = to_u8(compCnt - 255);
                    return
                    {
                        std::make_tuple(compBuf, 2 + smsg.maskLen() + to_uz(compCnt)),
                        std::make_tuple(nullptr, 0),
                    };
                }
                else{
                    throw fflerror("message length after compression is too long: %d", compCnt);
                }
            }
        case 2:
            {
                // not empty, fixed size, not compressed
                return
                {
                    std::make_tuple((const uint8_t *)(buf), bufSize),
                    std::make_tuple(nullptr, 0),

                };
            }
        case 3:
            {
                // not empty, not fixed size, not compressed
                encodeBuf.resize(4);
                const uint32_t bufSizeU32 = to_u32(bufSize);
                std::memcpy(encodeBuf.data(), &bufSizeU32, 4);
                return
                {
                    std::make_tuple(encodeBuf.data(), 4),
                    std::make_tuple((const uint8_t *)(buf), bufSize),
                };
            }
        default:
            {
                throw bad_reach();
            }
    }
}

void NetDriver::post(uint32_t channID, uint8_t headCode, const void *buf, size_t bufLen)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());
    fflassert(ServerMsg(headCode).checkData(buf, bufLen));

    // post current message to sendBuf, which links to Channel::m_nextSendQ
    // this function is called by server thread only
    //
    // current implementation is based on double-queue method
    // one queue (Q1) is used for store new packages in parallel
    // the other (Q2) queue is used to post all packages in ASIO main loop
    //
    // then Q1 needs to be protected from data race
    // but for Q2 since it's only used in ASIO main loop, we don't need to protect it
    //
    // idea from: https://stackoverflow.com/questions/4029448/thread-safety-for-stl-queue
    auto &slotRef = m_channList[channID];
    const auto encodedBufList = encodePostBuf(headCode, buf, bufLen, slotRef.encodeBuf);
    {
        std::lock_guard<std::mutex> lockGuard(slotRef.lock);
        slotRef.sendBuf.push_back(headCode);

        for(const auto &[encodedBuf, encodedBufSize]: encodedBufList){
            if(encodedBuf){
                slotRef.sendBuf.insert(slotRef.sendBuf.end(), encodedBuf, encodedBuf + encodedBufSize);
            }
        }
    }

    // when asio thread finish send all pending data it doesn't poll new packs
    // so every time when there is new packs, need to trigger the send, otherwise data stay in the buffer
    m_io->post([channID, this]()
    {
        if(auto channPtr = m_channList[channID].channPtr.get()){
            channPtr->flushSendQ();
        }
        else{
            // channel has been released, ignore the post request
            // the corresponding actor should aleady or shall get AM_BADCHANNEL by Channel::dtor
            // note even channPtr is empty, the dtor may not get called yet because of the shared_from_this() capture
        }
    });
}

void NetDriver::bindPlayer(uint32_t channID, uint64_t uid)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());
    fflassert(uidf::isPlayer(uid));

    m_io->post([uid, channID, this]()
    {
        // use post rather than directly assignement since asio thread accesses m_playerUID
        // potential bug is it's done by post so actor uid is not updated immediately after this call

        if(auto channPtr = m_channList[channID].channPtr.get()){
            channPtr->bindPlayer(uid);
        }
    });
}

void NetDriver::close(uint32_t channID)
{
    logProfiler();
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());

    // if actor thread would initialize a shutdown to a channel
    // it should call this function to schedule a shutdown event via m_io->post()

    // after this function call, the channel slot can still be not empty
    // player actor should keep a flag(m_channID.has_value() && m_channID.value() == 1) to indicate it shall not post any new message

    m_io->post([channID, this]()
    {
        // TODO shall I add any validation to confirm that only the bind player UID can close the channel?
        //      otherwise a careless call to NetDriver::close() with random channID can crash other player's connection
        doClose(channID);
    });
}

void NetDriver::doRelease()
{
    if(m_io){
        m_io->stop();
    }

    if(m_thread.joinable()){
        m_thread.join();
    }

    delete m_acceptor;
    delete m_endPoint;
    delete m_io;

    m_acceptor = nullptr;
    m_endPoint = nullptr;
    m_io       = nullptr;
}

void NetDriver::doClose(uint32_t channID)
{
    fflassert(to_uz(channID) > 0);
    fflassert(to_uz(channID) < m_channList.size());
    fflassert(isNetThread());

    if(m_channList[channID].channPtr){
        m_channList[channID].channPtr.reset();
        m_channIDQ.push(channID);
    }
}
