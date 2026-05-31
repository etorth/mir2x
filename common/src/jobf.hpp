#pragma once
#include <vector>
#include <string>
#include <optional>
#include <inplace_vector>
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
        std::inplace_vector<const char8_t *, 3> result;

        if(job & JOB_WARRIOR) result.push_back(u8"战士");
        if(job & JOB_TAOIST ) result.push_back(u8"道士");
        if(job & JOB_WIZARD ) result.push_back(u8"法师");

        return result;
    }

    inline auto jobGfxIndex(int job)
    {
        std::inplace_vector<std::optional<int>, 3> result;

        if(job & JOB_WARRIOR) result.push_back(0);
        if(job & JOB_TAOIST ) result.push_back(1);
        if(job & JOB_WIZARD ) result.push_back(2);
        if(result.empty()) result.push_back(std::nullopt);

        return result;
    }
}
