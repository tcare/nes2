#include "SimpleMapper.h"

void SimpleMapper::Write(uint16_t address, uint8_t value) {
    throw std::runtime_error("SimpleMapper::Write: Unhandled address");
}

uint8_t SimpleMapper::Read(uint16_t address) {
    if (address >= 0xC000) {
        return prgRom->at(address - 0xC000);
    } else {
        throw std::runtime_error("SimpleMapper::Read: Unhandled address");
    }
    return 0;
}

uint8_t SimpleMapper::ReadChr(uint16_t address)
{
    return 0;
}

void SimpleMapper::WriteChr(uint16_t address, uint8_t value) {
    
}

void SimpleMapper::PowerOn() {

}

void SimpleMapper::Reset() {

}

void SimpleMapper::LoadFromINES(const iNES &ines) {
    prgRom = &ines.GetPrgRom();
}