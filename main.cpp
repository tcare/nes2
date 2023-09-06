#include "pch.h"

#include "System.h"

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::trace); 

    VERIFY(argc == 2, "Usage: nes <rom>");

    System system;
    system.LoadCartridge(argv[1]);

    system.PowerOn();
    //system.Init();

    //system.Execute();

    system.Run();

    return 0;
}