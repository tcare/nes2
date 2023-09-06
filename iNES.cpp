#include "iNES.h"

#include <filesystem>
#include <fstream>

iNES::iNES(const char *path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("File does not exist");
    }

    auto fileSize = std::filesystem::file_size(path);
    if (fileSize < MinRomSize) {
        throw std::runtime_error("File is too small");
    }

    if (fileSize > MaxRomSize) {
        throw std::runtime_error("File is too large");
    }

    auto bytesLeft = fileSize;

    // Read the header
    std::ifstream file(path, std::ios::binary);
    file.read(reinterpret_cast<char *>(&header), sizeof(header));

    VERIFY(file, "Failed to read header");

    bytesLeft -= sizeof(header);

    if (header.hasTrainer()) {
        file.read(reinterpret_cast<char *>(&trainer), sizeof(trainer));
        VERIFY(file, "Failed to read trainer");
        bytesLeft -= sizeof(trainer);
    }

    if (header.prgRomChunks == 0) {
        throw std::runtime_error("PRG ROM size is zero");
    }

    // Read the PRG ROM
    auto prgRomSize = header.prgRomChunks * PrgRomChunkSize;
    prgRom.resize(prgRomSize);
    file.read(reinterpret_cast<char*>(prgRom.data()), prgRomSize);
    VERIFY(file, "Failed to read PRG ROM");
    bytesLeft -= prgRomSize;

    // Read the CHR ROM
    if (header.chrRomChunks > 0) {
        auto chrRomSize = header.chrRomChunks * ChrRomChunkSize;
        chrRom.resize(chrRomSize);
        file.read(reinterpret_cast<char*>(chrRom.data()), chrRomSize);
        VERIFY(file, "Failed to read CHR ROM");
        bytesLeft -= chrRomSize;
    }

    // TODO: Read the PlayChoice-10 data
    if (header.HasPlayChoice10Data()) {
        throw std::runtime_error("PlayChoice-10 data is not supported");
    }
}

iNES::~iNES() {
    SPDLOG_TRACE("iNES destructor");
}
