/*
 * =====================================================================================
 *
 *       Filename: uidrecord.hpp
 *        Created: 05/01/2017 11:35:58
 *  Last Modified: 05/03/2017 22:40:35
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
#include <vector>
#include <cstdint>
#include <Theron/Address.h>
#include "serverobject.hpp"

struct UIDRecord
{
    uint32_t UID;
    Theron::Address Address;
    const std::vector<ServerObject::ClassCodeName> &ClassEntry;

    UIDRecord(uint32_t,
            Theron::Address,
            const std::vector<ServerObject::ClassCodeName> &);

    bool Valid()
    {
        return UID;
    }

    operator bool ()
    {
        return Valid();
    }

    void Print();

    template<typename T> bool ClassFrom()
    {
        if(Valid()){
            for(const auto &rstCodeName: ClassEntry){
                if(true
                        && rstCodeName.Name == typeid(T).name()
                        && rstCodeName.Code == typeid(T).hash_code()){ return true; }
            }
        }
        return false;
    }
};
