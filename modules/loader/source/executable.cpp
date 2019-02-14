#include <istream>

#include <loader/executable.hpp>

std::unique_ptr<executable> executable::load_binary(std::istream& stream)
{
    std::string magic(4, '\0');

    stream.read(magic.data(), magic.size());
    stream.seekg(-magic.size(), std::ios::cur);

    if (magic.substr(0, 2) == "MZ")
        return std::make_unique<pe_binary>(stream);

/* TODO
    if (magic.front() == '\x7F' && magic.substr(1, 3) == "ELF")
        return std::make_unique<elf_binary>(stream);
*/

    throw std::runtime_error("Unknown binary format");
}
