#pragma once

#include <cstdint>
#include <vector>

/*!
 *  Disassembled machine code instruction
 */
struct instruction
{
    std::uint_fast64_t address;

    std::vector<std::byte> code;

    // TODO jump/call/register/...
};
