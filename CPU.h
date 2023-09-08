#include "MMU.h"

#include <fstream>

class CPU {
public:
    CPU(MMU& mmu) :
        nesTestOutput("nestest.log", std::ofstream::out),
        mmu(mmu) {
        SPDLOG_INFO("CPU created");
    }

    ~CPU() {
        SPDLOG_INFO("CPU destroyed");
    }

    void PowerOn();
    void Reset();

    void Execute();

    void Run();

    void Pause();

    enum AddrConstants : Addr {
        Addr_Stack = 0x0100,
        Addr_IRQ   = 0xFFFE,
        Addr_Reset = 0xFFFC,
        Addr_NMI   = 0xFFFA,
        Addr_BRK   = 0xFFFE
    };

private:

#include "InstrTable.h"

    void ReadResetVector();

    void FetchOperands(AddrMode addrMode, uint8_t opcode, uint16_t instrOffset);
    void ExecInstr(uint8_t opcode);

    void Push(uint8_t value);
    void PushAddr(Addr address);
    uint8_t Pop();
    Addr PopAddr();

    // Instruction string representation for NESTest
    std::string instrToStr;
    std::ofstream nesTestOutput;
    void PrintNESTestLine(Addr instrOffset);

    bool running = true;
    size_t cycles = 0;

    enum StatusFlags {
        Flag_Carry = 0,
        Flag_Zero = 1,
        Flag_InterruptDisable = 2,
        Flag_Decimal = 3,
        Flag_B4 = 4,
        Flag_B5 = 5,
        Flag_Overflow = 6,
        Flag_Negative = 7
    };

    std::bitset<8> P; // Processor status
    // NVss DIZC
    // |||| |||+- Carry
    // |||| ||+-- Zero
    // |||| |+--- Interrupt Disable
    // |||| +---- Decimal
    // ||++------ No CPU effect, see: the B flag
    // |+-------- Overflow
    // +--------- Negative

    // Memory
    MMU& mmu;

    // Registers
    uint8_t A; // Accumulator
    uint8_t X; // X index
    uint8_t Y; // Y index
    uint8_t S; // Stack pointer
    uint16_t PC; // Program counter

    const uint8_t NEGATIVE_BIT = 0b1000'0000;
    //const uint16_t CARRY_BIT = 0b1'0000'0000;
    const uint8_t OVERFLOW_BIT = 0b0100'0000;

    uint8_t operand = 0;
    Addr operandAddr = 0;
    // Optional values immediately following the opcode, depending on addressing mode
    uint8_t imm0 = 0;
    uint8_t imm1 = 0;
};