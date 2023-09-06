#pragma once

#include "iNES.h"
#include "Mapper.h"

// MMC1 internal registers are 5-bit wide
using Reg5 = std::bitset<5>;

// MMC1 Banks

// PPU $0000-$0FFF: 4 KB switchable CHR bank
// PPU $1000-$1FFF: 4 KB switchable CHR bank


class MMC1 : public Mapper {
public:
    MMC1();
    virtual ~MMC1() = default;

    void LoadFromINES(const iNES& ines);

    void PowerOn();
    void Reset();

    void Write(uint16_t address, uint8_t value);
    uint8_t Read(uint16_t address);

    uint8_t ReadChr(uint16_t address);
    void WriteChr(uint16_t address, uint8_t value);

private:
    bool loaded = false;

    constexpr static Reg5 SHIFTREG_DEFAULT_VALUE = Reg5{0b10000}; 
    Reg5 shiftRegister{SHIFTREG_DEFAULT_VALUE};
    void ResetShiftRegister() { shiftRegister = SHIFTREG_DEFAULT_VALUE; }

    constexpr static Reg5 CONTROL_REGISTER_DEFAULT_VALUE = Reg5{0b10000};
    Reg5 controlRegister{CONTROL_REGISTER_DEFAULT_VALUE};
    void ResetControlRegister() { controlRegister = CONTROL_REGISTER_DEFAULT_VALUE; }

    constexpr static Reg5 CHR_BANK_0_DEFAULT_VALUE = Reg5{0b00000};
    Reg5 chrBank0{CHR_BANK_0_DEFAULT_VALUE};
    void ResetCHRBank0() { chrBank0 = CHR_BANK_0_DEFAULT_VALUE; }

    constexpr static Reg5 CHR_BANK_1_DEFAULT_VALUE = Reg5{0b00000};
    Reg5 chrBank1{CHR_BANK_1_DEFAULT_VALUE};
    void ResetCHRBank1() { chrBank1 = CHR_BANK_1_DEFAULT_VALUE; }

    constexpr static Reg5 PRG_BANK_DEFAULT_VALUE = Reg5{0b00000};
    Reg5 prgBank{PRG_BANK_DEFAULT_VALUE};
    void ResetPRGBank() { prgBank = PRG_BANK_DEFAULT_VALUE; }
   
    const RomBank* prgRom;
};