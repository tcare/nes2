#include "CPU.h"

#include <fmt/format.h>

// https://www.nesdev.org/wiki/CPU_power_up_state#At_power-up
void CPU::PowerOn() {
    SPDLOG_INFO("CPU power up");
    P = 0x24; // IRQ Disabled
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
    // Save the offset before PC gets messed with
    const auto instrOffset = PC;

    // Read opcode
    auto opcode = mmu.Read(PC);

    // Read addressing mode
    auto addrMode = InstrDataTable[opcode].mode;

    VERIFY(!InstrDataTable[opcode].illegal);

    // Increment PC before address calculations
    auto operandSize = AddrModeDataTable[addrMode].size;
    PC += operandSize;

    // Decode addressing mode and fill operands
    FetchOperands(addrMode, opcode, instrOffset);

    cycles += InstrDataTable[opcode].cycles;

    // Print NESTest line for diffing/debugging
    PrintNESTestLine(instrOffset);

    // Execute instruction
    ExecInstr(opcode);

    if (addrMode == Addr_Accumulator) {
        A = operand;
    }
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
    //PC = mmu.Read(Addr_Reset) | (mmu.Read(Addr_Reset + 1) << 8);
    PC = 0xC000;
    SPDLOG_TRACE(fmt::runtime("PC initialized from reset vector to {:#x}"), PC);
}

void CPU::Push(uint8_t value) {
    SPDLOG_TRACE("Push to offset {:#02X} ({:#04X}) = {:#02X}", S, Addr_Stack + S, value);
    mmu.Write(Addr_Stack + S, value);
    S--;
}

void CPU::PushAddr(Addr address) {
    SPDLOG_TRACE("Pushing address to stack: {:#04X}", address);
    uint8_t upper = address >> 8;
    uint8_t lower = address & 0xFF;
    Push(upper);
    Push(lower);
}

uint8_t CPU::Pop() {
    S++;
    auto value = mmu.Read(Addr_Stack + S);
    SPDLOG_TRACE("Popping value {:#02X} from stack offset {:#02X} = {:#04X}", value, S, Addr_Stack + S);
    return value;
}

Addr CPU::PopAddr() {
    auto lower = Pop();
    auto upper = Pop();
    Addr addr = (upper << 8) | lower;
    SPDLOG_TRACE("Popping address from stack: {:#04X}", addr);
    return addr;
}

void CPU::FetchOperands(AddrMode addrMode, uint8_t opcode, uint16_t instrOffset) {
    auto addrData = AddrModeDataTable[addrMode];
    auto opData = InstrDataTable[opcode];

    switch (addrData.size) {
    case 1:
        break;
        imm0 = 0;
        imm1 = 0;
    case 2:
        imm0 = mmu.Read(instrOffset + 1);
        imm1 = 0;
        break;
    case 3:
        imm0 = mmu.Read(instrOffset + 1);
        imm1 = mmu.Read(instrOffset + 2);
        break;
    }

    instrToStr = "";
    switch (addrMode) {
    case Addr_Implicit:
        // No operand
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic);
        break;

    case Addr_Accumulator:
        operand = A;
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic);
        break;

    case Addr_Immediate:
        // Operand is immediately after opcode
        operand = imm0;
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, operand);
        break;

    case Addr_ZeroPage: {
        // Operand address is immediately after and extended to 16-bit
        operand = mmu.Read(imm0);
        operandAddr = imm0;
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, imm0, A);
        }
        break;

    case Addr_Absolute: {
        // Operand is next 16 bits after opcode
        auto addrLower = imm0;
        auto addrUpper = imm1 << 8;
        operandAddr = addrUpper | addrLower;
        operand = mmu.Read(operandAddr);
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, operandAddr);
        }
        break;

    case Addr_Relative: {
        // 8 bit signed offset relative to PC (including instruction legnth)
        operandAddr = PC;
        int8_t offset = imm0;
        operandAddr += offset;
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, operandAddr);
        }
        break;

    case Addr_Indirect: {
        // Operand is at address in the next 16 bits after opcode
        auto indirectUpper = imm0 << 8;
        auto indirectLower = imm1;
        Addr indirectAddr = indirectUpper | indirectLower;
        auto derefUpper = mmu.Read(indirectAddr) << 8;
        auto derefLower = mmu.Read(indirectAddr+1);
        Addr derefAddr = derefUpper | derefLower;
        operand = mmu.Read(derefAddr);
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, indirectAddr, operand);
        }
        break;

    // Indexed addressing modes
    case Addr_ZeroPageX: {  // Zero page indexed, val = PEEK((arg + X) % 256)
        Addr zpxAddr = imm0;
        Addr zpxEffectiveAddr = (zpxAddr + X) % 256;
        operand = mmu.Read(zpxEffectiveAddr);
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, zpxAddr, zpxEffectiveAddr, operand);
        }
        break;
    
    case Addr_ZeroPageY: {   // Zero page indexed, val = PEEK((arg + Y) % 256)
        auto zpyAddr = imm0;
        auto zpyEffectiveAddr = (zpyAddr + Y) % 256;
        operand = mmu.Read(zpyEffectiveAddr);
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, zpyAddr, zpyEffectiveAddr, operand);
        }
        break;

    case Addr_AbslX: {       // Absolute indexed, val = PEEK(arg + X)
        Addr abslXAddr = imm0 + X;
        operand = mmu.Read(abslXAddr);
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, imm0, abslXAddr, operand);
        }
        break;

    case Addr_AbslY: {      // Absolute indexed, val = PEEK(arg + Y)
        Addr abslYAddr = imm0 + Y;
        operand = mmu.Read(abslYAddr);
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, imm0, abslYAddr, operand);
        }
        break;

    case Addr_IndirX: {      // Indexed indirect, val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X + 1) % 256) * 256)
        auto indirXAddr = imm0 + X;
        auto indirXEffectiveAddr = (indirXAddr + X) % 256;
        auto indirXLower = mmu.Read(indirXEffectiveAddr);
        auto indirXUpper = mmu.Read(indirXEffectiveAddr+1) << 8;
        auto derefXAddr = indirXUpper | indirXLower;
        operand = mmu.Read(derefXAddr);
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, imm0, indirXAddr, derefXAddr, operand);
        }
        break;

    case Addr_IndirY: {     // Indexed indirect, val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256) * 256 + Y)
        auto indirYAddr = imm0;
        auto indirYLower = mmu.Read(indirYAddr);
        auto indirYUpper = mmu.Read(indirYAddr+1) << 8;
        auto derefYAddr = indirYUpper | indirYLower;
        auto indirYEffectiveAddr = derefYAddr + Y;
        operand = mmu.Read(indirYEffectiveAddr);
        instrToStr = fmt::format(fmt::runtime(addrData.fmt), opData.mnemonic, imm0, indirYAddr, derefYAddr, operand);
        }
        break;
    
    case Addr_Illegal:
        throw std::runtime_error("Illegal addressing mode");
        break;
    }

}

void CPU::ExecInstr(uint8_t opcode) {
    switch (opcode) {
    // ADC - Add with Carry
    case OP_ADC_IMM:
    case OP_ADC_ZP:
    case OP_ADC_ZPX:
    case OP_ADC_ABS:
    case OP_ADC_ABSX:
    case OP_ADC_ABSY:
    case OP_ADC_INDX:
    case OP_ADC_INDY: {
        uint16_t result = A + operand + P.test(Flag_Carry);
        P.set(Flag_Negative, result & NEGATIVE_BIT);
        P.set(Flag_Overflow, (A ^ result) & ( operand ^ result) & NEGATIVE_BIT);
        P.set(Flag_Carry, result > 0xFF);
        A = result & 0xFF;
        P.set(Flag_Zero, A == 0);
        break;
    }

    // AND - Logical AND
    case OP_AND_IMM:
    case OP_AND_ZP:
    case OP_AND_ZPX:
    case OP_AND_ABS:
    case OP_AND_ABSX:
    case OP_AND_ABSY:
    case OP_AND_INDX:
    case OP_AND_INDY: {
        A &= operand;
        P.set(Flag_Negative, A & NEGATIVE_BIT);
        P.set(Flag_Zero, A == 0);
        break;
    }

    // ASL - Arithmetic Shift Left
    case OP_ASL_ACC:
    case OP_ASL_ZP:
    case OP_ASL_ZPX:
    case OP_ASL_ABS:
    case OP_ASL_ABSX:
        P.set(Flag_Carry, operand & NEGATIVE_BIT);
        operand <<= 1;
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
        P.set(Flag_Zero, operand == 0);
        break;

    // BCC - Branch if Carry Clear
    case OP_BCC_REL:
        if (!P.test(Flag_Carry))
            PC = operandAddr;
        break;
    
    // BCS - Branch if Carry Set
    case OP_BCS_REL:
        if (P.test(Flag_Carry))
            PC = operandAddr;
        break;
    
    // BEQ - Branch if Equal
    case OP_BEQ_REL:
        if (P.test(Flag_Zero))
            PC = operandAddr;
        break;

    // BMI - Branch if Minus
    case OP_BMI_REL:
        if (P.test(Flag_Negative))
            PC = operandAddr;
        break;

    // BNE - Branch if Not Equal
    case OP_BNE_REL:
        if (!P.test(Flag_Zero))
            PC = operandAddr;
        break;

    // BPL - Branch if Positive
    case OP_BPL_REL:
        if (!P.test(Flag_Negative))
            PC = operandAddr;
        break;

    // BVC - Branch if Overflow Clear
    case OP_BVC_REL:
        if (!P.test(Flag_Overflow))
            PC = operandAddr;
        break;

    // BVS - Branch if Overflow Set
    case OP_BVS_REL:
        if (P.test(Flag_Overflow))
            PC = operandAddr;
        break;

    // BIT - Bit Test
    case OP_BIT_ZP:
    case OP_BIT_ABS:
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
        P.set(Flag_Overflow, operand & OVERFLOW_BIT);
        P.set(Flag_Zero, (operand & A) == 0);
        break;

    // BRK - Force Interrupt
    case OP_BRK_IMP:
        PushAddr(PC);
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
        P.set(Flag_Zero, A == 0);
        P.set(Flag_Negative, A & NEGATIVE_BIT);
        break;

    // PHP - Push Processor Status
    case OP_PHP_IMP:
        // B4 flag is implicitly set on push
        Push(P.to_ulong() | (1 << Flag_B4));
        break;
    
    // PLP - Pull Processor Status
    case OP_PLP_IMP:
        // B5 flag is implicitly set on pull
        P = Pop();
        P.set(Flag_B5);
        break;
    
    // RTI - Return from Interrupt
    case OP_RTI_IMP:
        P = Pop();
        // B5 flag is implicitly set on pull
        P.set(Flag_B5);
        PC = PopAddr();
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
        P.set(Flag_Zero, X == 0);
        P.set(Flag_Negative, X & NEGATIVE_BIT);
        break;

    // TXA - Transfer X to Accumulator
    case OP_TXA_IMP:
        A = X;
        P.set(Flag_Zero, A == 0);
        P.set(Flag_Negative, A & NEGATIVE_BIT);
        break;

    // TAY - Transfer Accumulator to Y
    case OP_TAY_IMP:
        Y = A;
        P.set(Flag_Zero, Y == 0);
        P.set(Flag_Negative, Y & NEGATIVE_BIT);
        break;

    // TYA - Transfer Y to Accumulator
    case OP_TYA_IMP:
        A = Y;
        P.set(Flag_Zero, A == 0);
        P.set(Flag_Negative, A & NEGATIVE_BIT);
        break;

    // TSX - Transfer Stack Pointer to X
    case OP_TSX_IMP:
        X = S;
        P.set(Flag_Zero, X == 0);
        P.set(Flag_Negative, X & NEGATIVE_BIT);
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
        mmu.Write(operandAddr, operand);
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
        mmu.Write(operandAddr, operand);
        break;

    // JMP - Jump
    case OP_JMP_ABS:
    case OP_JMP_IND:
        PC = operandAddr;
        break;

    // JSR - Jump to Subroutine
    case OP_JSR_ABS:
        PushAddr(PC - 1);
        PC = operandAddr;
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
        Y = operand;
        P.set(Flag_Negative, Y & NEGATIVE_BIT);
        P.set(Flag_Zero, Y == 0);
        break;
    
    // LSR - Logical Shift Right
    case OP_LSR_ACC:
    case OP_LSR_ZP:
    case OP_LSR_ZPX:
    case OP_LSR_ABS:
    case OP_LSR_ABSX:
        P.set(Flag_Carry, operand & 0b1);
        operand >>= 1;
        P.set(Flag_Zero, operand == 0);
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
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
    case OP_ROL_ABSX: {
        bool carrySave = P.test(Flag_Carry);
        P.set(Flag_Carry, operand & 0b1000'0000);
        operand <<= 1;
        operand |= carrySave;
        P.set(Flag_Zero, operand == 0);
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
    }
        break;
    
    // ROR - Rotate Right
    case OP_ROR_ACC:
    case OP_ROR_ZP:
    case OP_ROR_ZPX:
    case OP_ROR_ABS:
    case OP_ROR_ABSX: {
        bool carrySave = P.test(Flag_Carry);
        P.set(Flag_Carry, operand & 0b1);
        operand >>= 1;
        operand |= carrySave << 7;
        P.set(Flag_Zero, operand == 0);
        P.set(Flag_Negative, operand & NEGATIVE_BIT);
        break;
    }
    
    // SBC - Subtract with Carry
    case OP_SBC_IMM:
    case OP_SBC_ZP:
    case OP_SBC_ZPX:
    case OP_SBC_ABS:
    case OP_SBC_ABSX:
    case OP_SBC_ABSY:
    case OP_SBC_INDX:
    case OP_SBC_INDY: {
        int16_t result = static_cast<int8_t>(A) - operand - (1 - P.test(Flag_Carry));
        P.set(Flag_Negative, result & NEGATIVE_BIT);
        P.set(Flag_Overflow, result < INT8_MIN || result > INT8_MAX);
        A = result & 0xFF;
        P.set(Flag_Carry, static_cast<int8_t>(A) > -1);
        P.set(Flag_Zero, A == 0);
    }
    break;
    
    // STA - Store Accumulator
    case OP_STA_ZP:
    case OP_STA_ZPX:
    case OP_STA_ABS:
    case OP_STA_ABSX:
    case OP_STA_ABSY:
    case OP_STA_INDX:
    case OP_STA_INDY:
        mmu.Write(operandAddr, A);
        break;
    
    // STX - Store X Register
    case OP_STX_ZP:
    case OP_STX_ZPY:
    case OP_STX_ABS:
        mmu.Write(operandAddr, X);
        break;
    
    // STY - Store Y Register
    case OP_STY_ZP:
    case OP_STY_ZPX:
    case OP_STY_ABS:
        mmu.Write(operandAddr, Y);
        break;
    }
}

void CPU::PrintNESTestLine(Addr instrOffset) {
    // Print opcode based on instruction length
    auto opcode = mmu.Read(instrOffset);
    auto instr = InstrDataTable[opcode];
    auto addrMode = AddrModeDataTable[instr.mode];
    std::string fullOpcode;
    switch (addrMode.size) {
    case 1:
        fullOpcode = fmt::format("{:02X}", opcode);
        break;
    case 2:
        fullOpcode = fmt::format("{:02X} {:02X}", opcode, imm0);
        break;
    case 3:
        fullOpcode = fmt::format("{:02X} {:02X} {:02X}", opcode, imm0, imm1);
        break;
    }


    // Print mnemonic format of instruction and add whitespace to align
    // eg. "STX $00 = 00                    "

    // Print registers
    std::string registers = fmt::format("A:{:02X} X:{:02X} Y:{:02X} P:{:02X} SP:{:02X}", A, X, Y, P.to_ulong(), S);

    // Print PPU state
    std::string ppuInfo = fmt::format("PPU:{:3d},{:3d} ", 0, 0);

    // Print cycle count
    std::string cycleInfo = fmt::format("CYC:{}", cycles);

    //C000  4C F5 C5  JMP $C5F5                       A:00 X:00 Y:00 P:24 SP:FD PPU:  0, 21 CYC:7
    std::string fullString;
    fullString = fmt::format("{:04X}  {:9} {:30} {} {} {}", instrOffset, fullOpcode, instrToStr, registers, ppuInfo, cycleInfo);

    SPDLOG_INFO(fullString);
    nesTestOutput << fullString << std::endl;
    nesTestOutput.flush();
}
