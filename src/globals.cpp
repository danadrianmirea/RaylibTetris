#include <raylib.h>
#include "globals.h"

Color darkGreen = Color{20, 160, 133, 255};
Color grey = Color{29, 29, 27, 255};
int windowWidth = 1920;
int windowHeight = 1080;
const int gameScreenWidth = 500;
const int gameScreenHeight = 620;

bool exitWindowRequested = false;
bool exitWindow = false;
bool fullscreen = true;
const int minimizeOffset = 50;
float borderOffsetWidth = 20.0;
float borderOffsetHeight = 50.0f;

const Color darkGrey = {26, 31, 40, 255};
const Color green = {47, 230, 23, 255};
const Color red = {232, 18, 18, 255};
const Color orange = {226, 116, 17, 255};
const Color yellow = {237, 234, 4, 255};
const Color purple = {166, 0, 247, 255};
const Color cyan = {21, 204, 209, 255};
const Color blue = {13, 64, 216, 255};
const Color lightBlue = {59, 85, 162, 255};
const Color darkBlue = {44, 44, 127, 255};

std::vector<Color> GetCellColors()
{
    return {darkGrey, green, red, orange, yellow, purple, cyan, blue};
}