#pragma once

#include <regex>

inline std::runtime_error unknown_architecture() noexcept
{
    return std::runtime_error("Unknown architecture");
}
inline std::runtime_error invalid_binary_format() noexcept
{
    return std::runtime_error("Invalid binary format");
}

inline std::vector<std::byte> extract_bytes(std::string::const_iterator const& it, std::string::size_type const size)
{
    std::vector<std::byte> bytes(size);
    std::transform(it, it + size, bytes.begin(),
        [](auto const value)
        {
            return std::byte(value);
        });

    return bytes;
}

template <std::size_t N>
std::array<std::uint64_t, N> regex_parse_values(std::string const& regex_pattern, std::string::const_iterator* const it)
{
    std::smatch match_results;
    if (!std::regex_search(*it, std::string::const_iterator(), match_results,
            std::regex(regex_pattern, std::regex_constants::ECMAScript), std::regex_constants::match_continuous))
        throw invalid_binary_format();

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
