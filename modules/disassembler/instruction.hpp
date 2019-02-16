#pragma once

#include <unordered_set>
#include <vector>

/*!
 *  Instruction set architecture
 */
enum class instruction_set_architecture
{
    x86_32,
    x86_64

    // TODO
};

/*!
 *  Disassembled machine code instruction
 */
struct instruction
{
    struct order_by_address
    {
        using is_transparent = std::true_type;

        bool operator()(instruction const& instruction_1, instruction const& instruction_2) const;

        bool operator()(instruction const& instruction, std::uint_fast64_t address) const;
        bool operator()(std::uint_fast64_t address, instruction const& instruction) const;
    };

    struct operand
    {
        // TODO
    };

    instruction_set_architecture architecture;

    unsigned id;

    std::uint_fast64_t address;
    std::size_t size;

    std::vector<operand> operands;

    std::unordered_set<std::uint_fast64_t> get_called_addresses() const; // TODO Context
    std::unordered_set<std::uint_fast64_t> get_jumped_addresses() const; //
};
