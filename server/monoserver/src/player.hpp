/*
 * =====================================================================================
 *
 *       Filename: player.hpp
 *        Created: 04/08/2016 22:37:01
 *  Last Modified: 04/10/2016 20:52:36
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

#pragma once
#include <SDL2/SDL.h>

#include "monoserver.hpp"
#include "charobject.hpp"

#pragma pack(push, 1)
typedef struct stPLAYERFEATURE{
    uint8_t     Gender;
    uint8_t     Wear;
    uint8_t     Hair;
    uint8_t     Weapon;

    stPLAYERFEATURE()
    {
        std::memset(this, 0, sizeof(*this));
    }
}PLAYERFEATURE;

typedef struct stPLAYERFEATUREEX{
    uint8_t     btHorse;
    SDL_Color   HairColor;
    SDL_Color   WearColor;

    stPLAYERFEATUREEX()
    {
        std::memset(this, 0, sizeof(*this));
    }
}PLAYERFEATUREEX;
#pragma pack(pop)

class Player: public CharObject
{
    private:
        // shortcuts for internal use only
        using ObjectRecord     = std::tuple<uint32_t, uint32_t>;
        using ObjectRecordList = std::list<ObjectRecord>;
        const ObjectRecord EMPTYOBJECTRECORD {0, 0};

    public:
        Player();
        ~Player();

    public:
        virtual bool Friend(const CharObject *);
    public:
        PLAYERFEATURE               m_Feature;
        PLAYERFEATUREEX             m_FeatureEx;

    public:
        // type test function
        virtual bool Type(uint8_t) const;
        virtual bool Mode(uint8_t) const;
        virtual bool Attack(CharObject *);
        virtual bool Follow(CharObject *, bool);
        virtual bool Operate();

    public:
        virtual void SearchViewRange();



    public:
        virtual bool Friend(const CharObject *) const;

    public:
        bool RandomWalk();

        int                         m_CurrX;
        int                         m_CurrY;
        int                         m_Direction;
        int                         m_Event;
};
