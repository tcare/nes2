#include "CPU.h"
// https://www.nesdev.org/wiki/CPU_power_up_state#At_power-up
void CPU::PowerOn() {
    SPDLOG_INFO("CPU power up");
    P = 0x34; // IRQ Disabled
    A = 0x00;
    X = 0x00;
    Y = 0x00;
    S = 0xFD;
    mmu.Write(0x4017, 0x00);
    mmu.Write(0x4015, 0x00);
    for (Addr a = 0x4000; a <= 0x400F; ++a) {
        mmu.Write(a, 0x00);
    }
    for (Addr a = 0x4010; a < 0x4013; ++a) {
        mmu.Write(a, 0x00);
    }
    ReadResetVector();
}
// https://www.nesdev.org/wiki/CPU_power_up_state#After_reset
void CPU::Reset() {
    S -= 3;
    P |= 0x04;
    ReadResetVector();
}
void CPU::Execute() {
    // Read opcode
    auto opcode = mmu.Read(PC);
    SPDLOG_TRACE("PC = {:#x}", PC);

    // Read addressing mode
    auto addrMode = InstrDataTable[opcode].mode;

    VERIFY(!InstrDataTable[opcode].illegal);

    // Read operand
    auto operandSize = AddrModeDataTable[addrMode].size;
    uint8_t operand = 0;
    uint16_t operand16 = 0;
    switch (addrMode) {
    case Addr_Implicit:
        // No operand
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic);
        break;
    case Addr_Accumulator:
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic);
        operand = A;
        break;
    case Addr_Immediate:
        // Operand is immediately after PC
        operand = mmu.Read(PC);
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic, operand);
        break;

    case Addr_ZeroPage: {
        // Operand address is immediately after
        auto operandAddress = mmu.Read(PC);
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic, operandAddress);
        operand = mmu.Read(operandAddress);
        }
        break;

    case Addr_Absolute: {
        // Operand is next 16 bits after opcode
        auto addrUpper = mmu.Read(PC+1) << 8;
        auto addrLower = mmu.Read(PC+2);
        auto effectiveAddr = addrUpper | addrLower;
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic, addrLower, addrUpper);
        operand = mmu.Read(effectiveAddr);
        }
        break;

    case Addr_Relative: {
        // 8 bit signed offset relative to PC
        operand = PC;
        int8_t offset = mmu.Read(PC);
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic, offset);
        operand += offset;
        }
        break;

    case Addr_Indirect: {
        // Operand is at address in the next 16 bits after opcode
        auto indirectUpper = mmu.Read(PC+1) << 8;
        auto indirectLower = mmu.Read(PC+2);
        auto indirectAddr = indirectUpper | indirectLower;
        auto derefUpper = mmu.Read(indirectAddr) << 8;
        auto derefLower = mmu.Read(indirectAddr+1);
        auto derefAddr = derefUpper | derefLower;
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic);
        operand = mmu.Read(derefAddr);
        }
        break;

    // Indexed addressing modes
    case Addr_ZeroPageX: {  // Zero page indexed, val = PEEK((arg + X) % 256)
        auto zpxAddr = mmu.Read(PC);
        auto zpxEffectiveAddr = (zpxAddr + X) % 256;
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic);
        operand = mmu.Read(zpxEffectiveAddr);
        }
        break;
    
    case Addr_ZeroPageY: {   // Zero page indexed, val = PEEK((arg + Y) % 256)
        auto zpyAddr = mmu.Read(PC);
        auto zpyEffectiveAddr = (zpyAddr + Y) % 256;
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic);
        operand = mmu.Read(zpyEffectiveAddr);
        }
        break;

    case Addr_AbslX: {       // Absolute indexed, val = PEEK(arg + X)
        auto abslXAddr = mmu.Read(PC) + X;
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic, abslXAddr);
        operand = mmu.Read(abslXAddr);
        }
        break;

    case Addr_AbslY: {      // Absolute indexed, val = PEEK(arg + Y)
        auto abslYAddr = mmu.Read(PC) + Y;
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic, abslYAddr);
        operand = mmu.Read(abslYAddr);
        }
        break;

    case Addr_IndirX: {      // Indexed indirect, val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X + 1) % 256) * 256)
        auto indirXAddr = mmu.Read(PC) + X;
        auto indirXEffectiveAddr = (indirXAddr + X) % 256;
        auto indirXUpper = mmu.Read(indirXEffectiveAddr) << 8;
        auto indirXLower = mmu.Read(indirXEffectiveAddr+1);
        auto derefXAddr = indirXUpper | indirXLower;
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic);
        operand = mmu.Read(derefXAddr);
        }
        break;

    case Addr_IndirY: {     // Indexed indirect, val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256) * 256 + Y)
        auto indirYAddr = mmu.Read(PC);
        auto indirYUpper = mmu.Read(indirYAddr) << 8;
        auto indirYLower = mmu.Read(indirYAddr+1);
        auto derefYAddr = indirYUpper | indirYLower;
        auto indirYEffectiveAddr = derefYAddr + Y;
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic);
        operand = mmu.Read(indirYEffectiveAddr);
        }
        break;
    
    case Addr_Illegal:
        SPDLOG_TRACE(fmt::runtime(AddrModeDataTable[addrMode].fmt), InstrDataTable[opcode].mnemonic);
        throw std::runtime_error("Illegal addressing mode");
        break;
    }


    // Execute instruction
    switch (opcode) {
    // ADC - Add with Carry
    case OP_ADC_IMM:
    case OP_ADC_ZP:
    case OP_ADC_ZPX:
    case OP_ADC_ABS:
    case OP_ADC_ABSX:
    case OP_ADC_ABSY:
    case OP_ADC_INDX:
    case OP_ADC_INDY:

        break;

    // AND - Logical AND
    case OP_AND_IMM:
    case OP_AND_ZP:
    case OP_AND_ZPX:
    case OP_AND_ABS:
    case OP_AND_ABSX:
    case OP_AND_ABSY:
    case OP_AND_INDX:
    case OP_AND_INDY: {
        uint16_t result = A + operand + P.test(Flag_Carry);
        P.set(Flag_Negative, result & NEGATIVE_BIT);
        P.set(Flag_Overflow, (A ^ result) & (operand ^ result) & NEGATIVE_BIT);
        A = result & 0xFF;
        P.set(Flag_Zero, A == 0);
        break;
    }

    // ASL - Arithmetic Shift Left
    case OP_ASL_ACC:
    case OP_ASL_ZP:
    case OP_ASL_ZPX:
    case OP_ASL_ABS:
    case OP_ASL_ABSX:
        operand <<= 1;
        P.set(Flag_Carry, operand & CARRY_BIT);
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
        break;

    // BCC - Branch if Carry Clear
    case OP_BCC_REL:
        if (!P.test(Flag_Carry))
            PC += operand;
        break;
    
    // BCS - Branch if Carry Set
    case OP_BCS_REL:
        if (P.test(Flag_Carry))
            PC = operand16;
        break;
    
    // BEQ - Branch if Equal
    case OP_BEQ_REL:
        if (P.test(Flag_Zero))
            PC = operand16;
        break;

    // BMI - Branch if Minus
    case OP_BMI_REL:
        if (P.test(Flag_Negative))
            PC = operand16;
        break;

    // BNE - Branch if Not Equal
    case OP_BNE_REL:
        if (!P.test(Flag_Zero))
            PC = operand16;
        break;

    // BPL - Branch if Positive
    case OP_BPL_REL:
        if (!P.test(Flag_Negative))
            PC = operand16;
        break;

    // BVC - Branch if Overflow Clear
    case OP_BVC_REL:
        if (!P.test(Flag_Overflow))
            PC = operand16;
        break;

    // BVS - Branch if Overflow Set
    case OP_BVS_REL:
        if (P.test(Flag_Overflow))
            PC = operand16;
        break;

    // BIT - Bit Test
    case OP_BIT_ZP:
    case OP_BIT_ABS:
        operand &= A;
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
        P.set(Flag_Overflow, operand & OVERFLOW_BIT);
        P.set(Flag_Zero, operand == 0);
        break;

    // BRK - Force Interrupt
    case OP_BRK_IMP:
        Push(PC);
        Push(P.to_ulong());
        P.set(Flag_InterruptDisable);
        PC = Addr_BRK;
        break;

    // CLC - Clear Carry Flag
    case OP_CLC_IMP:
        P.reset(Flag_Carry);
        break;

    // CLD - Clear Decimal Mode
    case OP_CLD_IMP:
        P.reset(Flag_Decimal);
        break;

    // CLI - Clear Interrupt Disable
    case OP_CLI_IMP:
        P.reset(Flag_InterruptDisable);
        break;

    // CLV - Clear Overflow Flag
    case OP_CLV_IMP:
        P.reset(Flag_Overflow);
        break;

    // NOP - No Operation
    case OP_NOP_IMP:
        break;

    // PHA - Push Accumulator
    case OP_PHA_IMP:
        Push(A);
        break;

    // PLA - Pull Accumulator
    case OP_PLA_IMP:
        A = Pop();
        break;

    // PHP - Push Processor Status
    case OP_PHP_IMP:
        Push(P.to_ulong());
        break;
    
    // PLP - Pull Processor Status
    case OP_PLP_IMP:
        P = Pop();
        break;
    
    // RTI - Return from Interrupt
    case OP_RTI_IMP:
        P = Pop();
        PC = Pop();
        break;

    // RTS - Return from Subroutine
    case OP_RTS_IMP:
        PC = PopAddr() + 1;
        break;

    // SEC - Set Carry Flag
    case OP_SEC_IMP:
        P.set(Flag_Carry);
        break;

    // SED - Set Decimal Flag
    case OP_SED_IMP:
        P.set(Flag_Decimal);
        break;

    // SEI - Set Interrupt Disable
    case OP_SEI_IMP:
        P.set(Flag_InterruptDisable);
        break;

    // TAX - Transfer Accumulator to X
    case OP_TAX_IMP:
        X = A;
        break;

    // TXA - Transfer X to Accumulator
    case OP_TXA_IMP:
        A = X;
        break;

    // TAY - Transfer Accumulator to Y
    case OP_TAY_IMP:
        Y = A;
        break;

    // TYA - Transfer Y to Accumulator
    case OP_TYA_IMP:
        A = Y;
        break;

    // TSX - Transfer Stack Pointer to X
    case OP_TSX_IMP:
        X = S;
        break;

    // TXS - Transfer X to Stack Pointer
    case OP_TXS_IMP:
        S = X;
        break;

    // CMP - Compare
    case OP_CMP_IMM:
    case OP_CMP_ZP:
    case OP_CMP_ZPX:
    case OP_CMP_ABS:
    case OP_CMP_ABSX:
    case OP_CMP_ABSY:
    case OP_CMP_INDX:
    case OP_CMP_INDY:
        P.set(Flag_Carry, A >= operand);
        P.set(Flag_Zero, A == operand);
        P.set(Flag_Negative, (A - operand) & NEGATIVE_BIT);
        break;

    // CPX - Compare X Register
    case OP_CPX_IMM:
    case OP_CPX_ZP:
    case OP_CPX_ABS:
        P.set(Flag_Carry, X >= operand);
        P.set(Flag_Zero, X == operand);
        P.set(Flag_Negative, (X - operand) & NEGATIVE_BIT);
        break;

    // CPY - Compare Y Register
    case OP_CPY_IMM:
    case OP_CPY_ZP:
    case OP_CPY_ABS:
        P.set(Flag_Carry, Y >= operand);
        P.set(Flag_Zero, Y == operand);
        P.set(Flag_Negative, (Y - operand) & NEGATIVE_BIT);
        break;

    // DEC - Decrement Memory
    case OP_DEC_ZP:
    case OP_DEC_ZPX:
    case OP_DEC_ABS:
    case OP_DEC_ABSX:
        --operand;
        P.set(Flag_Zero, operand == 0);
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
        mmu.Write(operand16, operand);
        break;

    // DEX - Decrement X Register
    case OP_DEX_IMP:
        X--;
        P.set(Flag_Zero, X == 0);
        P.set(Flag_Negative, X & NEGATIVE_BIT);
        break;

    // DEY - Decrement Y Register
    case OP_DEY_IMP:
        Y--;
        P.set(Flag_Zero, Y == 0);
        P.set(Flag_Negative, Y & NEGATIVE_BIT);
        break;
    
    // INX - Increment X Register
    case OP_INX_IMP:
        X++;
        P.set(Flag_Zero, X == 0);
        P.set(Flag_Negative, X & NEGATIVE_BIT);
        break;

    // INY - Increment Y Register
    case OP_INY_IMP:
        Y++;
        P.set(Flag_Zero, Y == 0);
        P.set(Flag_Negative, Y & NEGATIVE_BIT);
        break;
    
    // EOR - Exclusive OR
    case OP_EOR_IMM:
    case OP_EOR_ZP:
    case OP_EOR_ZPX:
    case OP_EOR_ABS:
    case OP_EOR_ABSX:
    case OP_EOR_ABSY:
    case OP_EOR_INDX:
    case OP_EOR_INDY:
        A ^= operand;
        P.set(Flag_Zero, A == 0);
        P.set(Flag_Negative, A & NEGATIVE_BIT);
        break;

    // INC - Increment Memory
    case OP_INC_ZP:
    case OP_INC_ZPX:
    case OP_INC_ABS:
    case OP_INC_ABSX:
        ++operand;
        P.set(Flag_Zero, operand == 0);
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
        mmu.Write(operand16, operand);
        break;

    // JMP - Jump
    case OP_JMP_ABS:
    case OP_JMP_IND:
        PC = operand16;
        break;

    // JSR - Jump to Subroutine
    case OP_JSR_ABS:
        Push(PC);
        PC = operand16;
        break;
    
    // LDA - Load Accumulator
    case OP_LDA_IMM:
    case OP_LDA_ZP:
    case OP_LDA_ZPX:
    case OP_LDA_ABS:
    case OP_LDA_ABSX:
    case OP_LDA_ABSY:
    case OP_LDA_INDX:
    case OP_LDA_INDY:
        A = operand;
        P.set(Flag_Negative, A & NEGATIVE_BIT);
        P.set(Flag_Zero, A == 0);
        break;
    
    // LDX - Load X Register
    case OP_LDX_IMM:
    case OP_LDX_ZP:
    case OP_LDX_ZPY:
    case OP_LDX_ABS:
    case OP_LDX_ABSY:
        X = operand;
        P.set(Flag_Negative, X & NEGATIVE_BIT);
        P.set(Flag_Zero, X == 0);
        break;
    
    // LDY - Load Y Register
    case OP_LDY_IMM:
    case OP_LDY_ZP:
    case OP_LDY_ZPX:
    case OP_LDY_ABS:
    case OP_LDY_ABSX:
        break;
    
    // LSR - Logical Shift Right
    case OP_LSR_ACC:
    case OP_LSR_ZP:
    case OP_LSR_ZPX:
    case OP_LSR_ABS:
    case OP_LSR_ABSX:
        operand >>= 1;
        break;

    // ORA - Logical Inclusive OR
    case OP_ORA_IMM:
    case OP_ORA_ZP:
    case OP_ORA_ZPX:
    case OP_ORA_ABS:
    case OP_ORA_ABSX:
    case OP_ORA_ABSY:
    case OP_ORA_INDX:
    case OP_ORA_INDY:
        A |= operand;
        P.set(Flag_Zero, A == 0);
        P.set(Flag_Negative, A & NEGATIVE_BIT);
        break;
    
    // ROL - Rotate Left
    case OP_ROL_ACC:
    case OP_ROL_ZP:
    case OP_ROL_ZPX:
    case OP_ROL_ABS:
    case OP_ROL_ABSX:
        operand <<= 1;
        break;
    
    // ROR - Rotate Right
    case OP_ROR_ACC:
    case OP_ROR_ZP:
    case OP_ROR_ZPX:
    case OP_ROR_ABS:
    case OP_ROR_ABSX:
        operand >>= 1;
        break;
    
    // SBC - Subtract with Carry
    case OP_SBC_IMM:
    case OP_SBC_ZP:
    case OP_SBC_ZPX:
    case OP_SBC_ABS:
    case OP_SBC_ABSX:
    case OP_SBC_ABSY:
    case OP_SBC_INDX:
    case OP_SBC_INDY:
        A -= operand;
        P.set(Flag_Zero, A == 0);
        P.set(Flag_Negative, A & NEGATIVE_BIT);
        break;
    
    // STA - Store Accumulator
    case OP_STA_ZP:
    case OP_STA_ZPX:
    case OP_STA_ABS:
    case OP_STA_ABSX:
    case OP_STA_ABSY:
    case OP_STA_INDX:
    case OP_STA_INDY:
        mmu.Write(operand16, A);
        break;
    
    // STX - Store X Register
    case OP_STX_ZP:
    case OP_STX_ZPY:
    case OP_STX_ABS:
        mmu.Write(operand16, X);
        break;
    
    // STY - Store Y Register
    case OP_STY_ZP:
    case OP_STY_ZPX:
    case OP_STY_ABS:
        mmu.Write(operand16, Y);
        break;
    }

    // Update flags

    PC += operandSize;
}
void CPU::Run() {
    while (running) {
        Execute();
    }
}
void CPU::Pause() {
    running = false;
}
void CPU::ReadResetVector() {
    // Little Endian
    PC = mmu.Read(Addr_Reset) | (mmu.Read(Addr_Reset + 1) << 8);
    SPDLOG_TRACE(fmt::runtime("PC initialized from reset vector to {:#x}"), PC);
}

void CPU::Push(uint8_t value) {
    mmu.Write(Addr_Stack + S, value);
    S--;
}

void CPU::PushAddr(Addr address) {
    uint8_t upper = address >> 8;
    uint8_t lower = address & 0xFF;
    Push(upper);
    Push(lower);
}

uint8_t CPU::Pop() {
    auto value = mmu.Read(Addr_Stack + S);
    S++;
    return value;
}

Addr CPU::PopAddr() {
    auto lower = Pop();
    auto upper = Pop();
    return (upper << 8) | lower;
}
