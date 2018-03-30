#pragma once

#include "loader.h"

struct instruction_info
{
    uint32_t id;

    uint64_t address;

    uint16_t size;
    
    uint8_t bytes[16];
    
    char mnemonic[32];
    char operands[160];
};

struct register_info
{
    uint64_t registers[9];
};

class debugger
{
    csh cs_ { };
    uc_engine* uc_ { };

    uint64_t scale_ { };

    std::array<int, 9> regs_ { }; // TODO: Constantly 9 registers?

public:
    
    int load(const loader& l, std::vector<char> bytes);
    int unload();

    uint64_t scale() const;

    int step(instruction_info& ins_info) const;

    int reg(register_info& reg_info) const;
};