#pragma once

#include <span>

namespace grev
{
    struct pe_dos_header
    {
        std::uint32_t pe_offset;

        static pe_dos_header inspect(std::span<std::uint8_t const>* data_span);
    };
}
