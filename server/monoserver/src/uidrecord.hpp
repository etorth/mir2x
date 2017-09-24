/*
 * =====================================================================================
 *
 *       Filename: uidrecord.hpp
 *        Created: 05/01/2017 11:35:58
 *  Last Modified: 09/23/2017 22:24:43
 *
 *    Description: UID entry won't take care of one specific class
 *                 It's a framework for all classes derived from ServerObject
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

#include "invardata.hpp"
#include "serverobject.hpp"

struct UIDRecord
{
    uint32_t  UID;
    InvarData Desp;

    Theron::Address Address;
    const std::vector<ServerObject::ClassCodeName> &ClassEntry;

    UIDRecord(uint32_t,
            const InvarData &,
            const Theron::Address &,
            const std::vector<ServerObject::ClassCodeName> &);

    bool Valid() const
    {
        return UID;
    }

    operator bool () const
    {
        return Valid();
    }

    void Print() const;

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
