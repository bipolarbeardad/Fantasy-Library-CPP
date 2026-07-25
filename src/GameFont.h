#pragma once

#include "raylib.h"

#include <string>


bool InitGameFont();

void ShutdownGameFont();


int MeasureGameText(
    const char* text,
    int fontSize
);


int MeasureGameText(
    const std::string& text,
    int fontSize
);


void DrawGameText(
    const char* text,
    int x,
    int y,
    int fontSize,
    Color color
);


void DrawGameText(
    const std::string& text,
    int x,
    int y,
    int fontSize,
    Color color
);