#pragma once
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "protocoldef.hpp"
#include "pngtexoffdb.hpp"

class SelectCharDB: public PNGTexOffDB
{
    public:
        SelectCharDB(size_t resMax)
            : PNGTexOffDB(resMax)
        {}

    public:
        static std::tuple<uint32_t, size_t> retrieveAnimation(int job, bool gender, int animation)
        {
            // using raw index in Interface1c.wil
            // can use this function to re-index all PNGs

            // return frameCount as "max frame count capacity", constantly 20
            // runtime detected first nil texture then loop back

            fflassert(job >= JOB_BEGIN);
            fflassert(job <  JOB_END);

            fflassert(animation >= 0);
            fflassert(animation <  5);

            const auto startIndex = [job, gender]() -> uint32_t
            {
                switch(job){
                    case JOB_WARRIOR: return gender ?  200 :  500;
                    case JOB_TAOIST : return gender ? 1400 : 1700;
                    case JOB_WIZARD : return gender ?  800 : 1100;
                }
                return 0;
            }();

            return
            {
                startIndex + animation * 60,
                20, // frame capacity for animation, real frame count can be less
            };
        }
};
