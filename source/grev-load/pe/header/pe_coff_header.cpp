#include "reinterpret_copy.hpp"

#include "pe_coff_header.hpp"

namespace grev
{
    pe_coff_header pe_coff_header::inspect(std::span<std::uint8_t const>* const data_span)
    {
        pe_coff_header coff_header { };

        constexpr std::size_t     pos_machine_id{0};
        reinterpret_copy(&coff_header.machine_id,
               data_span->subspan(pos_machine_id));

        constexpr std::size_t     pos_section_count{2};
        reinterpret_copy(&coff_header.section_count,
               data_span->subspan(pos_section_count));

        constexpr std::size_t     pos_optional_header_size{16};
        reinterpret_copy(&coff_header.optional_header_size,
               data_span->subspan(pos_optional_header_size));

        constexpr std::size_t           size{20};
        *data_span = data_span->subspan(size);

        return coff_header;
    }
}
