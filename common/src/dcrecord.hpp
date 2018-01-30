/*
 * =====================================================================================
 *
 *       Filename: dcrecord.hpp
 *        Created: 05/08/2017 16:21:14
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
#include <string>
#include <cstdint>

struct DCRecord
{
    int ID;
    int HP;
    int MP;

    std::string Name;
    std::string Description;

    DCRecord(int nID,
             int nHP,
             int nMP,

             const char *szName,
             const char *szDescription)
        : ID            (nID)
        , HP		    (nHP)
        , MP		    (nMP)
        , Name	        (szName ? szName : "")
        , Description	(szDescription ? szDescription : "")
    {}

    bool Valid() const
    {
        return ID != 0;
    }

    operator bool() const
    {
        return Valid();
    }
};
