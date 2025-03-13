#pragma once
#include <type_traits>

namespace conceptf
{
    template<typename T> concept TriviallyCopyable = std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>;
}
