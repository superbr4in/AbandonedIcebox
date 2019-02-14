#include <iterator>

#include <loader/executable.hpp>

#include "helper.hpp"

pe_binary::pe_binary(std::istream& stream)
{
    std::string const data(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char> { });

    auto it = data.begin();

    auto const& [pe_offset] =
        regex_parse_values<1>("MZ[^]{58}([^]{4})", &it);
    it += pe_offset - 64;

    auto const& [machine_id, section_count, optional_header_size, relative_entry_point_address] =
        regex_parse_values<4>("PE\\0\\0([^]{2})([^]{2})[^]{12}([^]{2})[^]{18}([^]{4})", &it);
    it += 4;

    switch (machine_id)
    {
    case 332:
        architecture = instruction_set_architecture::x86_32;
        break;
    case 34404:
        architecture = instruction_set_architecture::x86_64;
        break;
    default:
        throw unknown_architecture();
    }

    std::size_t shift;
    switch (optional_header_size)
    {
    case 224:
        shift = 4;
        break;
    case 240:
        shift = 0;
        break;
    default:
        throw invalid_binary_format();
    }

    auto const& [base_address] =
        regex_parse_values<1>("[^]{" + std::to_string(shift) + "}([^]{" + std::to_string(8 - shift) + "})", &it);
    it += optional_header_size - 24;

    entry_point_address = base_address + relative_entry_point_address;

    memory_sections.emplace(base_address,
        extract_bytes(data.begin(), pe_offset + 20 + optional_header_size + section_count * 40));

    for (std::size_t section_index = 0; section_index < section_count; ++section_index)
    {
        auto const& [miscellaneous, relative_section_address, section_size, section_offset] =
            regex_parse_values<4>("([^]{4})([^]{4})([^]{4})([^]{4})", &it);
        it += 24;

        memory_sections.emplace(base_address + relative_section_address,
            miscellaneous > section_size
                ? std::vector<std::byte>(miscellaneous, std::byte(0))
                : extract_bytes(data.begin() + section_offset, section_size));
    }
}

static_assert(std::is_destructible_v<pe_binary>);

static_assert(std::is_move_constructible_v<pe_binary>);
static_assert(std::is_move_assignable_v<pe_binary>);

static_assert(std::is_copy_constructible_v<pe_binary>);
static_assert(std::is_copy_assignable_v<pe_binary>);
