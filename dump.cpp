#include "InstrTable.h"

#include "pch.h"

#include "iNES.h"

#include <filesystem>
#include <cstdint>
#include <iostream>

int main(int argc, char *argv[]) {
    VERIFY(argc == 2, "Usage: {} <rom>", argv[0]);
    iNES ines(argv[1]);
    
    auto prg_rom = ines.GetPrgRom();
    
    size_t pc = 0;
    while (pc < prg_rom.size()) {
        auto opcode = prg_rom[pc];
        auto instrData = InstrDataTable[opcode];
        auto addrData = AddrModeDataTable[instrData.mode];
        switch (addrData.size) {
        case 0:
            fmt::print(fmt::runtime(addrData.fmt), instrData.mnemonic);
            break;
        case 1:
            fmt::print(fmt::runtime(addrData.fmt), instrData.mnemonic, prg_rom[pc + 1]);
            break;
        case 2:
            fmt::print(fmt::runtime(addrData.fmt), instrData.mnemonic, prg_rom[pc + 1], prg_rom[pc + 2]);
            break;
        case 3:
            fmt::print(fmt::runtime(addrData.fmt), instrData.mnemonic, prg_rom[pc + 1], prg_rom[pc + 2], prg_rom[pc + 3]);
            break;
        }
        pc += addrData.size;
    }

    return 0;
}