#pragma once

#include <vector>

#include <span>

#include "header/pe_coff_header.hpp"
#include "header/pe_dos_header.hpp"
#include "header/pe_optional_header.hpp"
#include "header/pe_section_header.hpp"

namespace grev
{
    struct pe_header
    {
        pe_dos_header dos;
        pe_coff_header coff;
        pe_optional_header optional;

        std::vector<pe_section_header> sections;

        static pe_header inspect(std::span<std::uint8_t const> data_span);
    };
}
