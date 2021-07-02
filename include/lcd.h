#pragma once
#include <cstdint>
#include <M5Stack.h>
#include "memory.h"

// LCD Widthサイズ
#define GAMEBOY_WIDTH 160
// LCD Heightサイズ
#define GAMEBOY_HEIGHT 144
// M5Stackの描画x位置
#define CENTER_X ((320 - GAMEBOY_WIDTH)  >> 1)
// M5Stackの描画y位置
#define CENTER_Y ((240 - GAMEBOY_HEIGHT) >> 1)

// LCDの初期化
void lcd_init();
// LCD描画のアップデート
void update_graphics(int cycle);
// ScanLineの描画
void draw_scanline();
// カラーパレットの色を取得
int get_color(uint8_t color_num, uint16_t address);
// M5StackにBitmap形式で描画
void drawscreen();
