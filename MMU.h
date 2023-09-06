#pragma once

#include "Cartridge.h"
/*
Address range	Size	Device
$0000–$07FF	$0800	2 KB internal RAM
$0800–$0FFF	$0800	Mirrors of $0000–$07FF
$1000–$17FF	$0800
$1800–$1FFF	$0800
$2000–$2007	$0008	NES PPU registers
$2008–$3FFF	$1FF8	Mirrors of $2000–$2007 (repeats every 8 bytes)
$4000–$4017	$0018	NES APU and I/O registers
$4018–$401F	$0008	APU and I/O functionality that is normally disabled. See CPU Test Mode.
$4020–$FFFF	$BFE0	Cartridge space: PRG ROM, PRG RAM, and mapper registers
*/
class MMU {
public:
    MMU(Cartridge& cartridge);

    void PowerOn();
    void Reset();

    uint8_t Read(Addr address);
    void Write(Addr address, uint8_t value);

private:
    uint8_t& GetAddRef(Addr address);

    uint8_t ram[2048]; // 2KB of RAM   

    uint8_t ppuRegisters[8]; // 8 PPU registers
    uint8_t apuRegisters[24]; // 24 APU registers
    uint8_t disabledRegisters[8]; // 8 disabled registers

    Cartridge& cartridge;
};