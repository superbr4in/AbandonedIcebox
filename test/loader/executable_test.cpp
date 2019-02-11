#include <catch2/catch.hpp>
#include <libgen.h>
#include <loader/executable.hpp>

std::string get_absolute_path(std::string const& file_name)
{
    return std::string(dirname(std::string(__FILE__).data())) + "/" + file_name;
}

TEST_CASE("executable(std::string const&)")
{
    std::string file_name;

    instruction_set_architecture architecture;
    std::uint_fast64_t entry_point_address;
    std::vector<std::pair<std::uint_fast64_t, std::size_t>> memory_section_infos;

    SECTION("hello_world_32.exe")
    {
        file_name = "hello_world_32.exe";

        architecture = instruction_set_architecture::x86_32;
        entry_point_address = 0x44EE45;
        memory_section_infos =
        {
            { 0x400000,    860 },
            { 0x401000, 305869 },
            { 0x44C000, 659456 },
            { 0x4ED000, 148992 },
            { 0x512000,   9636 },
            { 0x515000,   3072 },
            { 0x516000,    512 },
            { 0x517000,    512 },
            { 0x518000,   1536 },
            { 0x519000,  22528 }
        };
    }
    SECTION("hello_world_64.exe")
    {
        file_name = "hello_world_64.exe";

        architecture = instruction_set_architecture::x86_64;
        entry_point_address = 0x140061230;
        memory_section_infos =
        {
            { 0x140000000,    916 },
            { 0x140001000, 389206 },
            { 0x140061000, 830464 },
            { 0x14012C000, 192512 },
            { 0x14015B000,  12952 },
            { 0x14015F000,  43008 },
            { 0x14016A000,   4096 },
            { 0x14016B000,    512 },
            { 0x14016C000,    512 },
            { 0x14016D000,   1536 },
            { 0x14016E000,   6656 }
        };
    }

    executable const executable(get_absolute_path(file_name));

    CHECK(executable.architecture == architecture);
    CHECK(executable.entry_point_address == entry_point_address);
    CHECK(executable.memory_sections.size() == memory_section_infos.size());
    for (auto const& [address, size] : memory_section_infos)
        CHECK(executable.memory_sections.at(address).size() == size);
}
