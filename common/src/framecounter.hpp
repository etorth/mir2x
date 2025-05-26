#pragma once
#include <cmath>
#include <cstdint>

class FrameCounter
{
    private:
        uint64_t m_seqFrame = 0;

    private:
        double m_fps      = 0.0;
        double m_accuTime = 0.0;

    public:
        FrameCounter(double fps)
            : m_fps((fps > 0.0) ? fps : -1.0)
        {}

    public:
        double fps() const
        {
            return std::max<double>(m_fps, 0.0);
        }

        void setFPS(double fps)
        {
            m_seqFrame = 0;
            m_accuTime = 0.0;

            m_fps = (fps > 0) ? fps : -1.0;
        }

    public:
        bool update(double fUpdateTime)
        {
            if(m_fps > 0.0){
                m_accuTime += fUpdateTime;
                if(m_seqFrame < absFrame()){
                    m_seqFrame++;
                    return true;
                }
            }
            return false;
        }

    public:
        uint64_t absFrame() const
        {
            if(m_fps <= 0.0){
                return 0;
            }
            return static_cast<uint64_t>(std::llround(std::floor(m_accuTime * m_fps)));
        }

        uint64_t seqFrame() const
        {
            return m_seqFrame;
        }
};
