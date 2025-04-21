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

            uint16_t    w;
            uint16_t    h;
        }info;

        struct _TokenBoxState
        {
            // we put mutable attributes here
            // should be valid after token board layout done

            uint16_t    x;
            uint16_t    y;
            uint16_t    w1;
            uint16_t    w2;
            uint16_t    h1;
            uint16_t    h2;
        }state;
    }box;

    union
    {
        struct
        {
            uint64_t    key;
        }utf8char;

        struct
        {
            uint32_t    key;

            uint8_t     fps;
            uint8_t     frameCount;

            uint8_t     tick;
            uint8_t     frame;
        }emoji;

        struct
        {
            uint64_t key;
        }image;
    };
};
