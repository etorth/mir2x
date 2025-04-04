#include <cstdint>
#include <functional>
#include <thread>
#include <asio.hpp>
#include "actormsgpack.hpp"

class ServerPeer;
class ActorNetDriver
{
    private:
        friend class ServerPeer;

    private:
        struct PeerSlot
        {
            std::mutex lock;
            std::vector<char> sendBuf;
            std::shared_ptr<ServerPeer> peer;
        };

    private:
        std::optional<size_t> m_peerIndex;

    private:
        std::vector<std::unique_ptr<PeerSlot>> m_peerSlotList;

    private:
        std::unique_ptr<asio::io_context> m_context;
        std::unique_ptr<asio::ip::tcp::acceptor> m_acceptor;

    private:
        std::thread m_thread;

    private:
        std::string m_sendBuf; // scratch buffer

    public:
        ActorNetDriver();

    public:
        ~ActorNetDriver();

    private:
        void launch(asio::ip::port_type);

    public:
        size_t hasPeer() const;

    private:
        void doRelease();
        void doClose(uint32_t);

    private:
        void close(uint32_t);

    public:
        void closeAcceptor();

    public:
        static bool isNetThread();

    public:
        asio::awaitable<void> listener();

    public:
        void post(size_t, uint64_t, ActorMsgPack);

    public:
        size_t peerIndex() const
        {
            return m_peerIndex.value();
        }

    private:
        void postMaster(ActorMsgPack);

    private:
        void onRemoteMessage(uint64_t, ActorMsgPack);
        void asyncConnect(size_t, const std::string &, asio::ip::port_type, std::function<void()>);
};
