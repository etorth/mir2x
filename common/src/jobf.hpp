#pragma once
#include <vector>
#include <string>
#include "totype.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"

namespace jobf
{
    inline bool jobValid(int job)
    {
        return job >= JOB_BEGIN && job < JOB_END;
    }

    inline int firstJob(int job)
    {
        /**/ if(job & JOB_WARRIOR) return JOB_WARRIOR;
        else if(job & JOB_TAOIST ) return JOB_TAOIST ;
        else if(job & JOB_WIZARD ) return JOB_WIZARD ;
        else                       return JOB_NONE   ;
    }

    inline auto jobName(int job)
    {
        size_t i = 0;
        std::array<const char8_t *, 3> result {};

        if(job & JOB_WARRIOR) result[i++] = u8"战士";
        if(job & JOB_TAOIST ) result[i++] = u8"道士";
        if(job & JOB_WIZARD ) result[i++] = u8"法师";

        return result;
    }

    inline auto jobGfxIndex(int job)
    {
        size_t i = 0;
        std::array<std::optional<int>, 3> result {};

        if(job & JOB_WARRIOR) result[i++] = 0;
        if(job & JOB_TAOIST ) result[i++] = 1;
        if(job & JOB_WIZARD ) result[i++] = 2;

        return result;
    }
}
