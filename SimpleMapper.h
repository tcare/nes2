#pragma once

#include "pch.h"

#include "iNES.h"
#include "Mapper.h"

class SimpleMapper : public Mapper {
public:
    void Write(uint16_t address, uint8_t value);
    uint8_t Read(uint16_t address);

    uint8_t ReadChr(uint16_t address);
    void WriteChr(uint16_t address, uint8_t value);

    void PowerOn();
    void Reset();

    void LoadFromINES(const iNES& ines);
private:
    const RomBank* prgRom;
};