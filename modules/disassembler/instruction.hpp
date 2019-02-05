#pragma once

#include <string_view>

/*!
 *  Disassembled machine code instruction
 */
struct instruction
{
    std::uint_fast64_t address;

    std::basic_string_view<std::byte> code;

    // TODO jump/call/register/...
};
