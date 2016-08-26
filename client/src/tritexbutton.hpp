/*
 * =====================================================================================
 *
 *       Filename: tritexbutton.hpp
 *        Created: 08/26/2016 13:20:23
 *  Last Modified: 08/26/2016 13:35:24
 *
 *    Description: button with three texture, it has a position shift when
 *                 state changes.
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "buttonbase.hpp"

class TritexButton
{
    private:
    public:
        // use three texture IDs
        TritexButton(
                int                          nX,
                int                          nY,
                uint32_t                     nTexID0,
                uint32_t                     nTexID1,
                uint32_t                     nTexID2,
                int                          nOffsetXOnOver  = 0,
                int                          nOffsetYOnOver  = 0,
                int                          nOffsetXOnClick = 0,
                int                          nOffsetYOnClick = 0,
                const std::function<void()> &fnOnOver        = [](){},
                const std::function<void()> &fnOnClick       = [](){},
                bool                         bOnClickDone    = true,
                Widget                      *pWidget         = nullptr,
                bool                         bFreeWidget     = false)
            : ButtonBase(
                    nX, 
                    nY,
                    nTexID0,
                    nTexID1,
                    nTexID2,
                    fnOnOver,
                    fnOnClick,
                    pWidget,
                    bFreeWidget)
            , m_Offset {{0, 0}, {nOffsetXOnOver, nOffsetYOnOver}, {nOffsetXOnClick, nOffsetYOnClick}}
        {}

        // use one specified texture ID
        TritexButton(
                int                          nX,
                int                          nY,
                uint32_t                     nBaseTexID,
                int                          nOffsetXOnOver  = 0,
                int                          nOffsetYOnOver  = 0,
                int                          nOffsetXOnClick = 0,
                int                          nOffsetYOnClick = 0,
                const std::function<void()> &fnOnOver        = [](){},
                const std::function<void()> &fnOnClick       = [](){},
                bool                         bOnClickDone    = true,
                Widget                      *pWidget         = nullptr,
                bool                         bFreeWidget     = false)
            : TritexButton(nX, nY, nBaseTexID)


};
