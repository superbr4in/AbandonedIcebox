#pragma once

#include <string_view>

namespace grev
{
    struct pe_dos_header
    {
        std::uint32_t pe_offset;

        static pe_dos_header inspect(std::u8string_view* data_view);
    };
}
