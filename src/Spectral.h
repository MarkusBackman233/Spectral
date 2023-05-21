#pragma once
#include <SDL.h>
#include <stdio.h>
#include <iostream>

void Render();
void verline(int x, int y0, int y1, uint32_t color);


typedef struct Vector2 { int x, y; };