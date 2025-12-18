#pragma once
#include "texaniboard.hpp"

class WMDAniBoard: public TexAniBoard
{
    public:
        WMDAniBoard(
                dir8_t argDir,

                int argX,
                int argY,

                Widget * argParent = nullptr,
                bool     argAutoDelete = false)

            : TexAniBoard
              {
                  argDir,
                  argX,
                  argY,

                  0X04000010,
                  10,
                  8,

                  true,
                  true,

                  argParent,
                  argAutoDelete,
              }
        {}
};
