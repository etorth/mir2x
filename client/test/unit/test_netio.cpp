#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>
#include "netio.hpp"

namespace
{
    template<typename Poll, typename Done> void pollUntil(Poll poll, Done done)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while(!done() && std::chrono::steady_clock::now() < deadline){
            poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        REQUIRE(done());
    }
}

TEST_CASE("Connection refusal reports an error once")
{
    asio::io_context io;
    asio::ip::tcp::acceptor reservedPort(io);
    reservedPort.open(asio::ip::tcp::v4());
    reservedPort.bind({asio::ip::address_v4::loopback(), 0});

    NetIO net;
    int errors = 0;
    std::error_code error;
    net.start("127.0.0.1", std::to_string(reservedPort.local_endpoint().port()),
            [](uint8_t, const uint8_t *, size_t, uint64_t){ FAIL("Unexpected message"); },
            [&](std::error_code ec){ ++errors; error = ec; });

    pollUntil([&]{ net.poll(); }, [&]{ return errors != 0; });
    REQUIRE(error == asio::error::connection_refused);
    net.poll();
    REQUIRE(errors == 1);
}

TEST_CASE("Resolution failure reports a connection error")
{
    NetIO net;
    std::error_code error;
    net.start("invalid host name", "12345",
            [](uint8_t, const uint8_t *, size_t, uint64_t){ FAIL("Unexpected message"); },
            [&](std::error_code ec){ error = ec; });

    pollUntil([&]{ net.poll(); }, [&]{ return bool(error); });
}

TEST_CASE("Peer disconnect reports an error after connecting")
{
    asio::io_context io;
    asio::ip::tcp::acceptor listener(io);
    listener.open(asio::ip::tcp::v4());
    listener.bind({asio::ip::address_v4::loopback(), 0});
    listener.listen();
    asio::ip::tcp::socket probe(io);
    std::error_code probeError;
    probe.connect(listener.local_endpoint(), probeError);
    if(probeError){
        SKIP("Direct loopback connection unavailable: " << probeError.message());
    }
    asio::ip::tcp::socket probePeer(io);
    listener.accept(probePeer);
    probe.close();
    probePeer.close();
    asio::ip::tcp::socket peer(io);
    bool accepted = false;
    listener.async_accept(peer, [&](std::error_code ec)
    {
        REQUIRE_FALSE(ec);
        accepted = true;
    });

    NetIO net;
    int errors = 0;
    std::error_code error;
    net.start("127.0.0.1", std::to_string(listener.local_endpoint().port()),
            [](uint8_t, const uint8_t *, size_t, uint64_t){ FAIL("Unexpected message"); },
            [&](std::error_code ec){ ++errors; error = ec; });

    pollUntil([&]{ net.poll(); io.poll(); }, [&]{ return accepted || errors != 0; });
    INFO(error.message());
    REQUIRE(errors == 0);
    REQUIRE(accepted);
    peer.shutdown(asio::ip::tcp::socket::shutdown_both);
    peer.close();
    pollUntil([&]{ net.poll(); }, [&]{ return errors != 0; });
    REQUIRE(error == asio::error::eof);
    net.poll();
    REQUIRE(errors == 1);
}
