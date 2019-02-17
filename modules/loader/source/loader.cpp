#include <istream>

#include <loader/loader.hpp>

#include "pe_loader.hpp"

void load_binary(std::istream& stream, instruction_set_architecture* const architecture,
    std::uint_fast64_t* const entry_point_address, virtual_data* const data)
{
    std::string magic(4, '\0');

    stream.read(magic.data(), magic.size());
    stream.seekg(-magic.size(), std::ios::cur);

    if (magic.substr(0, 2) == "MZ")
        return load_pe_binary(stream, architecture, entry_point_address, data);

/* TODO
    if (magic.front() == '\x7F' && magic.substr(1, 3) == "ELF")
        return std::make_unique<elf_binary>(stream);
*/

    throw std::runtime_error("Unknown binary format");
}
