#pragma once

#include <span>

namespace grev
{
    struct pe_optional_header
    {
        std::uint32_t relative_entry_point_address;
        std::uint32_t base_address;
        std::uint32_t relative_exports_address;
        std::uint32_t relative_imports_address;

        static pe_optional_header inspect(std::span<std::uint8_t const>* data_span);
    };
}
