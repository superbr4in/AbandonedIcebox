#pragma once

#include <span>

namespace grev
{
    struct pe_coff_header
    {
        std::uint16_t machine_id;
        std::uint16_t section_count;
        std::uint16_t optional_header_size;

        static pe_coff_header inspect(std::span<std::uint8_t const>* data_span);
    };
}
