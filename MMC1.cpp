#include "MMC1.h"

MMC1::MMC1() {
    SPDLOG_INFO("MMC1 mapper created, but not initialized");
}

void MMC1::LoadFromINES(const iNES &ines) {
    VERIFY(!loaded, "MMC1 mapper already initialized");
    VERIFY(ines.GetHeader().GetMapper() == iNES::Header::MapperType_MMC1, "Only MMC1 mapper is supported");

    // TODO: Load PRG ROM and CHR ROM
    prgRom = &ines.GetPrgRom();

    if (prgRom->size() < 0x4000) {
       SPDLOG_WARN("MMC1 mapper PRG ROM size is less than 16KB, accesses may fail");
    }

    SPDLOG_INFO("MMC1 PRG ROM size {} bytes", prgRom->size());

    SPDLOG_INFO("MMC1 mapper initialized from iNES header and ready for I/O");
    loaded = true;
}

void MMC1::PowerOn() {
}

void MMC1::Reset() {
    ResetControlRegister();
    ResetCHRBank0();
    ResetCHRBank1();
    ResetPRGBank();
    ResetShiftRegister();
}

void MMC1::Write(uint16_t address, uint8_t value) {
    ASSERT(address >= 0x6000 && address <= 0xFFFF, "MMC1 write out of range");
    VERIFY(loaded, "MMC1 mapper not initialized");

    // Check for shift register write
    if (address >= 0x8000 && address <= 0xFFFF) {
        // Shift register reset
        if (value & (1 << 7)) {
            ResetShiftRegister();
        } else {
            // Shift register write
            shiftRegister >>= 1;
            shiftRegister.set(4, value & 0b1);
            // Five bits written. Now the address matters.
            if (shiftRegister.count() == 5) {
                throw std::runtime_error("MMC1::Write() not implemented");

                // Bits 14 and 13 of the address select the register.
                uint8_t reg = (address >> 13) & 0b11;
                // Copy D0 and the SR to a bank register.
                uint8_t bankReg = (shiftRegister.to_ulong() & 0b11110) | (value & 0b1);
                // Write the bank register to the appropriate register.
                
                //  Clear the shift register.
                ResetShiftRegister();
            }
        }
    } else {
        throw std::runtime_error("MMC1::Write() not implemented");
    }

}

uint8_t MMC1::Read(uint16_t address) {
    ASSERT(address >= 0x6000 && address <= 0xFFFF, "MMC1 read out of range");
    VERIFY(loaded, "MMC1 mapper not initialized");

    if (address >= 0x6000 && address <= 0x7FFF) {
        // CPU $6000-$7FFF: 8 KB PRG RAM bank, (optional)
        throw std::runtime_error("MMC1::Read() RAM bank not implemented");
    } else if (address >= 0x8000 && address <= 0xBFFF) {
        // CPU $8000-$BFFF: 16 KB PRG ROM bank, either switchable or fixed to the first bank
        // TODO: implement switching
        auto offset = address - 0x8000;
        auto effectiveAddress = offset;
        VERIFY(effectiveAddress < prgRom->size(), "MMC1 read out of range");
        SPDLOG_TRACE("MMC1 read from address {} effective address {}", address, effectiveAddress);
        return prgRom->at(effectiveAddress);
    } else if (address >= 0xC000 && address <= 0xFFFF) {
        // CPU $C000-$FFFF: 16 KB PRG ROM bank, either fixed to the last bank or switchable
        // TODO: implement switching
        auto last16k = prgRom->size() - 0x4000;
        auto offset = address - 0xC000;
        auto effectiveAddress = last16k + offset;
        VERIFY(effectiveAddress < prgRom->size(), "MMC1 read out of range");
        SPDLOG_TRACE("MMC1 read from address {} effective address {}", address, effectiveAddress);
        return prgRom->at(effectiveAddress);
    } else {
        throw std::runtime_error("MMC1::Read() not implemented");
    }
}

uint8_t MMC1::ReadChr(uint16_t address) {
    return 0;
}

void MMC1::WriteChr(uint16_t address, uint8_t value) {

}
