#pragma once

#include <grev/execution.hpp>
#include <grev/machine_environment.hpp>
#include <grev/machine_program.hpp>

namespace grev
{
    class machine_process
    {
        machine_program const& program_;

        machine_environment environment_;

    public:

        machine_process(machine_program const& program, machine_environment environment);

        template <typename Disassembler>
        execution execute(Disassembler const& disassembler) const;
    };
}

#ifndef LINT
#include <grev/machine_process.tpp>
#endif
