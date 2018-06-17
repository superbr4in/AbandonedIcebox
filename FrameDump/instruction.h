#pragma once

#include "../Bin-Capstone/capstone.h"

#define INS_MAX_STR 64
#define INS_MAX_OPS 4

class instruction_x86
{
    uint16_t id_;

    uint8_t op_count_;

    uint8_t operand_types_[INS_MAX_OPS];

    uint64_t address_;

    char str_[INS_MAX_STR];

    uint64_t operand_values_[INS_MAX_OPS];

public:

    instruction_x86();
    instruction_x86(cs_insn cs_instruction);

    x86_insn identification() const;

    uint64_t address() const;

    std::string string() const;

    std::vector<std::pair<x86_op_type, uint64_t>> operands() const;
};
