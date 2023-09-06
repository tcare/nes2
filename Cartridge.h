#pragma once

#include "Mapper.h"
#include "iNES.h"

class Cartridge {
public:
    Cartridge();

    void LoadFromPath(const char* path);

    void PowerOn();
    void Reset();

    void Write(uint16_t address, uint8_t value);
    uint8_t Read(uint16_t address);

    bool IsLoaded() const { return loaded; }
private:
    bool loaded = false;
    std::unique_ptr<Mapper> mapper;
    std::unique_ptr<iNES> ines;
};