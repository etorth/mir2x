#pragma once 
#include "process.hpp"
#include "processrun.hpp"
#include "processlogo.hpp"
#include "processsyrc.hpp"
#include "processlogin.hpp"

class Game
{
    public:
        Game();
        ~Game();

    public:
        void Init();
        void MainLoop();
        void Clear();

    public:
        static void StartSystem();

    private:
        void SwitchProcess(int, int);

    private:
        ProcessLogo     m_ProcessLogo;
        ProcessSyrc     m_ProcessSyrc;
        ProcessLogin    m_ProcessLogin;
        ProcessRun      m_ProcessRun;

    public:
        ProcessRun      *OnProcessRun();

    private:
        Process        *m_CurrentProcess;
};
