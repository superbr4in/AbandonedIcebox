#pragma once

#include <set>
#include <vector>

class virtual_data
{
    struct section
    {
        struct order_excluding
        {
            using is_transparent = std::true_type;

            bool operator()(section const& section_1, section const& section_2) const;

            bool operator()(section const& section, std::uint_fast64_t address) const;
            bool operator()(std::uint_fast64_t address, section const& section) const;
        };

        std::uint_fast64_t address;
        std::vector<std::byte> data;
    };

    std::set<section, section::order_excluding> sections_;

public:

    void allocate(std::uint_fast64_t address, std::vector<std::byte> const& data);

    std::basic_string_view<std::byte> operator[](std::uint_fast64_t address) const;
};
