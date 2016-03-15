#pragma once

#include <array>
#include <vector>

#include "actorginfo.hpp"
#include "mir2xmapext.hpp"
#include "pngtexoffdb.hpp"

class Actor
{
    public:
        Actor(int, int, int);
        ~Actor();

    protected:
        Mir2xMapExt *m_MapExt;

    public:
        void SetMap(int, int, Mir2xMapExt *);

    protected:
        int m_LID;
        int m_SID;
        int m_UID;
        int m_GenTime;
        int m_X;
        int m_Y;
        int m_HP;

    protected:
        double m_FrameUpdateDelay;

    protected:
        int    m_FrameIndex;
        int    m_Direction;
        int    m_State;
        int    m_NextState;
        double m_UpdateTime;
        int    m_Speed;

    private:
        bool    m_RedPoision;
        bool    m_GreenPoision;
        bool    m_StonePoision;

        bool    m_WhiteBody;
        bool    m_Holy;

    private:
        int     m_EstimateNextX;
        int     m_EstimateNextY;

        int     m_TargetX;
        int     m_targetY;

    public:

        int EstimateNextX()
        {
            return m_EstimateNextX;
        }

        int EstimateNextY()
        {
            return m_EstimateNextY;
        }

    public:
        bool TryStepMove(int);

    public:
        int SID();
        int UID();
        int GenTime();
        int X();
        int Y();
        int ScreenX();
        int ScreenY();

    public:
        int  CalculateDirection(int, int);

        // update the next possible position based on current state
        void EstimateNextPosition(int);

    public:
        virtual int FrameCount() = 0;

    protected:
        int FrameCount(int, int);

    protected:
        void InnDraw(bool, const std::function<void(int, int, uint32_t, uint32_t)> &);

    public:
        virtual void Draw() = 0;
        virtual void Update() = 0;

    public:
        void UpdateCurrentState();
        void UpdateWithNewState();

    protected:
        void UpdateMotion(int);

    public:
        virtual void SetNextState(int);
        virtual void SetNextPosition(int, int);
        virtual void SetPosition(int, int);
        virtual void SetDirection(int);
        virtual void SetState(int);

    public:
        void SetHP(int);

    public:
        virtual void Goto(int, int);

    public:
        static bool GInfoValid(int);
        static bool LoadGInfo(int, const PNGTexOffDB<0> &);

    private:
        static std::unordered_map<int, ActorGInfo> sm_ActorGInfo;
};
