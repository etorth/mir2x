/*
 * =====================================================================================
 *
 *       Filename: fontexdb.h
 *        Created: 02/24/2016 17:51:16
 *  Last Modified: 02/24/2016 17:52:25
 *
 *    Description: 
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


class FontexDB final
{
    private:
        std::tuple<uint8_t, uint8_t, TTF_Font *> m_FontCache[256][5];
}
