#pragma once

#include <disassembler/instruction.hpp>
#include <loader/virtual_data.hpp>

void load_pe_binary(std::istream& stream, instruction_set_architecture* architecture, std::uint_fast64_t* entry_point_address,
    virtual_data* data);
