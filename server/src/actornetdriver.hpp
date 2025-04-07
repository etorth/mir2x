#include <cstdint>
#include <functional>
#include <thread>
#include <asio.hpp>
#include "actormsgpack.hpp"

class ActorPool;
class ServerPeer;

class ActorNetDriver
{
    private:
        friend class ActorPool;
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
        std::map<size_t, std::pair<std::string, uint32_t>> m_remotePeerList;

    private:
        size_t m_launchedCount = 0;
        std::vector<std::unique_ptr<PeerSlot>> m_peerSlotList;

    private:
        std::unique_ptr<asio::io_context> m_context;
        std::unique_ptr<asio::ip::tcp::acceptor> m_acceptor;

    private:
        std::thread m_thread;

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
        asio::awaitable<void> readPeerIndex(asio::ip::tcp::socket);

    public:
        void post(size_t, uint64_t, ActorMsgPack);

    public:
        size_t peerIndex() const
        {
            return m_peerIndex.value();
        }

    private:
        void postMaster(ActorMsgPack);
        void postPeer(size_t, ActorMsgPack);

    private:
        void onRemoteMessage(size_t, uint64_t, ActorMsgPack);
        void asyncConnect(size_t, const std::string &, asio::ip::port_type, std::function<void()>);
};
