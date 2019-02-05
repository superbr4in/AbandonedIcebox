#pragma once

#include <memory>

#include <disassembler/architecture.hpp>
#include <disassembler/instruction.hpp>

/*!
 *  Disassembler for machine code instructions
 */
class disassembler
{
    std::shared_ptr<void const> handle_;

public:

    /*!
     *  Constructs a new disassembler.
     *  \param [in] architecture The instruction set architecture to interpret the machine code with.
     */
    explicit disassembler(architecture architecture);

    /*!
     *  Iteratively disassembles one machine code instruction.
     *  \param [in,out] code    View on the machine code to be disassembled. Updated to point to the machine code past this
     *                          instruction.
     *  \param [in,out] address The address of the machine code instruction. The updated value is the address past this
     *                          instruction.
     *  \exception std::invalid_argument The code range is empty.
     *  \returns A disassembled machine code instruction.
     */
    instruction operator()(std::basic_string_view<std::byte>* code, std::uint_fast64_t* address) const;
};
