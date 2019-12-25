#include "pe_header.hpp"

namespace grev
{
    pe_header pe_header::inspect(std::span<std::uint8_t const> data_span)
    {
        pe_header header { };

        auto const size = data_span.size();

        header.dos = pe_dos_header::inspect(&data_span);
        data_span = data_span.subspan(header.dos.pe_offset - (size - data_span.size()) + 4);

        // TODO PE/0/0 error (?)

        header.coff = pe_coff_header::inspect(&data_span);

        switch (header.coff.optional_header_size)
        {
        case 224:
            header.optional = pe_optional_header::inspect(&data_span);
            break;
        // TODO error (?)
        }

        for (std::size_t section_index{0}; section_index < header.coff.section_count; ++section_index)
            header.sections.push_back(pe_section_header::inspect(&data_span)); // TODO section_header.miscellaneous

        return header;
    }
}
