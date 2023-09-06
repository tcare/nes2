#pragma once

#include "pch.h"

#include "CPU.h"
#include "MMU.h"
#include "Cartridge.h"

class System {
public:
    System();
    ~System();

    void LoadCartridge(const char* path);
    void Run();

    void PowerOn();
    void Reset();

private:
    CPU cpu;
    MMU mmu;
    //PPU ppu;
    //APU apu;
    Cartridge cartridge;
    //iNES ines;

    bool running = false;
};