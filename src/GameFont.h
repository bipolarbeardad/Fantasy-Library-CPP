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


// Script-style font used for special storybook moments such
// as the final "The End" signature page. Falls back to the
// normal game font if Segoe Script is unavailable.
int MeasureScriptText(
    const char* text,
    int fontSize
);


int MeasureScriptText(
    const std::string& text,
    int fontSize
);


void DrawScriptText(
    const char* text,
    int x,
    int y,
    int fontSize,
    Color color
);


void DrawScriptText(
    const std::string& text,
    int x,
    int y,
    int fontSize,
    Color color
);