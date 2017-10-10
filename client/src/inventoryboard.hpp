/*
 * =====================================================================================
 *
 *       Filename: inventoryboard.hpp
 *        Created: 10/08/2017 19:06:52
 *  Last Modified: 10/09/2017 18:47:02
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

#include "widget.hpp"
class InventoryBoard: public Widget
{
    public:
        InventoryBoard();

    public:
        void DrawEx(int, int, int, int, int, int);

    public:
        bool ProcessEvent(const SDL_Event &, bool *);
};
