#pragma once

#include <cstdint>
#include <vector>

struct instruction
{
    std::uint_fast64_t address;

    std::vector<std::byte> code;

    // TODO jump/call/register information
};

static_assert(std::is_destructible_v<instruction>);

static_assert(std::is_move_constructible_v<instruction>);
static_assert(std::is_move_assignable_v<instruction>);

static_assert(std::is_copy_constructible_v<instruction>);
static_assert(std::is_copy_assignable_v<instruction>);
