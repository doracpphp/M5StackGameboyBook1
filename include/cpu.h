#pragma once
#include <memory.h>
#include <M5Stack.h>
struct CPU {
    //registers
    uint8_t A, B, C, D, E, H, L, F;
    //stack pointer, program counter
    uint16_t SP, PC;
    uint16_t cycle;
};
static struct CPU cpu;

void cpu_init();
bool cpu_execute();
uint8_t cpu_get_cycle();
::CPU* getCPU();
void debug_registers();
