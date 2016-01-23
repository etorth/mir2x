#pragma once 

class Game
{
    public:
        Game();
        ~Game();

    public:
        void Init();
        void MainLoop();

    public:
        static void StartSystem();

    public:
        enum{
            PROCESSID_NULL   = 0,
            PROCESSID_LOGO   = 1,
            PROCESSID_LOGIN  = 2,
            PROCESSID_RUN    = 3,
            PROCESSID_EXIT   = 4,
        }

    private:
        void SwitchProcess(int, int);
};
