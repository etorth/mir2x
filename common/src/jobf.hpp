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

    inline const char8_t * jobName(int job)
    {
        switch(job){
            case JOB_WARRIOR: return u8"战士";
            case JOB_TAOIST : return u8"道士";
            case JOB_WIZARD : return u8"法师";
            default         : return nullptr;
        }
    }

    inline std::vector<int> getJobList(const std::string &s)
    {
        std::vector<int> jobList;
        if(s.find(to_cstr(u8"道士")) != std::string::npos){
            jobList.push_back(JOB_TAOIST);
        }

        if(s.find(to_cstr(u8"战士")) != std::string::npos){
            jobList.push_back(JOB_WARRIOR);
        }

        if(s.find(to_cstr(u8"法师")) != std::string::npos){
            jobList.push_back(JOB_WIZARD);
        }
        return jobList;
    }

    inline std::u8string getJobString(const std::vector<int> &jobList)
    {
        if(jobList.empty()){
            throw fflerror("empty job list");
        }

        std::u8string result;
        for(const auto job: jobList){
            if(const auto name = jobName(job); str_haschar(name)){
                if(!result.empty()){
                    result.push_back('|');
                }
                result += name;
            }
            else{
                throw fflerror("invalid job id: %d", job);
            }
        }
        return result;
    }
}
