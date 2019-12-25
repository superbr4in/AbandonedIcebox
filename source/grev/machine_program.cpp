#include <fstream>

#include <libgen.h>

#include <grev/machine_program.hpp>

namespace grev
{
    machine_program::machine_program() = default;

    machine_program::machine_program(std::vector<std::uint8_t> data, machine_architecture architecture) :
        data_(std::move(data)),
        architecture_(std::move(architecture)),
        entry_point_address_(0)
    {
        memory_segments_.emplace(0, data_);
    }

    machine_architecture machine_program::architecture() const
    {
        return architecture_;
    }
    std::uint32_t machine_program::entry_point_address() const
    {
        return entry_point_address_;
    }

    std::span<std::uint8_t const> machine_program::operator[](std::uint32_t const address) const
    {
        std::uint32_t segment_address;
        std::span<std::uint8_t const> segment_data;

        for (auto segment = memory_segments_.begin();; ++segment)
        {
            if (segment == memory_segments_.end())
                return { };

            std::tie(segment_address, segment_data) = *segment;

            if (segment_address <= address && address < segment_address + segment_data.size())
                break;
        }

        return segment_data.subspan(address - segment_address);
    }

    std::optional<machine_program> machine_program::load_imported(std::uint32_t const address) const
    {
        if (auto const import_map_entry = import_map_.find(address); import_map_entry != import_map_.end())
        {
            auto import = *import_map_entry->second;
            import.entry_point_address_ = import_reals_.at(address);

            return import;
        }

        return std::nullopt;
    }

    std::string const& machine_program::import_name(std::uint32_t address) const
    {
        return import_names_.at(address);
    }

    std::vector<std::uint8_t> machine_program::load_data(std::string const& file_name)
    {
        std::ifstream file_stream(file_name, std::ios::ate);

        std::vector<std::uint8_t> data(file_stream.tellg(), '\0');

        file_stream.seekg(0);
        file_stream.read(reinterpret_cast<char*>(data.data()), data.size());

        return data;
    }

    std::string machine_program::directory_name(std::string file_name)
    {
        return std::string{dirname(file_name.data())} + '/';
    }
}
