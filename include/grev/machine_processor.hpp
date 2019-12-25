#pragma once

#include <grev/execution.hpp>
#include <grev/machine_environment.hpp>
#include <grev/machine_program.hpp>

namespace grev
{
    template <typename Disassembler>
    class machine_processor
    {
        Disassembler disassembler_;

        machine_environment environment_;

    public:

        machine_processor(Disassembler disassembler, machine_environment environment);

        execution execute(machine_program const& program) const;
    };
}

#ifndef LINT
#include <grev/machine_processor.tpp>
#endif
