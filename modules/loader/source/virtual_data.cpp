#include <loader/virtual_data.hpp>

bool virtual_data::section::order_excluding::operator()(section const& section_1, section const& section_2) const
{
    return section_1.address + section_1.data.size() - 1 < section_2.address;
}

bool virtual_data::section::order_excluding::operator()(section const& section, std::uint_fast64_t const address) const
{
    return section.address + section.data.size() - 1 < address;
}
bool virtual_data::section::order_excluding::operator()(std::uint_fast64_t const address, section const& section) const
{
    return address < section.address;
}

void virtual_data::allocate(std::uint_fast64_t address, std::vector<std::byte> const& data)
{
    // TODO Align to page size

    section section
    {
        .address = address,
        .data = data
    };

    if (!sections_.insert(section).second)
        throw std::invalid_argument("Overlapping data regions");
}

std::basic_string_view<std::byte> virtual_data::operator[](std::uint_fast64_t address) const
{
    auto const section = sections_.lower_bound(address);

    if (section == sections_.upper_bound(address))
        throw std::invalid_argument("Invalid address");

    auto const section_data_position = address - section->address;

    return std::basic_string_view<std::byte>(&section->data.at(section_data_position),
        section->data.size() - section_data_position);
}
