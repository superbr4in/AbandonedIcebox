#pragma once

#include <span>

namespace grev
{
    struct pe_section_header
    {
        std::uint32_t miscellaneous;
        std::uint32_t relative_section_address;
        std::uint32_t section_size;
        std::uint32_t section_offset;

        static pe_section_header inspect(std::span<std::uint8_t const>* data_span);
    };
}
