#pragma once

#include <grev/execution_state.hpp>
#include <grev/machine_program.hpp>

namespace grev
{
    class machine_environment
    {
        std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> patches_;

    public:

        explicit machine_environment(std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> patches);

        // TODO machine_program?
        execution_state patch(machine_program const& program, std::unordered_set<z3::expression> const& dependencies) const;
    };
}
