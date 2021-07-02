//M5StackGameboyBook1 

#include <Arduino.h>
#include <M5Stack.h>
#include "cpu.h"
#include "memory.h"
#include "lcd.h"
int timer;
bool flag = true;
void render(void* arg);

void setup() {
  M5.begin();
  // メモリ初期化
  memory_init();
  // CPU初期化
  cpu_init();
  timer = 0;
  // LCD初期化
  lcd_init();
  // 描画処理をマルチタスク処理させる
  xTaskCreatePinnedToCore(render, "render", 4096, NULL, 5, NULL, 0);
}

void loop() {
  //本来はタイミングを合わせなければいけない
  while(timer < 65536){
    if (!cpu_execute()) {
      flag = false;
    }
    // CPUのサイクルを取得してtimerに加算
    timer += cpu_get_cycle();
    // GBグラフィック更新
    update_graphics(cpu_get_cycle());
  }
  timer=0;
}
// render : M5StackにLCDの内容を描画する。
void render(void* arg){
  while(1){
      drawscreen();
      delay(16);
  }
}