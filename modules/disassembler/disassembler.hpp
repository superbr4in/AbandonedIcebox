#pragma once

#include <memory>

#include <disassembler/instruction.hpp>

/*!
 *  Disassembler for machine code instructions
 */
class disassembler
{
    struct handle;
    std::shared_ptr<handle> handle_;

public:

    /*!
     *  Constructs a new disassembler.
     *  \param [in] architecture The instruction set architecture to interpret the machine code with.
     */
    explicit disassembler(instruction_set_architecture architecture);

    /*!
     *  Iteratively disassembles one machine code instruction.
     *  \param [in,out] code    View on the machine code to be disassembled. Updated to point to the machine code past this
     *                          instruction.
     *  \param [in,out] address The address of the machine code instruction. The updated value is the address past this
     *                          instruction.
     *  \exception std::invalid_argument The code range is empty.
     *  \exception std::runtime_error    Encountered an invalid instruction.
     *  \returns A disassembled machine code instruction.
     */
    instruction iterate(std::uint_fast64_t* address, std::basic_string_view<std::byte>* code) const;
};
