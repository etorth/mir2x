#include "client.hpp"
#include "sdldevice.hpp"
#include "processconnectionerror.hpp"

extern Client *g_client;
extern SDLDevice *g_sdlDevice;

ProcessConnectionError::ProcessConnectionError()
    : m_board(
            DIR_NONE,
            []{ return std::get<0>(g_sdlDevice->getWindowLogicalSize()) / 2; },
            []{ return std::get<1>(g_sdlDevice->getWindowLogicalSize()) / 2; },
            false)
{
    const auto exit = []{ g_client->requestProcess(PROCESSID_EXIT); };
    m_board.waitChoice(u8"<layout><par>无法连接到服务器</par></layout>", exit, exit);
}

void ProcessConnectionError::draw() const
{
    const SDLDeviceHelper::RenderNewFrame newFrame;
    m_board.drawRoot({});
}

void ProcessConnectionError::update(double elapsed)
{
    m_board.update(elapsed);
}

void ProcessConnectionError::processEvent(const SDL_Event &event)
{
    if(event.type == SDL_EVENT_QUIT){
        g_client->requestProcess(PROCESSID_EXIT);
        return;
    }
    m_board.processEventRoot(event, true, {});
}
