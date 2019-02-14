#pragma once

#include <unordered_map>
#include <vector>

#include <disassembler/instruction.hpp>

struct executable
{
    instruction_set_architecture architecture;

    std::uint_fast64_t entry_point_address;
    std::unordered_map<std::uint_fast64_t, std::vector<std::byte>> memory_sections;

public:

    static std::unique_ptr<executable> load_binary(std::istream& stream);
};

struct pe_binary : executable
{
    explicit pe_binary(std::istream& stream);
};
struct elf_binary : executable
{
    explicit elf_binary(std::istream& stream);
};
