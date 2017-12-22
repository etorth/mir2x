list(APPEND THERON_SRCS
    Actor.cpp
    Address.cpp
    AllocatorManager.cpp
    BuildDescriptor.cpp
    Clock.cpp
    DefaultHandlerCollection.cpp
    EndPoint.cpp
    FallbackHandlerCollection.cpp
    Framework.cpp
    HandlerCollection.cpp
    Receiver.cpp
    StringPool.cpp
    YieldPolicy.cpp
)

include_directories(SYSTEM
  ${CMAKE_SOURCE_DIR}/external/include
)

add_library(therond STATIC ${THERON_SRCS})
