#pragma once

#include <cstdint>
#include <string>

enum AddrMode : uint8_t {
    Addr_Implicit,
    Addr_Accumulator, // A
    Addr_Immediate,   // #v
    Addr_ZeroPage,    // d
    Addr_Absolute,    // a
    Addr_Relative,    // label
    Addr_Indirect,    // (a)

    // Indexed addressing modes
    Addr_ZeroPageX,   // Zero page indexed, val = PEEK((arg + X) % 256)
    Addr_ZeroPageY,   // Zero page indexed, val = PEEK((arg + Y) % 256)
    Addr_AbslX,       // Absolute indexed, val = PEEK(arg + X)
    Addr_AbslY,       // Absolute indexed, val = PEEK(arg + Y)
    Addr_IndirX,      // Indexed indirect, val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X + 1) % 256) * 256)
    Addr_IndirY,      // Indexed indirect, val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256) * 256 + Y)

    Addr_Illegal      // For illegal instructions
};

struct AddrModeData {
    AddrMode mode;
    uint8_t size;
    std::string_view mnemonic;
    std::string_view name;
    std::string_view fmt;
    std::string_view desc;
};

static constexpr AddrModeData AddrModeDataTable[14] = {
    // Mode enum        Size Mnemonic Name           Nestest Fmt String    Description
    { Addr_Implicit,    1,   "imp",   "Implicit",    "{}",                                        "Implicit" },
    { Addr_Accumulator, 1,   "ACC",   "Accumulator", "{} A",                                      "Accumulator" },
    { Addr_Immediate,   2,   "IMM",   "Immediate",   "{} #${:02X}",                               "Immediate" },
    { Addr_ZeroPage,    2,   "ZP",    "Zero Page",   "{} ${:02X} = {:02X}",                       "Zero Page" },
    { Addr_Absolute,    3,   "ABS",   "Absolute",    "{} ${:04X}",                                "Absolute" },
    { Addr_Relative,    2,   "REL",   "Relative",    "{} ${:04X}",                                "Relative" },
    { Addr_Indirect,    3,   "IND",   "Indirect",    "{} (${:04X}) = {:04X}",                     "Indirect" },
    { Addr_ZeroPageX,   2,   "ZPX",   "Zero Page X", "{} ${:02X},X @ {:02X} = {:02X}",            "Zero Page X" },
    { Addr_ZeroPageY,   2,   "ZPY",   "Zero Page Y", "{} ${:02X},Y @ {:02X} = {:02X}",            "Zero Page Y" },
    { Addr_AbslX,       3,   "ABX",   "Absolute X",  "{} ${:04X},X @ {:04X} = {:02X}",            "Absolute X" },
    { Addr_AbslY,       3,   "ABY",   "Absolute Y",  "{} ${:04X},Y @ {:04X} = {:02X}",            "Absolute Y" },
    { Addr_IndirX,      2,   "IDX",   "Indexed X",   "{} (${:02X},X) @ {:02X} = {:04X} = {:02X}", "Indexed X" },
    { Addr_IndirY,      2,   "IDY",   "Indexed Y",   "{} (${:02X}),Y = {:04X} @ {:04X} = {:02X}", "Indexed Y" },
    { Addr_Illegal,     1,   "ILL",   "Illegal",     "{}",                                        "Illegal" }
};

struct InstrData {
    std::string_view mnemonic;
    AddrMode mode;
    bool illegal;
    uint8_t cycles;
    uint8_t pageCycles;
    bool updatesOperand; // Updates operand (memory or register) after execution
    std::string_view desc;
};

static constexpr InstrData InstrDataTable[256] = {
    // Opcode    Mnemonic Mode              Illegal Cycles PageCycles UpdatesOperand Description
    /* 0x00 */ { "BRK",   Addr_Implicit,    false,  7,     0,         false,         "Force Break" },
    /* 0x01 */ { "ORA",   Addr_IndirX,      false,  6,     0,         false,         "Or Memory with Accumulator" }, 
    /* 0x02 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x03 */ { "*SLO",  Addr_IndirX,      true,   8,     0,         false,         "Illegal" },
    /* 0x04 */ { "*NOP",  Addr_ZeroPage,    true,   3,     0,         false,         "Test and Set Memory Bits" },
    /* 0x05 */ { "ORA",   Addr_ZeroPage,    false,  3,     0,         false,         "Or Memory with Accumulator" },
    /* 0x06 */ { "ASL",   Addr_ZeroPage,    false,  5,     0,         true,          "Arithmetic Shift Left" },
    /* 0x07 */ { "*SLO",  Addr_ZeroPage,    true,   5,     0,         false,         "Illegal" },
    /* 0x08 */ { "PHP",   Addr_Implicit,    false,  3,     0,         false,         "Push Processor Status on Stack" }, 
    /* 0x09 */ { "ORA",   Addr_Immediate,   false,  2,     0,         false,         "Or Memory with Accumulator" },
    /* 0x0A */ { "ASL",   Addr_Accumulator, false,  2,     0,         true,          "Arithmetic Shift Left" },
    /* 0x0B */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0x0C */ { "*NOP",  Addr_Absolute,    true,   4,     0,         false,         "Illegal" },
    /* 0x0D */ { "ORA",   Addr_Absolute,    false,  4,     0,         false,         "Or Memory with Accumulator" },
    /* 0x0E */ { "ASL",   Addr_Absolute,    false,  6,     0,         true,          "Arithmetic Shift Left" },
    /* 0x0F */ { "*SLO",  Addr_Absolute,    true,   6,     0,         false,         "Illegal" },
    /* 0x10 */ { "BPL",   Addr_Relative,    false,  2,     1,         false,         "Branch if Positive" },
    /* 0x11 */ { "ORA",   Addr_IndirY,      false,  5,     1,         false,         "Or Memory with Accumulator" },
    /* 0x12 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x13 */ { "*SLO",  Addr_IndirY,      true,   8,     0,         false,         "Illegal" },
    /* 0x14 */ { "*NOP",  Addr_ZeroPageX,   true,   4,     0,         false,         "Illegal" },
    /* 0x15 */ { "ORA",   Addr_ZeroPageX,   false,  4,     0,         false,         "Or Memory with Accumulator" },
    /* 0x16 */ { "ASL",   Addr_ZeroPageX,   false,  6,     0,         true,          "Arithmetic Shift Left" },
    /* 0x17 */ { "*SLO",  Addr_ZeroPageX,   true,   6,     0,         false,         "Illegal" },
    /* 0x18 */ { "CLC",   Addr_Implicit,    false,  2,     0,         false,         "Clear Carry Flag" },
    /* 0x19 */ { "ORA",   Addr_AbslY,       false,  4,     1,         false,         "Or Memory with Accumulator" },
    /* 0x1A */ { "*NOP",  Addr_Illegal,     true,   2,     0,         false,         "Illegal" },
    /* 0x1B */ { "*SLO",  Addr_AbslY,       true,   7,     0,         false,         "Illegal" },
    /* 0x1C */ { "*NOP",  Addr_AbslX,       true,   4,     1,         false,         "Illegal" },
    /* 0x1D */ { "ORA",   Addr_AbslX,       false,  4,     1,         false,         "Or Memory with Accumulator" },
    /* 0x1E */ { "ASL",   Addr_AbslX,       false,  7,     0,         true,          "Arithmetic Shift Left" },
    /* 0x1F */ { "*SLO",  Addr_AbslX,       true,   7,     0,         false,         "Illegal" },
    /* 0x20 */ { "JSR",   Addr_Absolute,    false,  6,     0,         false,         "Jump to New Location Saving Return Address" },
    /* 0x21 */ { "AND",   Addr_IndirX,      false,  6,     0,         false,         "And Memory with Accumulator" },
    /* 0x22 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x23 */ { "*RLA",  Addr_IndirX,      true,   8,     0,         false,         "Illegal" },
    /* 0x24 */ { "BIT",   Addr_ZeroPage,    false,  3,     0,         false,         "Test Bits in Memory with Accumulator" },
    /* 0x25 */ { "AND",   Addr_ZeroPage,    false,  3,     0,         false,         "And Memory with Accumulator" },
    /* 0x26 */ { "ROL",   Addr_ZeroPage,    false,  5,     0,         true,          "Rotate One Bit Left (Memory or Accumulator)" },
    /* 0x27 */ { "*RLA",  Addr_ZeroPage,    true,   5,     0,         false,         "Illegal" },
    /* 0x28 */ { "PLP",   Addr_Implicit,    false,  4,     0,         false,         "Pull Processor Status from Stack" },
    /* 0x29 */ { "AND",   Addr_Immediate,   false,  2,     0,         false,         "And Memory with Accumulator" },
    /* 0x2A */ { "ROL",   Addr_Accumulator, false,  2,     0,         true,          "Rotate One Bit Left (Memory or Accumulator)" },
    /* 0x2B */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0x2C */ { "BIT",   Addr_Absolute,    false,  4,     0,         false,         "Test Bits in Memory with Accumulator" },
    /* 0x2D */ { "AND",   Addr_Absolute,    false,  4,     0,         false,         "And Memory with Accumulator" },
    /* 0x2E */ { "ROL",   Addr_Absolute,    false,  6,     0,         true,          "Rotate One Bit Left (Memory or Accumulator)" },
    /* 0x2F */ { "*RLA",  Addr_Absolute,    true,   6,     0,         false,         "Illegal" },
    /* 0x30 */ { "BMI",   Addr_Relative,    false,  2,     1,         false,         "Branch if Minus" },
    /* 0x31 */ { "AND",   Addr_IndirY,      false,  5,     1,         false,         "And Memory with Accumulator" },
    /* 0x32 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x33 */ { "*RLA",  Addr_IndirY,      true,   8,     0,         false,         "Illegal" },
    /* 0x34 */ { "*NOP",  Addr_ZeroPageX,   true,   4,     0,         false,         "Illegal" },
    /* 0x35 */ { "AND",   Addr_ZeroPageX,   false,  4,     0,         false,         "And Memory with Accumulator" },
    /* 0x36 */ { "ROL",   Addr_ZeroPageX,   false,  6,     0,         true,          "Rotate One Bit Left (Memory or Accumulator)" },
    /* 0x37 */ { "*RLA",  Addr_ZeroPageX,   true,   6,     0,         false,         "Illegal" },
    /* 0x38 */ { "SEC",   Addr_Implicit,    false,  2,     0,         false,         "Set Carry Flag" },
    /* 0x39 */ { "AND",   Addr_AbslY,       false,  4,     1,         false,         "And Memory with Accumulator" },
    /* 0x3A */ { "*NOP",  Addr_Illegal,     true,   2,     0,         false,         "Illegal" },
    /* 0x3B */ { "*RLA",  Addr_AbslY,       true,   7,     0,         false,         "Illegal" },
    /* 0x3C */ { "*NOP",  Addr_AbslX,       true,   4,     1,         false,         "Illegal" },
    /* 0x3D */ { "AND",   Addr_AbslX,       false,  4,     1,         false,         "And Memory with Accumulator" },
    /* 0x3E */ { "ROL",   Addr_AbslX,       false,  7,     0,         true,          "Rotate One Bit Left (Memory or Accumulator)" },
    /* 0x3F */ { "*RLA",  Addr_AbslX,       true,   7,     0,         false,         "Illegal" },
    /* 0x40 */ { "RTI",   Addr_Implicit,    false,  6,     0,         false,         "Return from Interrupt" },
    /* 0x41 */ { "EOR",   Addr_IndirX,      false,  6,     0,         false,         "Exclusive-Or Memory with Accumulator" },
    /* 0x42 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x43 */ { "*SRE",  Addr_IndirX,      true,   8,     0,         false,         "Illegal" },
    /* 0x44 */ { "*NOP",  Addr_ZeroPage,    true,   3,     0,         false,         "Illegal" },
    /* 0x45 */ { "EOR",   Addr_ZeroPage,    false,  3,     0,         false,         "Exclusive-Or Memory with Accumulator" },
    /* 0x46 */ { "LSR",   Addr_ZeroPage,    false,  5,     0,         true,          "Logical Shift Right" },
    /* 0x47 */ { "*SRE",  Addr_ZeroPage,    true,   5,     0,         false,         "Illegal" },
    /* 0x48 */ { "PHA",   Addr_Implicit,    false,  3,     0,         false,         "Push Accumulator on Stack" },
    /* 0x49 */ { "EOR",   Addr_Immediate,   false,  2,     0,         false,         "Exclusive-Or Memory with Accumulator" },
    /* 0x4A */ { "LSR",   Addr_Accumulator, false,  2,     0,         true,          "Logical Shift Right" },
    /* 0x4B */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0x4C */ { "JMP",   Addr_Absolute,    false,  3,     0,         false,         "Jump to New Location" },
    /* 0x4D */ { "EOR",   Addr_Absolute,    false,  4,     0,         false,         "Exclusive-Or Memory with Accumulator" },
    /* 0x4E */ { "LSR",   Addr_Absolute,    false,  6,     0,         true,          "Logical Shift Right" },
    /* 0x4F */ { "*SRE",  Addr_Absolute,    true,   6,     0,         false,         "Illegal" },
    /* 0x50 */ { "BVC",   Addr_Relative,    false,  2,     1,         false,         "Branch if Overflow Clear" },
    /* 0x51 */ { "EOR",   Addr_IndirY,      false,  5,     1,         false,         "Exclusive-Or Memory with Accumulator" },
    /* 0x52 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x53 */ { "*SRE",  Addr_IndirY,      true,   8,     0,         false,         "Illegal" },
    /* 0x54 */ { "*NOP",  Addr_ZeroPageX,   true,   4,     0,         false,         "Illegal" },
    /* 0x55 */ { "EOR",   Addr_ZeroPageX,   false,  4,     0,         false,         "Exclusive-Or Memory with Accumulator" },
    /* 0x56 */ { "LSR",   Addr_ZeroPageX,   false,  6,     0,         true,          "Logical Shift Right" },
    /* 0x57 */ { "*SRE",  Addr_ZeroPageX,   true,   6,     0,         false,         "Illegal" },
    /* 0x58 */ { "CLI",   Addr_Implicit,    false,  2,     0,         false,         "Clear Interrupt Disable Bit" },
    /* 0x59 */ { "EOR",   Addr_AbslY,       false,  4,     1,         false,         "Exclusive-Or Memory with Accumulator" },
    /* 0x5A */ { "*NOP",  Addr_Illegal,     true,   2,     0,         false,         "Illegal" },
    /* 0x5B */ { "*SRE",  Addr_AbslY,       true,   7,     0,         false,         "Illegal" },
    /* 0x5C */ { "*NOP",  Addr_AbslX,       true,   4,     1,         false,         "Illegal" },
    /* 0x5D */ { "EOR",   Addr_AbslX,       false,  4,     1,         false,         "Exclusive-Or Memory with Accumulator" },
    /* 0x5E */ { "LSR",   Addr_AbslX,       false,  7,     0,         true,          "Logical Shift Right" },
    /* 0x5F */ { "*SRE",  Addr_AbslX,       true,   7,     0,         false,         "Illegal" },
    /* 0x60 */ { "RTS",   Addr_Implicit,    false,  6,     0,         false,         "Return from Subroutine" },
    /* 0x61 */ { "ADC",   Addr_IndirX,      false,  6,     0,         false,         "Add Memory to Accumulator with Carry" },
    /* 0x62 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x63 */ { "*RRA",  Addr_IndirX,      true,   8,     0,         true,         "Illegal" },
    /* 0x64 */ { "*NOP",  Addr_ZeroPage,    true,   3,     0,         false,         "Illegal" },
    /* 0x65 */ { "ADC",   Addr_ZeroPage,    false,  3,     0,         false,         "Add Memory to Accumulator with Carry" },
    /* 0x66 */ { "ROR",   Addr_ZeroPage,    false,  5,     0,         true,          "Rotate One Bit Right (Memory or Accumulator)" },
    /* 0x67 */ { "*RRA",  Addr_ZeroPage,    true,   5,     0,         true,         "Illegal" },
    /* 0x68 */ { "PLA",   Addr_Implicit,    false,  4,     0,         false,         "Pull Accumulator from Stack" },
    /* 0x69 */ { "ADC",   Addr_Immediate,   false,  2,     0,         false,         "Add Memory to Accumulator with Carry" },
    /* 0x6A */ { "ROR",   Addr_Accumulator, false,  2,     0,         true,          "Rotate One Bit Right (Memory or Accumulator)" },
    /* 0x6B */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0x6C */ { "JMP",   Addr_Indirect,    false,  5,     0,         false,         "Jump to New Location" },
    /* 0x6D */ { "ADC",   Addr_Absolute,    false,  4,     0,         false,         "Add Memory to Accumulator with Carry" },
    /* 0x6E */ { "ROR",   Addr_Absolute,    false,  6,     0,         true,          "Rotate One Bit Right (Memory or Accumulator)" },
    /* 0x6F */ { "*RRA",  Addr_Absolute,    true,   6,     0,         true,         "Illegal" },
    /* 0x70 */ { "BVS",   Addr_Relative,    false,  2,     1,         false,         "Branch if Overflow Set" },
    /* 0x71 */ { "ADC",   Addr_IndirY,      false,  5,     1,         false,         "Add Memory to Accumulator with Carry" },
    /* 0x72 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x73 */ { "*RRA",  Addr_IndirY,      true,   8,     0,         true,         "Illegal" },
    /* 0x74 */ { "*NOP",  Addr_ZeroPageX,   true,   4,     0,         false,         "Illegal" },
    /* 0x75 */ { "ADC",   Addr_ZeroPageX,   false,  4,     0,         false,         "Add Memory to Accumulator with Carry" },
    /* 0x76 */ { "ROR",   Addr_ZeroPageX,   false,  6,     0,         true,          "Rotate One Bit Right (Memory or Accumulator)" },
    /* 0x77 */ { "*RRA",  Addr_ZeroPageX,   true,   6,     0,         true,         "Illegal" },
    /* 0x78 */ { "SEI",   Addr_Implicit,    false,  2,     0,         false,         "Set Interrupt Disable Status" },
    /* 0x79 */ { "ADC",   Addr_AbslY,       false,  4,     1,         false,         "Add Memory to Accumulator with Carry" },
    /* 0x7A */ { "*NOP",  Addr_Illegal,     true,   2,     0,         false,         "Illegal" },
    /* 0x7B */ { "*RRA",  Addr_AbslY,       true,   7,     0,         true,         "Illegal" },
    /* 0x7C */ { "*NOP",  Addr_AbslX,       true,   4,     1,         false,         "Illegal" },
    /* 0x7D */ { "ADC",   Addr_AbslX,       false,  4,     1,         false,         "Add Memory to Accumulator with Carry" },
    /* 0x7E */ { "ROR",   Addr_AbslX,       false,  7,     0,         true,          "Rotate One Bit Right (Memory or Accumulator)" },
    /* 0x7F */ { "*RRA",  Addr_AbslX,       true,   7,     0,         true,         "Illegal" },
    /* 0x80 */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0x81 */ { "STA",   Addr_IndirX,      false,  6,     0,         false,         "Store Accumulator in Memory" },
    /* 0x82 */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0x83 */ { "*SAX",  Addr_IndirX,      true,   6,     0,         false,         "Illegal" },
    /* 0x84 */ { "STY",   Addr_ZeroPage,    false,  3,     0,         false,         "Store Index Y in Memory" },
    /* 0x85 */ { "STA",   Addr_ZeroPage,    false,  3,     0,         false,         "Store Accumulator in Memory" },
    /* 0x86 */ { "STX",   Addr_ZeroPage,    false,  3,     0,         false,         "Store Index X in Memory" },
    /* 0x87 */ { "*SAX",  Addr_ZeroPage,    true,   3,     0,         false,         "Illegal" },
    /* 0x88 */ { "DEY",   Addr_Implicit,    false,  2,     0,         false,         "Decrement Index Y by One" },
    /* 0x89 */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0x8A */ { "TXA",   Addr_Implicit,    false,  2,     0,         false,         "Transfer Index X to Accumulator" },
    /* 0x8B */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0x8C */ { "STY",   Addr_Absolute,    false,  4,     0,         false,         "Store Index Y in Memory" },
    /* 0x8D */ { "STA",   Addr_Absolute,    false,  4,     0,         false,         "Store Accumulator in Memory" },
    /* 0x8E */ { "STX",   Addr_Absolute,    false,  4,     0,         false,         "Store Index X in Memory" },
    /* 0x8F */ { "*SAX",  Addr_Absolute,    true,   4,     0,         false,         "Illegal" },
    /* 0x90 */ { "BCC",   Addr_Relative,    false,  2,     1,         false,         "Branch if Carry Clear" },
    /* 0x91 */ { "STA",   Addr_IndirY,      false,  6,     0,         false,         "Store Accumulator in Memory" },
    /* 0x92 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0x93 */ { "*NOP",  Addr_IndirY,      true,   6,     0,         false,         "Illegal" },
    /* 0x94 */ { "STY",   Addr_ZeroPageX,   false,  4,     0,         false,         "Store Index Y in Memory" },
    /* 0x95 */ { "STA",   Addr_ZeroPageX,   false,  4,     0,         false,         "Store Accumulator in Memory" },
    /* 0x96 */ { "STX",   Addr_ZeroPageY,   false,  4,     0,         false,         "Store Index X in Memory" },
    /* 0x97 */ { "*SAX",  Addr_ZeroPageY,   true,   4,     0,         false,         "Illegal" },
    /* 0x98 */ { "TYA",   Addr_Implicit,    false,  2,     0,         false,         "Transfer Index Y to Accumulator" },
    /* 0x99 */ { "STA",   Addr_AbslY,       false,  5,     0,         false,         "Store Accumulator in Memory" },
    /* 0x9A */ { "TXS",   Addr_Implicit,    false,  2,     0,         false,         "Transfer Index X to Stack Pointer" },
    /* 0x9B */ { "*NOP",  Addr_AbslY,       true,   5,     0,         false,         "Illegal" },
    /* 0x9C */ { "*NOP",  Addr_AbslX,       true,   5,     0,         false,         "Illegal" },
    /* 0x9D */ { "STA",   Addr_AbslX,       false,  5,     0,         false,         "Store Accumulator in Memory" },
    /* 0x9E */ { "*NOP",  Addr_AbslY,       true,   5,     0,         false,         "Illegal" },
    /* 0x9F */ { "*NOP",  Addr_AbslY,       true,   5,     0,         false,         "Illegal" },
    /* 0xA0 */ { "LDY",   Addr_Immediate,   false,  2,     0,         false,         "Load Index Y with Memory" },
    /* 0xA1 */ { "LDA",   Addr_IndirX,      false,  6,     0,         false,         "Load Accumulator with Memory" },
    /* 0xA2 */ { "LDX",   Addr_Immediate,   false,  2,     0,         false,         "Load Index X with Memory" },
    /* 0xA3 */ { "*LAX",  Addr_IndirX,      true,   6,     0,         false,         "Illegal" },
    /* 0xA4 */ { "LDY",   Addr_ZeroPage,    false,  3,     0,         false,         "Load Index Y with Memory" },
    /* 0xA5 */ { "LDA",   Addr_ZeroPage,    false,  3,     0,         false,         "Load Accumulator with Memory" },
    /* 0xA6 */ { "LDX",   Addr_ZeroPage,    false,  3,     0,         false,         "Load Index X with Memory" },
    /* 0xA7 */ { "*LAX",  Addr_ZeroPage,    true,   3,     0,         false,         "Illegal" },
    /* 0xA8 */ { "TAY",   Addr_Implicit,    false,  2,     0,         false,         "Transfer Accumulator to Index Y" },
    /* 0xA9 */ { "LDA",   Addr_Immediate,   false,  2,     0,         false,         "Load Accumulator with Memory" },
    /* 0xAA */ { "TAX",   Addr_Implicit,    false,  2,     0,         false,         "Transfer Accumulator to Index X" },
    /* 0xAB */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0xAC */ { "LDY",   Addr_Absolute,    false,  4,     0,         false,         "Load Index Y with Memory" },
    /* 0xAD */ { "LDA",   Addr_Absolute,    false,  4,     0,         false,         "Load Accumulator with Memory" },
    /* 0xAE */ { "LDX",   Addr_Absolute,    false,  4,     0,         false,         "Load Index X with Memory" },
    /* 0xAF */ { "*LAX",  Addr_Absolute,    true,   4,     0,         false,         "Illegal" },
    /* 0xB0 */ { "BCS",   Addr_Relative,    false,  2,     1,         false,         "Branch if Carry Set" },
    /* 0xB1 */ { "LDA",   Addr_IndirY,      false,  5,     1,         false,         "Load Accumulator with Memory" },
    /* 0xB2 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0xB3 */ { "*LAX",  Addr_IndirY,      true,   5,     1,         false,         "Illegal" },
    /* 0xB4 */ { "LDY",   Addr_ZeroPageX,   false,  4,     0,         false,         "Load Index Y with Memory" },
    /* 0xB5 */ { "LDA",   Addr_ZeroPageX,   false,  4,     0,         false,         "Load Accumulator with Memory" },
    /* 0xB6 */ { "LDX",   Addr_ZeroPageY,   false,  4,     0,         false,         "Load Index X with Memory" },
    /* 0xB7 */ { "*LAX",  Addr_ZeroPageY,   true,   4,     0,         false,         "Illegal" },
    /* 0xB8 */ { "CLV",   Addr_Implicit,    false,  2,     0,         false,         "Clear Overflow Flag" },
    /* 0xB9 */ { "LDA",   Addr_AbslY,       false,  4,     1,         false,         "Load Accumulator with Memory" },
    /* 0xBA */ { "TSX",   Addr_Implicit,    false,  2,     0,         false,         "Transfer Stack Pointer to Index X" },
    /* 0xBB */ { "*NOP",  Addr_AbslY,       true,   4,     1,         false,         "Illegal" },
    /* 0xBC */ { "LDY",   Addr_AbslX,       false,  4,     1,         false,         "Load Index Y with Memory" },
    /* 0xBD */ { "LDA",   Addr_AbslX,       false,  4,     1,         false,         "Load Accumulator with Memory" },
    /* 0xBE */ { "LDX",   Addr_AbslY,       false,  4,     1,         false,         "Load Index X with Memory" },
    /* 0xBF */ { "*LAX",  Addr_AbslY,       true,   4,     1,         false,         "Illegal" },
    /* 0xC0 */ { "CPY",   Addr_Immediate,   false,  2,     0,         false,         "Compare Memory with Index Y" },
    /* 0xC1 */ { "CMP",   Addr_IndirX,      false,  6,     0,         false,         "Compare Memory with Accumulator" },
    /* 0xC2 */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0xC3 */ { "*DCP",  Addr_IndirX,      true,   8,     0,         false,         "Illegal" },
    /* 0xC4 */ { "CPY",   Addr_ZeroPage,    false,  3,     0,         false,         "Compare Memory with Index Y" },
    /* 0xC5 */ { "CMP",   Addr_ZeroPage,    false,  3,     0,         false,         "Compare Memory with Accumulator" },
    /* 0xC6 */ { "DEC",   Addr_ZeroPage,    false,  5,     0,         true,          "Decrement Memory by One" },
    /* 0xC7 */ { "*DCP",  Addr_ZeroPage,    true,   5,     0,         false,         "Illegal" },
    /* 0xC8 */ { "INY",   Addr_Implicit,    false,  2,     0,         false,         "Increment Index Y by One" },
    /* 0xC9 */ { "CMP",   Addr_Immediate,   false,  2,     0,         false,         "Compare Memory with Accumulator" },
    /* 0xCA */ { "DEX",   Addr_Implicit,    false,  2,     0,         false,         "Decrement Index X by One" },
    /* 0xCB */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0xCC */ { "CPY",   Addr_Absolute,    false,  4,     0,         false,         "Compare Memory with Index Y" },
    /* 0xCD */ { "CMP",   Addr_Absolute,    false,  4,     0,         false,         "Compare Memory with Accumulator" },
    /* 0xCE */ { "DEC",   Addr_Absolute,    false,  6,     0,         false,         "Decrement Memory by One" },
    /* 0xCF */ { "*DCP",  Addr_Absolute,    true,   6,     0,         false,         "Illegal" },
    /* 0xD0 */ { "BNE",   Addr_Relative,    false,  2,     1,         false,         "Branch if Not Equal" },
    /* 0xD1 */ { "CMP",   Addr_IndirY,      false,  5,     1,         false,         "Compare Memory with Accumulator" },
    /* 0xD2 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0xD3 */ { "*DCP",  Addr_IndirY,      true,   8,     0,         false,         "Illegal" },
    /* 0xD4 */ { "*NOP",  Addr_ZeroPageX,   true,   4,     0,         false,         "Illegal" },
    /* 0xD5 */ { "CMP",   Addr_ZeroPageX,   false,  4,     0,         false,         "Compare Memory with Accumulator" },
    /* 0xD6 */ { "DEC",   Addr_ZeroPageX,   false,  6,     0,         false,         "Decrement Memory by One" },
    /* 0xD7 */ { "*DCP",  Addr_ZeroPageX,   true,   6,     0,         false,         "Illegal" },
    /* 0xD8 */ { "CLD",   Addr_Implicit,    false,  2,     0,         false,         "Clear Decimal Mode" },
    /* 0xD9 */ { "CMP",   Addr_AbslY,       false,  4,     1,         false,         "Compare Memory with Accumulator" },
    /* 0xDA */ { "*NOP",  Addr_Illegal,     true,   2,     0,         false,         "Illegal" },
    /* 0xDB */ { "*DCP",  Addr_AbslY,       true,   7,     0,         false,         "Illegal" },
    /* 0xDC */ { "*NOP",  Addr_AbslX,       true,   4,     1,         false,         "Illegal" },
    /* 0xDD */ { "CMP",   Addr_AbslX,       false,  4,     1,         false,         "Compare Memory with Accumulator" },
    /* 0xDE */ { "DEC",   Addr_AbslX,       false,  7,     0,         false,         "Decrement Memory by One" },
    /* 0xDF */ { "*DCP",  Addr_AbslX,       true,   7,     0,         false,         "Illegal" },
    /* 0xE0 */ { "CPX",   Addr_Immediate,   false,  2,     0,         false,         "Compare Memory with Index X" },
    /* 0xE1 */ { "SBC",   Addr_IndirX,      false,  6,     0,         false,         "Subtract Memory from Accumulator with Borrow" },
    /* 0xE2 */ { "*NOP",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0xE3 */ { "*ISB",  Addr_IndirX,      true,   8,     0,         false,         "Illegal" },
    /* 0xE4 */ { "CPX",   Addr_ZeroPage,    false,  3,     0,         false,         "Compare Memory with Index X" },
    /* 0xE5 */ { "SBC",   Addr_ZeroPage,    false,  3,     0,         false,         "Subtract Memory from Accumulator with Borrow" },
    /* 0xE6 */ { "INC",   Addr_ZeroPage,    false,  5,     0,         true,          "Increment Memory by One" },
    /* 0xE7 */ { "*ISB",  Addr_ZeroPage,    true,   5,     0,         false,         "Illegal" },
    /* 0xE8 */ { "INX",   Addr_Implicit,    false,  2,     0,         false,         "Increment Index X by One" },
    /* 0xE9 */ { "SBC",   Addr_Immediate,   false,  2,     0,         false,         "Subtract Memory from Accumulator with Borrow" },
    /* 0xEA */ { "NOP",   Addr_Implicit,    false,  2,     0,         false,         "No Operation" },
    /* 0xEB */ { "*SBC",  Addr_Immediate,   true,   2,     0,         false,         "Illegal" },
    /* 0xEC */ { "CPX",   Addr_Absolute,    false,  4,     0,         false,         "Compare Memory with Index X" },
    /* 0xED */ { "SBC",   Addr_Absolute,    false,  4,     0,         false,         "Subtract Memory from Accumulator with Borrow" },
    /* 0xEE */ { "INC",   Addr_Absolute,    false,  6,     0,         true,          "Increment Memory by One" },
    /* 0xEF */ { "*ISB",  Addr_Absolute,    true,   6,     0,         false,         "Illegal" },
    /* 0xF0 */ { "BEQ",   Addr_Relative,    false,  2,     1,         false,         "Branch if Equal" },
    /* 0xF1 */ { "SBC",   Addr_IndirY,      false,  5,     1,         false,         "Subtract Memory from Accumulator with Borrow" },
    /* 0xF2 */ { "*NOP",  Addr_Illegal,     true,   0,     0,         false,         "Illegal" },
    /* 0xF3 */ { "*ISB",  Addr_IndirY,      true,   8,     0,         false,         "Illegal" },
    /* 0xF4 */ { "*NOP",  Addr_ZeroPageX,   true,   4,     0,         false,         "Illegal" },
    /* 0xF5 */ { "SBC",   Addr_ZeroPageX,   false,  4,     0,         false,         "Subtract Memory from Accumulator with Borrow" },
    /* 0xF6 */ { "INC",   Addr_ZeroPageX,   false,  6,     0,         true,          "Increment Memory by One" },
    /* 0xF7 */ { "*ISB",  Addr_ZeroPageX,   true,   6,     0,         false,         "Illegal" },
    /* 0xF8 */ { "SED",   Addr_Implicit,    false,  2,     0,         false,         "Set Decimal Mode" },
    /* 0xF9 */ { "SBC",   Addr_AbslY,       false,  4,     1,         false,         "Subtract Memory from Accumulator with Borrow" },
    /* 0xFA */ { "*NOP",  Addr_Illegal,     true,   2,     0,         false,         "Illegal" },
    /* 0xFB */ { "*ISB",  Addr_AbslY,       true,   7,     0,         false,         "Illegal" },
    /* 0xFC */ { "*NOP",  Addr_AbslX,       true,   4,     1,         false,         "Illegal" },
    /* 0xFD */ { "SBC",   Addr_AbslX,       false,  4,     1,         false,         "Subtract Memory from Accumulator with Borrow" },
    /* 0xFE */ { "INC",   Addr_AbslX,       false,  7,     0,         true,          "Increment Memory by One" },
    /* 0xFF */ { "*ISB",  Addr_AbslX,       true,   7,     0,         false,         "Illegal" }
};

enum Opcodes {
    OP_ADC_IMM = 0x69,
    OP_ADC_ZP = 0x65,
    OP_ADC_ZPX = 0x75,
    OP_ADC_ABS = 0x6d,
    OP_ADC_ABSX = 0x7d,
    OP_ADC_ABSY = 0x79,
    OP_ADC_INDX = 0x61,
    OP_ADC_INDY = 0x71,
		
    OP_AND_IMM = 0x29,
    OP_AND_ZP = 0x25,
    OP_AND_ZPX = 0x35,
    OP_AND_ABS = 0x2d,
    OP_AND_ABSX = 0x3d,
    OP_AND_ABSY = 0x39,
    OP_AND_INDX = 0x21,
    OP_AND_INDY = 0x31,
		
    OP_ASL_ACC = 0x0a,
    OP_ASL_ZP = 0x06,
    OP_ASL_ZPX = 0x16,
    OP_ASL_ABS = 0x0e,
    OP_ASL_ABSX = 0x1e,
		
    OP_BCC_REL = 0x90,
    OP_BCS_REL = 0xB0,
    OP_BEQ_REL = 0xF0,
    OP_BMI_REL = 0x30,
    OP_BNE_REL = 0xD0,
    OP_BPL_REL = 0x10,
    OP_BVC_REL = 0x50,
    OP_BVS_REL = 0x70,
		
    OP_BIT_ZP = 0x24,
    OP_BIT_ABS = 0x2c,
		
    OP_BRK_IMP = 0x00,
    OP_CLC_IMP = 0x18,
    OP_CLD_IMP = 0xd8,
    OP_CLI_IMP = 0x58,
    OP_CLV_IMP = 0xb8,
    OP_NOP_IMP = 0xea,
    OP_PHA_IMP = 0x48,
    OP_PLA_IMP = 0x68,
    OP_PHP_IMP = 0x08,
    OP_PLP_IMP = 0x28,
    OP_RTI_IMP = 0x40,
    OP_RTS_IMP = 0x60,
    OP_SEC_IMP = 0x38,
    OP_SED_IMP = 0xf8,
    OP_SEI_IMP = 0x78,
    OP_TAX_IMP = 0xaa,
    OP_TXA_IMP = 0x8a,
    OP_TAY_IMP = 0xa8,
    OP_TYA_IMP = 0x98,
    OP_TSX_IMP = 0xba,
    OP_TXS_IMP = 0x9a,
		
    OP_CMP_IMM = 0xc9,
    OP_CMP_ZP = 0xc5,
    OP_CMP_ZPX = 0xd5,
    OP_CMP_ABS = 0xcd,
    OP_CMP_ABSX = 0xdd,
    OP_CMP_ABSY = 0xd9,
    OP_CMP_INDX = 0xc1,
    OP_CMP_INDY = 0xd1,
		
    OP_CPX_IMM = 0xe0,
    OP_CPX_ZP = 0xe4,
    OP_CPX_ABS = 0xec,
		
    OP_CPY_IMM = 0xc0,
    OP_CPY_ZP = 0xc4,
    OP_CPY_ABS = 0xcc,
		
    OP_DEC_ZP = 0xc6,
    OP_DEC_ZPX = 0xd6,
    OP_DEC_ABS = 0xce,
    OP_DEC_ABSX = 0xde,
		
    OP_DEX_IMP = 0xca,
    OP_DEY_IMP = 0x88,
    OP_INX_IMP = 0xe8,
    OP_INY_IMP = 0xc8,
		
    OP_EOR_IMM = 0x49,
    OP_EOR_ZP = 0x45,
    OP_EOR_ZPX = 0x55,
    OP_EOR_ABS = 0x4d,
    OP_EOR_ABSX = 0x5d,
    OP_EOR_ABSY = 0x59,
    OP_EOR_INDX = 0x41,
    OP_EOR_INDY = 0x51,
		
    OP_INC_ZP = 0xe6,
    OP_INC_ZPX = 0xf6,
    OP_INC_ABS = 0xee,
    OP_INC_ABSX = 0xfe,
		
    OP_JMP_ABS = 0x4c,
    OP_JMP_IND = 0x6c,
    OP_JSR_ABS = 0x20,
		
    OP_LDA_IMM = 0xa9,
    OP_LDA_ZP = 0xa5,
    OP_LDA_ZPX = 0xb5,
    OP_LDA_ABS = 0xad,
    OP_LDA_ABSX = 0xbd,
    OP_LDA_ABSY = 0xb9,
    OP_LDA_INDX = 0xa1,
    OP_LDA_INDY = 0xb1,
		
    OP_LDX_IMM = 0xa2,
    OP_LDX_ZP = 0xa6,
    OP_LDX_ZPY = 0xb6,
    OP_LDX_ABS = 0xae,
    OP_LDX_ABSY = 0xbe,
		
    OP_LDY_IMM = 0xa0,
    OP_LDY_ZP = 0xa4,
    OP_LDY_ZPX = 0xb4,
    OP_LDY_ABS = 0xac,
    OP_LDY_ABSX = 0xbc,
		
    OP_LSR_ACC = 0x4a,
    OP_LSR_ZP = 0x46,
    OP_LSR_ZPX = 0x56,
    OP_LSR_ABS = 0x4e,
    OP_LSR_ABSX = 0x5e,
		
    OP_ORA_IMM = 0x09,
    OP_ORA_ZP = 0x05,
    OP_ORA_ZPX = 0x15,
    OP_ORA_ABS = 0x0d,
    OP_ORA_ABSX = 0x1d,
    OP_ORA_ABSY = 0x19,
    OP_ORA_INDX = 0x01,
    OP_ORA_INDY = 0x11,
		
    OP_ROL_ACC = 0x2a,
    OP_ROL_ZP = 0x26,
    OP_ROL_ZPX = 0x36,
    OP_ROL_ABS = 0x2e,
    OP_ROL_ABSX = 0x3e,
		
    OP_ROR_ACC = 0x6a,
    OP_ROR_ZP = 0x66,
    OP_ROR_ZPX = 0x76,
    OP_ROR_ABS = 0x7e,
    OP_ROR_ABSX = 0x6e,
		
    OP_SBC_IMM = 0xe9,
    OP_SBC_ZP = 0xe5,
    OP_SBC_ZPX = 0xf5,
    OP_SBC_ABS = 0xed,
    OP_SBC_ABSX = 0xfd,
    OP_SBC_ABSY = 0xf9,
    OP_SBC_INDX = 0xe1,
    OP_SBC_INDY = 0xf1,
		
    OP_STA_ZP = 0x85,
    OP_STA_ZPX = 0x95,
    OP_STA_ABS = 0x8d,
    OP_STA_ABSX = 0x9d,
    OP_STA_ABSY = 0x99,
    OP_STA_INDX = 0x81,
    OP_STA_INDY = 0x91,
		
    OP_STX_ZP = 0x86,
    OP_STX_ZPY = 0x96,
    OP_STX_ABS = 0x8e,
    OP_STY_ZP = 0x84,
    OP_STY_ZPX = 0x94,
    OP_STY_ABS = 0x8c,

    // Illegal opcodes
    OP_I_LAX_IMM = 0xab,
    OP_I_LAX_ZP = 0xa7,
    OP_I_LAX_ZPY = 0xb7,
    OP_I_LAX_ABS = 0xaf,
    OP_I_LAX_ABSY = 0xbf,
    OP_I_LAX_INDX = 0xa3,
    OP_I_LAX_INDY = 0xb3,

    OP_I_SAX_ZP = 0x87,
    OP_I_SAX_ZPY = 0x97,
    OP_I_SAX_ABS = 0x8f,
    OP_I_SAX_INDX = 0x83,

    OP_I_SBC_IMM = 0xeb,

    OP_I_DCP_ZP = 0xc7,
    OP_I_DCP_ZPX = 0xd7,
    OP_I_DCP_ABS = 0xcf,
    OP_I_DCP_ABSX = 0xdf,
    OP_I_DCP_ABSY = 0xdb,
    OP_I_DCP_INDX = 0xc3,
    OP_I_DCP_INDY = 0xd3,

    OP_I_ISB_ZP = 0xe7,
    OP_I_ISB_ZPX = 0xf7,
    OP_I_ISB_ABS = 0xef,
    OP_I_ISB_ABSX = 0xff,
    OP_I_ISB_ABSY = 0xfb,
    OP_I_ISB_INDX = 0xe3,
    OP_I_ISB_INDY = 0xf3,

    OP_I_SLO_ZP = 0x07,
    OP_I_SLO_ZPX = 0x17,
    OP_I_SLO_ABS = 0x0f,
    OP_I_SLO_ABSX = 0x1f,
    OP_I_SLO_ABSY = 0x1b,
    OP_I_SLO_INDX = 0x03,
    OP_I_SLO_INDY = 0x13,

    OP_I_RLA_ZP = 0x27,
    OP_I_RLA_ZPX = 0x37,
    OP_I_RLA_ABS = 0x2f,
    OP_I_RLA_ABSX = 0x3f,
    OP_I_RLA_ABSY = 0x3b,
    OP_I_RLA_INDX = 0x23,
    OP_I_RLA_INDY = 0x33,

    OP_I_SRE_ZP = 0x47,
    OP_I_SRE_ZPX = 0x57,
    OP_I_SRE_ABS = 0x4f,
    OP_I_SRE_ABSX = 0x5f,
    OP_I_SRE_ABSY = 0x5b,
    OP_I_SRE_INDX = 0x43,
    OP_I_SRE_INDY = 0x53,

    OP_I_RRA_ZP = 0x67,
    OP_I_RRA_ZPX = 0x77,
    OP_I_RRA_ABS = 0x6f,
    OP_I_RRA_ABSX = 0x7f,
    OP_I_RRA_ABSY = 0x7b,
    OP_I_RRA_INDX = 0x63,
    OP_I_RRA_INDY = 0x73,

    OP_I_AHX_ABSY = 0x9f,
    OP_I_AHX_INDX = 0x93,
    OP_I_AHX_ZPY = 0x97,

};