#include "reinterpret_copy.hpp"

#include "pe_section_header.hpp"

namespace grev
{
    pe_section_header pe_section_header::inspect(std::span<std::uint8_t const>* const data_span)
    {
        pe_section_header section_header { };

        constexpr std::size_t        pos_miscellaneous{8};
        reinterpret_copy(&section_header.miscellaneous,
                  data_span->subspan(pos_miscellaneous));

        constexpr std::size_t        pos_relative_section_address{12};
        reinterpret_copy(&section_header.relative_section_address,
                  data_span->subspan(pos_relative_section_address));

        constexpr std::size_t        pos_section_size{16};
        reinterpret_copy(&section_header.section_size,
                  data_span->subspan(pos_section_size));

        constexpr std::size_t        pos_section_offset{20};
        reinterpret_copy(&section_header.section_offset,
                  data_span->subspan(pos_section_offset));

        constexpr std::size_t           size{40};
        *data_span = data_span->subspan(size);

        return section_header;
    }
}
