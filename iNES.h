#pragma once

#include "pch.h"

using RomBank = std::vector<uint8_t>;

class iNES {
    const size_t MinRomSize = 16 + 16384;
    const size_t MaxRomSize = 64 * 1024 * 1024;
    const size_t PrgRomChunkSize = 16384;
    const size_t ChrRomChunkSize = 8192;

    static constexpr const char Preamble[4] = { 'N', 'E', 'S', '\x1A' };

public:
    struct Header {
        enum Flags6 : uint8_t {
            Flags6_None                = 0,

            Flags6_IsVerticalMirroring = 1 << 0,
            Flags6_HasPersistentMemory = 1 << 2,
            Flags6_Has512ByteTrainer   = 1 << 3,
            Flags6_HasFourScreenVRAM   = 1 << 4,
            Flags6_MapperLowerNybble   = 0b1111 << 4
        };
        enum Flags7 : uint8_t {
            Flags7_None                = 0,

            Flags7_IsVSUnisystem       = 1 << 0,
            Flags7_HasPlayChoice10Data = 1 << 1,
            Flags7_NES2Format          = 0b11 << 2,
            Flags7_MapperUpperNybble   = 0b1111 << 4,
        };

        enum MapperType : uint8_t {
            MapperType_NROM = 0,
            MapperType_MMC1 = 1
        };

        char name[4];
        uint8_t prgRomChunks;
        uint8_t chrRomChunks;
        uint8_t mapper1;
        uint8_t mapper2;
        uint8_t prgRamSize;
        uint8_t tvSystem1;
        uint8_t tvSystem2;
        char unused[5];

        bool hasTrainer() const {
            return mapper1 & Flags6_Has512ByteTrainer;
        }

        bool HasPlayChoice10Data() const {
            return mapper2 & Flags7_HasPlayChoice10Data;
        }

        MapperType GetMapper() const {
            uint8_t upperNybble = mapper2 & Flags7_MapperUpperNybble;
            uint8_t lowerNybble = (mapper1 & Flags6_MapperLowerNybble) >> 4;
            return static_cast<MapperType>(upperNybble | lowerNybble);
        }
    };

private:
    Header header;
    char trainer[512];
    RomBank prgRom;
    RomBank chrRom;

public:
    iNES(const char *path);
    ~iNES();
    const Header& GetHeader() const { return header; }
    const RomBank& GetPrgRom() const { return prgRom; }
};