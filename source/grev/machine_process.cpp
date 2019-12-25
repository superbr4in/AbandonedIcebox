#include <climits>

#include <grev/machine_process.hpp>

namespace grev
{
    machine_process::machine_process(machine_program const& program, std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> patches) :
        program_(program),
        patches_(std::move(patches)) { }

    execution_state machine_process::memory_patch(std::unordered_set<z3::expression> const& dependencies) const
    {
        execution_state memory_patch;
        for (auto const& dependency : dependencies)
        {
            auto const dependency_reference = dependency.reference();

            // Needs to be a memory access
            if (!dependency_reference)
                continue;

            auto const address = dependency_reference->evaluate();

            // Needs to be an unambiguous number
            if (!address)
                continue;

            auto const value_width = dependency.width();
            auto const value_width_bytes = (value_width - 1) / CHAR_BIT + 1; // TODO Possible underflow (?)

            std::span<std::uint8_t const> data;
            if (auto const patch_entry = patches_.find(*address); patch_entry != patches_.end())
                data = patch_entry->second;
            else
                data = program_[*address];

            if (data.size() < value_width_bytes)
                continue;

            std::uint32_t value { };
            for (data = data.first(value_width_bytes); !data.empty(); data = data.first(data.size() - 1)) // Little endian
                value = (value << std::uint8_t{CHAR_BIT}) + *std::prev(data.end());

            memory_patch.define(dependency, z3::expression(value_width, value));
        }

        return memory_patch;
    }
}

static_assert(std::is_destructible_v<grev::machine_process>);

static_assert(std::is_copy_constructible_v<grev::machine_process>);
static_assert(std::is_nothrow_move_constructible_v<grev::machine_process>);

static_assert(!std::is_copy_assignable_v<grev::machine_process>); // TODO
static_assert(!std::is_nothrow_move_assignable_v<grev::machine_process>); // TODO
