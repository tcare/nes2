#include "MMU.h"

MMU::MMU(Cartridge &cartridge) : cartridge(cartridge) {
    SPDLOG_INFO("MMU created, but not initialized");
}

void MMU::PowerOn() {
    SPDLOG_INFO("MMU setting power on state");
}

void MMU::Reset() {
    SPDLOG_INFO("MMU resetting");
}

uint8_t MMU::Read(Addr address) {
    if (address > 0x4020) {
        auto value = cartridge.Read(address);
        SPDLOG_TRACE("MMU read from cartridge address 0x{:X} value 0x{:X}", address, value);
        return value;
    }
    auto value = GetAddRef(address);
    SPDLOG_TRACE("MMU read from RAM address 0x{:X} value 0x{:X}", address, value);
    return value;
}

void MMU::Write(Addr address, uint8_t value) {
    if (address > 0x4020) {
        SPDLOG_TRACE("MMU delegating write to cartridge address 0x{:X} value 0x{:X}", address, value);
        cartridge.Write(address, value);
        return;
    }
    SPDLOG_TRACE("MMU write to RAM address 0x{:X} value 0x{:X}", address, value);
    GetAddRef(address) = value;
}

uint8_t& MMU::GetAddRef(Addr address) {
    // Check for RAM and mirrors
    if (address < 0x2000) {
        auto effectiveAddress = address % 0x800;
        SPDLOG_TRACE("MMU referencing RAM address 0x{:X}, effective address 0x{:X}", address, effectiveAddress);
        return ram[effectiveAddress];
    } else if (address >= 0x2000 && address < 0x4000) {
        auto effectiveRegister = (address - 0x2000) % 8;
        SPDLOG_TRACE("MMU referencing address 0x{:X}, PPU register {}", address, effectiveRegister);
        return ppuRegisters[(address - 0x2000) % 8];
    } else if (address >= 0x4000 && address < 0x4018) {
        auto effectiveRegister = (address - 0x4000) % 24;
        SPDLOG_TRACE("MMU referencing address 0x{:X} to APU or I/O register {}", address, effectiveRegister);
        return apuRegisters[effectiveRegister];
    } else if (address >= 0x4018 && address < 0x4020) {
        auto effectiveRegister = (address - 0x4018) % 8;
        SPDLOG_TRACE("MMU referencing to disabled APU or I/O register {}", address);
        return disabledRegisters[effectiveRegister];
    } else if (address >= 0x4020) {
        VERIFY(false, "MMU trying to get reference to cartridge address {}", address);
        LIBASSERT_UNREACHABLE;
    } else {
        VERIFY(false, "MMU GetAddRef Unreachable {}", address);
        LIBASSERT_UNREACHABLE;
    }
    LIBASSERT_UNREACHABLE;
}
