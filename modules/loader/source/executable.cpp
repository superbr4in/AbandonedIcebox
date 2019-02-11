#include <algorithm>
#include <climits>
#include <fstream>
#include <regex>

#include <loader/executable.hpp>

template <std::size_t N>
std::array<std::uint64_t, N> regex_parse_values(std::string const& regex_pattern, std::string::const_iterator* const it)
{
    std::smatch match_results;
    if (!std::regex_search(*it, std::string::const_iterator(), match_results,
            std::regex(regex_pattern, std::regex_constants::ECMAScript), std::regex_constants::match_continuous))
        throw std::runtime_error("Corrupt data format");

    if (match_results.size() != N + 1)
        throw std::logic_error("Invalid regex match");

    std::array<std::uint64_t, N> values { };
    std::transform(match_results.begin() + 1, match_results.end(), values.begin(),
        [](auto const& match)
        {
            std::string match_data(match.first, match.second);
            match_data.resize(sizeof(std::uint64_t));

            return
                // NOLINTNEXTLINE [cppcoreguidelines-pro-type-reinterpret-cast]
                *reinterpret_cast<std::uint64_t const*>(match_data.data());
        });

    *it += match_results.length(0);
    return values;
}

std::vector<std::byte> extract_bytes(std::string const& data, std::string::size_type const offset,
    std::string::size_type const size)
{
    auto const data_begin = data.begin() + offset;

    std::vector<std::byte> bytes(size);
    std::transform(data_begin, data_begin + size, bytes.begin(),
        [](auto const value)
        {
            return std::byte(value);
        });

    return bytes;
}

void load_pe(std::string const& data, executable* executable)
{
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
        executable->architecture = instruction_set_architecture::x86_32;
        break;
    case 34404:
        executable->architecture = instruction_set_architecture::x86_64;
        break;
    default:
        throw std::runtime_error("Unknown architecture");
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
        throw std::runtime_error("Corrupt data format");
    }

    auto const& [base_address] =
        regex_parse_values<1>("[^]{" + std::to_string(shift) + "}([^]{" + std::to_string(8 - shift) + "})", &it);
    it += optional_header_size - 24;

    executable->entry_point_address = base_address + relative_entry_point_address;

    executable->memory_sections.emplace(base_address,
        extract_bytes(data, 0, pe_offset + 20 + optional_header_size + section_count * 40));

    for (std::size_t section_index = 0; section_index < section_count; ++section_index)
    {
        auto const& [miscellaneous, relative_section_address, section_size, section_offset] =
            regex_parse_values<4>("([^]{4})([^]{4})([^]{4})([^]{4})", &it);
        it += 24;

        executable->memory_sections.emplace(base_address + relative_section_address,
            miscellaneous > section_size
                ? std::vector<std::byte>(miscellaneous, std::byte(0))
                : extract_bytes(data, section_offset, section_size));
    }
}

executable::executable(std::string const& file_name) :
    architecture(), entry_point_address()
{
    std::ifstream file_stream(file_name, std::ios::binary);

    if (!file_stream)
        throw std::runtime_error("Invalid file");

    std::string const data(
        std::istreambuf_iterator<char>(file_stream),
        std::istreambuf_iterator<char> { });

    if (data.substr(0, 2) == "MZ")
    {
        load_pe(data, this);
        return;
    }

    // TODO

    throw std::runtime_error("Unknown file format");
}
