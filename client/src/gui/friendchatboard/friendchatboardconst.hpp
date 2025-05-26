#pragma once
constexpr int UIPage_DRAGBORDER[4] // for board resize
{
    10,
    10,
    12,
    10,
};

constexpr int UIPage_BORDER[4]
{
    54,
    10,
    13,
    38,
};

constexpr int UIPage_MARGIN     =   4; // drawing area margin
constexpr int UIPage_MIN_WIDTH  = 400; // the area excludes border area, margin included
constexpr int UIPage_MIN_HEIGHT = 400;

enum UIPageType: int
{
    UIPage_CHAT = 0,
    UIPage_CHATPREVIEW,
    UIPage_FRIENDLIST,
    UIPage_FRIENDSEARCH,
    UIPage_CREATEGROUP,
    UIPage_END,
};
