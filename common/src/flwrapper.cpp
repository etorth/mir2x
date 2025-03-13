#include <thread>
#include "flwrapper.hpp"
const static auto s_main_thread_id = std::this_thread::get_id(); // assume globals are created in main thread if static linked

bool fl_wrapper::is_main_thread()
{
    return std::this_thread::get_id() == s_main_thread_id;
}
