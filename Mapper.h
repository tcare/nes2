#pragma once

#include "pch.h"

#include "iNES.h"

class Mapper {
public:
    virtual void LoadFromINES(const iNES& ines) = 0;

    virtual void PowerOn() = 0;
    virtual void Reset() = 0;

    virtual void Write(CPUAddr address, uint8_t value) = 0;
    virtual uint8_t Read(CPUAddr address) = 0;

    virtual uint8_t ReadChr(PPUAddr address) = 0;
    virtual void WriteChr(PPUAddr address, uint8_t value) = 0;

    virtual ~Mapper() {}
};