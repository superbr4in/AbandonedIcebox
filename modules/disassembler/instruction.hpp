#pragma once

#include <memory>

/*!
 *  Disassembled machine code instruction
 */
struct instruction
{
    unsigned id;

    std::uint_fast64_t address;
    std::size_t size;

    // TODO jump/call/register/...
};

/*!
 *  Instruction set architecture
 */
enum class instruction_set_architecture
{
    x86_32,
    x86_64

    // TODO
};
