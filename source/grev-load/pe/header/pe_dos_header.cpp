#include "reinterpret_copy.hpp"

#include "pe_dos_header.hpp"

namespace grev
{
    pe_dos_header pe_dos_header::inspect(std::span<std::uint8_t const>* const data_span)
    {
        pe_dos_header dos_header { };

        constexpr std::size_t    pos_pe_offset{60};
        reinterpret_copy(&dos_header.pe_offset,
              data_span->subspan(pos_pe_offset));

        constexpr std::size_t           size{64};
        *data_span = data_span->subspan(size);

        return dos_header;
    }
}
