#include <climits>

#include <grev/machine_process.hpp>

namespace grev
{
    machine_process::machine_process(machine_program const& program, machine_environment environment) :
        program_{program},
        environment_{std::move(environment)} { }
}

static_assert(std::is_destructible_v<grev::machine_process>);

static_assert(std::is_copy_constructible_v<grev::machine_process>);
static_assert(std::is_nothrow_move_constructible_v<grev::machine_process>);

static_assert(!std::is_copy_assignable_v<grev::machine_process>); // TODO
static_assert(!std::is_nothrow_move_assignable_v<grev::machine_process>); // TODO
