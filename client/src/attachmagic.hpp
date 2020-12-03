/*
 * =====================================================================================
 *
 *       Filename: attachmagic.hpp
 *        Created: 08/10/2017 12:17:50
 *    Description: For attached magic we don't need its location info
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

#pragma once
#include "magicbase.hpp"
#include "magicrecord.hpp"
class AttachMagic: public MagicBase
{
    protected:
        const int m_direction = DIR_NONE;

    public:
        AttachMagic(int magicID, int magicParam, int magicStage, int direction, double lastTime)
            : MagicBase(magicID, magicParam, magicStage, lastTime)
            , m_direction(direction)
        {
            if(!refreshCache()){
                throw fflerror("failed to refresh cache");
            }

            if(m_cacheEntry->type != EGT_BOUND){
                throw fflerror("magic can't be attached");
            }

            switch(m_cacheEntry->dirType){
                case 8:
                    {
                        if(!(direction >= DIR_BEGIN && direction < DIR_END)){
                            throw fflerror("invalid direction: %d", direction);
                        }
                        break;
                    }
                case 4:
                    {
                        if(!(direction == DIR_UP || direction == DIR_DOWN || direction == DIR_LEFT || direction == DIR_RIGHT)){
                            throw fflerror("invalid direction: %d", direction);
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }

    public:
        AttachMagic(int magicID, int magicParam, int magicStage)
            : AttachMagic(magicID, magicParam, magicStage, DIR_NONE, -1.0)
        {}

        AttachMagic(int magicID, int magicParam, int magicStage, double lastTime)
            : AttachMagic(magicID, magicParam, magicStage, DIR_NONE, lastTime)
        {}

        AttachMagic(int magicID, int magicParam, int magicStage, int direction)
            : AttachMagic(magicID, magicParam, magicStage, direction, -1.0)
        {}

    public:
        void Update(double) override;
        void Draw(int, int) override;

    public:
        bool Done() const;
};
