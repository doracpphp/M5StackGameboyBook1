#include "cpu.h"
#include "memory.h"
#define set_HL(x) do {uint16_t macro = (x); \
    cpu.L = macro&0xFF; cpu.H = macro>>8;} while(0) 
#define set_DE(x) do {uint16_t macro = (x); \
    cpu.E = macro&0xFF; cpu.D = macro>>8;} while(0)
#define set_BC(x) do {uint16_t macro = (x); \
    cpu.C = macro&0xFF; cpu.B = macro>>8;} while(0)

#define get_HL() (uint16_t)((cpu.H<<8) | cpu.L)
#define get_DE() (uint16_t)((cpu.D<<8) | cpu.E)
#define get_BC() (uint16_t)((cpu.B<<8) | cpu.C)
/* Flags */
#define set_Z(x) cpu.F = ((cpu.F&0x7F) | ((x)<<7))
#define set_N(x) cpu.F = ((cpu.F&0xBF) | ((x)<<6))
#define set_H(x) cpu.F = ((cpu.F&0xDF) | ((x)<<5))
#define set_C(x) cpu.F = ((cpu.F&0xEF) | ((x)<<4))

#define get_flag_C() !!((cpu.F & 0x10))
#define get_flag_Z() !!((cpu.F & 0x80))

void cpu_init() {
    cpu.PC = 0x0000;
    cpu.cycle = 0;
}

// prefix
bool prefix_CB(uint8_t value) {
    uint8_t t;
    switch (value) {
    case 0x11:
        t = get_flag_C();
        set_C(!!(cpu.C & 0x80));
        cpu.C = (cpu.C << 1) | !!(t);
        set_Z(!cpu.C);
        set_N(0);
        set_H(0);
        break;
    case 0x17:
        t = get_flag_C();
        set_C(!!(cpu.A & 0x80));
        cpu.A = (cpu.A << 1) | !!(t);
        set_Z(!cpu.A);
        set_N(0);
        set_H(0);
        break;
    case 0x7C:
        set_Z(!((cpu.H >> 7) & 0x1));
        set_N(0);
        set_H(1);
        break;
    default:
        return false;
        break;
    }
    return true;
}

bool cpu_execute() {
    uint8_t code = get_memory_value(cpu.PC);
    uint16_t hl;
    uint8_t d;

    switch (code) {
    case 0x04:
        set_H((cpu.B & 0xF) == 0xF);
        cpu.B += 1;
        set_Z(!cpu.B);
        set_N(0);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x05:
        cpu.B -= 1;
        set_N(1);
        set_Z(!cpu.B);
        set_H((cpu.B & 0xF) == 0xF);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x06:
        cpu.B = get_memory_value(cpu.PC + 1);
        cpu.PC += 2;
        cpu.cycle = 2;
        break;
    case 0x0C:
        set_N(0);
        set_H((cpu.C & 0xF) == 0xF);
        cpu.C++;
        set_Z(!cpu.C);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x0D:
        set_H((cpu.C & 0xF) == 0);
        cpu.C -= 1;
        set_Z(!cpu.C);
        set_N(1);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x0E:
        cpu.C = get_memory_value(cpu.PC + 1);
        cpu.PC += 2;
        cpu.cycle = 2;
        break;
    case 0x11:
        set_DE(get_memory_d16(cpu.PC + 1));
        cpu.PC += 3;
        cpu.cycle = 3;
        break;
    case 0x13:
        hl = get_DE();
        set_DE(hl + 1);
        cpu.PC += 1;
        cpu.cycle = 2;
        break;
    case 0x15:
        cpu.D -= 1;
        set_N(1);
        set_Z(!cpu.D);
        set_H((cpu.D & 0xF) == 0xF);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x16:
        cpu.D = get_memory_value(cpu.PC + 1);
        cpu.PC += 2;
        cpu.cycle = 2;
        break;
    case 0x17:
        prefix_CB(0x17);
        set_Z(0);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x18:
        cpu.PC += ((signed char)get_memory_value(cpu.PC + 1) + 2);
        cpu.cycle = 3;
        break;
    case 0x1A:
        cpu.A = get_memory_value(get_DE());
        cpu.PC += 1;
        cpu.cycle = 2;
        break;
    case 0x1D:
        set_H((cpu.E & 0xF) == 0);
        cpu.E -= 1;
        set_Z(!cpu.E);
        set_N(1);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x1E:
        cpu.E = get_memory_value(cpu.PC + 1);
        cpu.PC += 2;
        cpu.cycle = 2;
        break;
    case 0x20:
        if ((cpu.F & 0x80) == 0) {
            cpu.PC += ((signed char)get_memory_value(cpu.PC + 1));
            cpu.cycle = 3;
        }
        else {
            cpu.cycle = 2;
        }
        cpu.PC += 2;
        break;
    case 0x21:
        set_HL(get_memory_d16(cpu.PC + 1));
        cpu.PC += 3;
        cpu.cycle = 3;
        break;
    case 0x22:
        hl = get_HL();
        set_memory_value(hl, cpu.A);
        set_HL(hl + 1);
        cpu.PC += 1;
        cpu.cycle = 2;
        break;
    case 0x23:
        hl = get_HL();
        set_HL(hl + 1);
        cpu.PC += 1;
        cpu.cycle = 2;
        break;
    case 0x24:
        set_H((cpu.H & 0xF) == 0xF);
        cpu.H += 1;
        set_Z(!cpu.H);
        set_N(0);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x28:
        if (get_flag_Z() == 1) {
            cpu.PC += (signed char)get_memory_value(cpu.PC + 1) + 2;
            cpu.cycle = 3;
        }
        else {
            cpu.PC += 2;
            cpu.cycle = 2;
        }
        break;
    case 0x2E:
        cpu.L = get_memory_value(cpu.PC + 1);
        cpu.PC += 2;
        cpu.cycle = 2;
        break;
    case 0x31:
        cpu.SP = get_memory_d16(cpu.PC + 1);
        cpu.PC += 3;
        cpu.cycle = 3;
        break;
    case 0x32:
        set_memory_value(get_HL(), cpu.A);
        hl = get_HL();
        set_HL(hl - 1);
        cpu.PC += 1;
        cpu.cycle = 2;
        break;
    case 0x3D:
        cpu.A -= 1;
        set_Z(!cpu.A);
        set_N(1);
        set_H((cpu.A & 0xF) == 0xF);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x3E:
        cpu.A = get_memory_value(cpu.PC + 1);
        cpu.PC += 2;
        cpu.cycle = 2;
        break;
    case 0x4F:
        cpu.C = cpu.A;
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x57:
        cpu.D = cpu.A;
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x67:
        cpu.H = cpu.A;
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x77:
        set_memory_value(get_HL(), cpu.A);
        cpu.PC += 1;
        cpu.cycle = 2;
        break;
    case 0x7C:
        cpu.A = cpu.H;
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
        //LD A,E
    case 0x7B:
        cpu.A = cpu.E;
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0x90:
        set_C((cpu.A - cpu.B) < 0);
        set_H(((cpu.A - cpu.B) & 0xF) > (cpu.A & 0xF));
        cpu.A -= cpu.B;
        set_Z(!cpu.A);
        set_N(1);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0xAF:
        cpu.A = 0;
        cpu.F = 0x80;
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0xBE:
        d = get_memory_value(get_HL());
        set_Z(cpu.A == d);
        set_H(((cpu.A - d) & 0xF) > (cpu.A & 0xF));
        set_N(1);
        set_C((cpu.A - d) < 0);
        cpu.PC += 1;
        cpu.cycle = 1;
        break;
    case 0xC1:
        set_BC(get_memory_d16(cpu.SP));
        cpu.SP += 2;
        cpu.PC += 1;
        cpu.cycle = 3;
        break;
    case 0xC5:
        cpu.SP -= 2;
        set_memory_d16(cpu.SP, get_BC());
        cpu.PC += 1;
        cpu.cycle = 3;
        break;
    case 0xC9:
        cpu.PC = get_memory_d16(cpu.SP);
        cpu.SP += 2;
        cpu.cycle = 3;
        break;
    case 0xCB:
        if (!prefix_CB(get_memory_value(cpu.PC + 1))) {
            return false;
        }
        cpu.PC += 2;
        cpu.cycle = 2;
        break;
    case 0xCD:
        cpu.SP -= 2;
        set_memory_d16(cpu.SP, cpu.PC + 3);
        cpu.PC = get_memory_d16(cpu.PC + 1);
        cpu.cycle = 6;
        break;
    case 0xE0:
        set_memory_value(0xFF00 | get_memory_value(cpu.PC + 1), cpu.A);
        cpu.PC += 2;
        cpu.cycle = 3;
        break;
    case 0xE2:
        set_memory_value(0xFF00 | cpu.C, cpu.A);
        cpu.PC += 1;
        cpu.cycle = 2;
        break;
    case 0xEA:
        set_memory_value(get_memory_d16(cpu.PC + 1), cpu.A);
        cpu.PC += 3;
        cpu.cycle = 4;
        break;
    case 0xF0:
        cpu.A = get_memory_value((0xFF00 | get_memory_value(cpu.PC + 1)));
        cpu.PC += 2;
        cpu.cycle = 3;
        break;
    case 0xFE:
        d = get_memory_value(cpu.PC + 1);
        set_Z(cpu.A == d);
        set_N(1);
        set_C(cpu.A < d);
        set_H(((cpu.A - d) & 0xF) > (cpu.A & 0xF));
        cpu.PC += 2;
        cpu.cycle = 2;
        break;
    default:
        return false;
        break;
    }
    return true;
}
//Debug用
::CPU* getCPU() {
    return &cpu;
}
uint8_t cpu_get_cycle() {
    return cpu.cycle*4;
}
void debug_registers() {
    M5.Lcd.printf("A:0x%02x,B:0x%02x,C:0x%02x,D:0x%02x,E:0x%02x,H:0x%02x,L:0x%02x\nF:0x%02x PC:0x%04x,SP:0x%04x\n",cpu.A,cpu.B,cpu.C,cpu.D,cpu.E,cpu.H,cpu.L,cpu.F,cpu.PC,cpu.SP);
}


