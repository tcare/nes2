#include "pch.h"

#include "System.h"

System::System() :
    cpu(mmu),
    mmu(cartridge) {
    SPDLOG_INFO("System created");
}

System::~System() {
    SPDLOG_INFO("System shut down");
}

void System::LoadCartridge(const char *path) {
    VERIFY(!running, "Cannot load cartridge while system is running");
    VERIFY(path != nullptr, "Cannot load cartridge from null path");

    SPDLOG_INFO("System loading cartridge from {}", path);
    cartridge.LoadFromPath(path);
}

void System::Run() {
    SPDLOG_INFO("System running");
    running = true;
    cpu.Run();
}

void System::PowerOn() {
    VERIFY(!running, "Cannot power on system while it is running");
    VERIFY(cartridge.IsLoaded(), "Cannot power on system without a cartridge");

    SPDLOG_INFO("System setting power on state");

    cartridge.PowerOn();
    mmu.PowerOn();
    
    // Power on CPU last since it will implicitly read from the MMU
    cpu.PowerOn();

    SPDLOG_INFO("System powered on");
}

void System::Reset() {
    cartridge.Reset();
    mmu.Reset();

    // Reset CPU last since it will implicitly read from the MMU
    cpu.Reset();
}
