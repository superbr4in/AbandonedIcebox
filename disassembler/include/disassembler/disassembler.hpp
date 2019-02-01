#pragma once

#include <memory>

#include "instruction.hpp"
#include "instruction_set_architecture.hpp"

class disassembler
{
    std::shared_ptr<void const> handle_;

public:

    explicit disassembler(instruction_set_architecture architecture);

    instruction operator()(std::basic_string_view<std::byte>* code, std::uint_fast64_t* address) const;
};

static_assert(std::is_destructible_v<disassembler>);

static_assert(std::is_move_constructible_v<disassembler>);
static_assert(std::is_move_assignable_v<disassembler>);

static_assert(std::is_copy_constructible_v<disassembler>);
static_assert(std::is_copy_assignable_v<disassembler>);
