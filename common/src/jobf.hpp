#pragma once
#include <vector>
#include <string>
#include <inplace_vector>
#include "protocoldef.hpp"

namespace jobf
{
    inline bool jobValid(int job)
    {
        return job && valmaskCheck(JOB_VALMASK, job);
    }

    inline int firstJob(int job)
    {
        /**/ if(job & JOB_WARRIOR) return JOB_WARRIOR;
        else if(job & JOB_TAOIST ) return JOB_TAOIST ;
        else if(job & JOB_WIZARD ) return JOB_WIZARD ;
        else                       return 0;
    }

    inline auto jobName(int job)
    {
        std::inplace_vector<const char8_t *, 3> result;

        if(job & JOB_WARRIOR) result.push_back(u8"战士");
        if(job & JOB_TAOIST ) result.push_back(u8"道士");
        if(job & JOB_WIZARD ) result.push_back(u8"法师");

        return result;
    }

    inline auto jobCapacity(int job, bool english = true)
    {
        std::inplace_vector<const char8_t *, 3> result;

        if(job & JOB_WARRIOR) result.push_back(english ? u8"DC" : u8"攻击");
        if(job & JOB_TAOIST ) result.push_back(english ? u8"SC" : u8"道术");
        if(job & JOB_WIZARD ) result.push_back(english ? u8"MC" : u8"魔法");

        return result;
    }

    inline auto jobGfxIndex(int job)
    {
        std::inplace_vector<int, 3> result;

        if(job & JOB_WARRIOR) result.push_back(0);
        if(job & JOB_TAOIST ) result.push_back(1);
        if(job & JOB_WIZARD ) result.push_back(2);

        return result;
    }

    inline int jobFromGfxIndex(int index)
    {
        switch(index){
            case  0: return JOB_WARRIOR;
            case  1: return JOB_TAOIST ;
            case  2: return JOB_WIZARD ;
            default: return 0;
        }
    }
}
