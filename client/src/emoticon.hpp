#pragma once

// How the frame will be shown
// use EMOTICONTEXTUREINDICATOR and EMOTICONFRAMEINFO to show one frame
typedef struct{
    int     X;
    int     Y;
    int     W;
    int     H;
    int     DX;
    int     DY;
}EMOTICONFRAMEINFO;

// we put this info in the cache
// but also put a copy of it in the EMOTICONBOXHANDLER
typedef struct{
    int     W;
    int     H;
    int     H1;
    int     H2;
    int     FPS;
    int     Method;
}EMOTICONINFO;
