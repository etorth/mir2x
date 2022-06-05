#pragma once
#include <tuple>
#include <vector>
#include "raiitimer.hpp"
#include "animation.hpp"

class AnimationDB
{
    private:
        hres_timer m_timer;

    private:
        std::vector<Animation> m_animationList;

    public:
        AnimationDB();

    public:
        Animation *getAnimation();

    public:
        std::tuple<int, int, Fl_Image *> getFrame()
        {
            if(auto aniPtr = getAnimation()){
                const auto aniFrameCount = aniPtr->frameCount();
                fflassert(aniFrameCount > 0);

                return aniPtr->frame((m_timer.diff_msec() / 20) % aniFrameCount);
            }
            return {0, 0, nullptr};
        }
};
