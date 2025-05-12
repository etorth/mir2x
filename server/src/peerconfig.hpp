#include <mutex>
#include "serdesmsg.hpp"

class PeerConfig
{
    private:
        mutable std::mutex m_lock;

    private:
        SDPeerConfig m_sdPC;

    public:
        PeerConfig() = default;

    public:
        void setConfig(SDPeerConfig sdPC)
        {
            const std::lock_guard<std::mutex> lock(m_lock);
            m_sdPC = std::move(sdPC);
        }

        SDPeerConfig getConfig() const
        {
            const std::lock_guard<std::mutex> lock(m_lock);
            return m_sdPC;
        }
};
