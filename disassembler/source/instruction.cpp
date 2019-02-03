#include <disassembler/instruction.hpp>

static_assert(std::is_destructible_v<instruction>);

static_assert(std::is_move_constructible_v<instruction>);
static_assert(std::is_move_assignable_v<instruction>);

static_assert(std::is_copy_constructible_v<instruction>);
static_assert(std::is_copy_assignable_v<instruction>);
