#include "pch.h"

#include "Cartridge.h"
#include "iNES.h"
#include "MMC1.h"
#include "SimpleMapper.h"

Cartridge::Cartridge() {
    SPDLOG_INFO("Cartridge created, but not initialized");
}

void Cartridge::LoadFromPath(const char *path) {
    SPDLOG_INFO("Cartridge loading iNES format from {}", path);
    ines = std::make_unique<iNES>(path);

    switch (ines->GetHeader().GetMapper()) {
        case iNES::Header::MapperType_NROM:
            mapper = std::make_unique<SimpleMapper>();
            break;
        case iNES::Header::MapperType_MMC1:
            mapper = std::make_unique<MMC1>();
            break;
        default:
            VERIFY(false, "Unsupported mapper");
    }

    SPDLOG_INFO("Cartridge loading mapper {}", ines->GetHeader().GetMapper());
    mapper->LoadFromINES(*ines);

    loaded = true;

    SPDLOG_INFO("Cartridge loaded and ready for I/O");
}

void Cartridge::PowerOn() {
    VERIFY(loaded, "Cartridge not loaded");
    SPDLOG_INFO("Cartridge setting power on state");
    mapper->PowerOn();
}

void Cartridge::Reset() {
    VERIFY(loaded, "Cartridge not loaded");
    SPDLOG_INFO("Cartridge resetting");
    mapper->Reset();
}

void Cartridge::Write(uint16_t address, uint8_t value) {
    ASSERT(loaded, "Cartridge not loaded");
    ASSERT(address >= 0x4020, "Cartridge write out of range");
    return mapper->Write(address, value);
}

uint8_t Cartridge::Read(uint16_t address) {
    ASSERT(loaded, "Cartridge not loaded");
    ASSERT(address >= 0x4020, "Cartridge read out of range");
    return mapper->Read(address);
}