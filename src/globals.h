#pragma once
#include <raylib.h>
#include <vector>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define WEB_WIDTH 960
#define WEB_HEIGHT 540
#define WEB_SCREEN_SCALE 0.9f

//#define AM_RAY_DEBUG

extern Color darkGreen;
extern Color grey;

extern int windowWidth;
extern int windowHeight;
extern const int gameScreenWidth;
extern const int gameScreenHeight;
extern bool exitWindow;
extern bool exitWindowRequested;
extern bool fullscreen;
extern const int minimizeOffset;
extern float borderOffsetWidth;
extern float borderOffsetHeight;

extern const Color darkGrey;
extern const Color green;
extern const Color red;
extern const Color orange;
extern const Color yellow;
extern const Color purple;
extern const Color cyan;
extern const Color blue;
extern const Color lightBlue;
extern const Color darkBlue;

extern std::vector<Color> GetCellColors();

int GetScreenWidthWrapper();
int GetScreenHeightWrapper();