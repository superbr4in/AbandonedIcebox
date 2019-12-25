#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <grev-lift/reil_disassembler.hpp>
#include <grev-load/pe_loader.hpp>
#include <grev/machine_process.hpp>

int main(int const argument_count, char const* const* const raw_arguments)
{
    std::vector<std::string> const arguments(raw_arguments, std::next(raw_arguments, argument_count));

    if (arguments.size() < 2)
    {
        auto const& command = arguments.front();

        std::cerr << command << " <file_name> [<patch_address>[:<patch_value>...] ...]" << std::endl;
        return 1;
    }

    auto const& file_name = arguments[1];

    if (std::ifstream file_stream(file_name); file_stream.good())
    {
        std::string magic(2, '\0');
        file_stream.read(magic.data(), magic.size());

        if (magic != "MZ")
        {
            std::cerr << "Invalid file format" << std::endl;
            return 1;
        }
    }
    else
    {
        std::cerr << "Invalid file name" << std::endl;
        return 1;
    }

    std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> patches;
    for (auto patch_string = std::next(arguments.begin(), 2); patch_string != arguments.end(); ++patch_string)
    {
        std::istringstream patch_stream{*patch_string};

        std::uint32_t patch_address;
        patch_stream >> std::hex >> patch_address;

        std::vector<std::uint8_t> patch_data;
        while (!patch_stream.eof() && !patch_stream.fail())
        {
            int patch_value;
            patch_stream.ignore();
            patch_stream >> std::hex >> patch_value;

            patch_data.push_back(static_cast<std::uint8_t>(patch_value));
        }

        patches.emplace(std::move(patch_address), std::move(patch_data));
    }

    auto program = grev::machine_program::load<grev::pe_loader>(file_name);

    grev::reil_disassembler const disassembler(program.architecture());

    grev::machine_process const process{program, grev::machine_environment{std::move(patches)}};
    for (auto const& [address, state] : process.execute(disassembler).import_calls)
    {
        std::cout << program.import_name(address) << std::endl;

        for (auto const& [key, value] : state)
        {
            if (key == grev::z3::expression(32, "R_ESP") || key == grev::z3::expression(32, "R_EBP"))
                continue;

            std::cout << '\t' << key.str() << " := " << value.str();

            if (auto const value_value = value.evaluate())
            {
                if (auto data = program[*value_value]; !data.empty())
                {
                    data = data.first(30);
                    std::cout << " \"" << reinterpret_cast<char const*>(data.data()) << "\"";
                }
            }

            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    return 0;
}
