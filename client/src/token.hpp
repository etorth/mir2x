// +-------------+------(*: X, Y)
// |             |
// |             v
// |           W1      W       W2
// |     --->|   |<--------->|   |<--
// +-------> +---*-----------+---+  --------
//           |   |  .    .   |   |  ^    ^
//           |   |           |   |  |    |
//           |   |  /\_/\    |   |  |    |
//           |   |=( 0w0 )=  |   |  | H1 | H
//           |   |  )   (  //|   |  |    |
//           |   | (__ __)// |   |  v    |
//           +---+   .       +---+  -    |
//               |    .      |        H2 v
//               +-----------+      --------
//                                  ^
//                                  |

#pragma once
#include <cstdint>
#include <cstddef>

struct TOKEN
{
    int leaf;
    int selectID;

    struct _TokenBox
    {
        struct _TokenBoxInfo
        {
            // general static information
            // keep unchanged after token initialization

            uint16_t    W;
            uint16_t    H;
        }Info;

        struct _TokenBoxState
        {
            // we put mutable attributes here
            // should be valid after token board layout done

            uint16_t    X;
            uint16_t    Y;
            uint16_t    W1;
            uint16_t    W2;
            uint16_t    H1;
            uint16_t    H2;
        }State;
    }Box;

    union
    {
        struct
        {
            uint64_t    U64Key;
        }UTF8Char;

        struct
        {
            uint32_t    U32Key;

            uint8_t     FPS;
            uint8_t     FrameCount;

            uint8_t     Tick;
            uint8_t     Frame;
        }Emoji;

        struct
        {
            uint64_t U64Key;
        }Image;
    };
};
