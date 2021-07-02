#pragma once
#include <M5Stack.h>
//メインメモリの初期化
void memory_init();
//特定の番地の値取得(8bit)
uint8_t get_memory_value(uint16_t addr);
//特定の番地とその次の番地の値取得(16bit)
uint16_t get_memory_d16(uint16_t addr);
//特定の番地の値を設定
void set_memory_value(uint16_t addr, uint8_t value);
void set_lcd_memory(uint16_t addr, uint8_t value);
void set_memory_d16(uint16_t addr, uint16_t value);

