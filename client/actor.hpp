#pragma once
#include <array>
#include <vector>

typedef struct{
    SDL_Texture *Texture;
    int DX;
    int DY;
}ACTORFRAME;

class ActorFrameSourceManager: public SourceManager<uint32_t, ACTORFRAMEDESC>
{
}

class Actor
{
    public:
        Actor(int, int, int);
        ~Actor();

    protected:
        int m_HP;

    protected:
        Mir2ClientMap *m_Map;

    public:
        void SetMap(int, int, Mir2ClientMap *);

    protected:
        int m_SID;
        int m_UID;
        int m_X;
        int m_Y;

    protected:
        int m_FrameUpdateDelay;

    protected:
        int m_FrameIndex;
        int m_GenTime;
        int m_Direction;
        int m_State;
        int m_NextState;
        int m_UpdateTime;
        int m_Speed;

    public:
        int SID();
        int UID();
        int GenTime();
        int X();
        int Y();
        int ScreenX();
        int ScreenY();

    public:
        int CalculateDirection(int, int);

    public:
        virtual int FrameCount() = 0;

    protected:
        int FrameCount(int, int);

    protected:
        void InnDraw(int, int, int, int, int, int, int);

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
        virtual void DGoto(int, int);

    public:
        // offset cache for drawing
        static void LoadGlobalOffset(int, int);

    public:
        // DRC cache
        static bool GlobalCoverInfoCacheValid(int);
        static bool SetGlobalCoverInfoCache(int, int, int, int, int);
        static int  GlobalCoverInfoCacheIndex(int, int, int);
};
