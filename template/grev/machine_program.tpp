#include <functional>

#ifdef LINT
#include <grev/machine_program.hpp>
#endif

namespace grev
{
    template <typename Loader>
    machine_program machine_program::load(std::string const& file_name)
    {
        auto data = load_data(file_name);

        Loader const loader(data);

        machine_program program;

        program.data_ = std::move(data);

        program.architecture_ = loader.architecture();

        program.base_address_ = loader.base_address();
        program.entry_point_address_ = loader.entry_point_address();

        for (std::span<std::uint8_t const> data_span(program.data_.data(), program.data_.size()); auto const& [address, bounds] : loader.memory_segments())
            program.memory_segments_.emplace(address, data_span.subspan(bounds.first, bounds.second));

        program.export_map_ = loader.export_map(std::bind(&machine_program::operator[], &program, std::placeholders::_1));

        if (auto const imports_address = loader.imports_address())
        {
            for (auto const& import_descriptor : loader.import_descriptors(program[*imports_address]))
            {
                std::string const idn{reinterpret_cast<char const*>(program[import_descriptor.name_address].data())};
                program.imports_.push_front(load<Loader>(directory_name(file_name) + idn));

                for (auto const& origin : loader.import_origins(program[import_descriptor.origin_address]))
                {
                    auto import = loader.import(program[origin]);
                    auto real = program.imports_.front().export_map_.at(import.name);

                    program.import_map_.emplace(origin - program.base_address_, &program.imports_.front());
                    program.import_map_.emplace(real, &program.imports_.front());

                    import.address += program.imports_.front().base_address_;
                    program.import_reals_.emplace(origin - program.base_address_, real);
                    program.import_reals_.emplace(real, real);

                    program.import_names_.emplace(origin - program.base_address_, idn + " -> " + import.name);
                    program.import_names_.emplace(real, idn + " -> " + import.name);
                }
            }
        }

        return program;
    }
}

#ifdef LINT
#include <grev-load/pe_loader.hpp>
template grev::machine_program grev::machine_program::load<grev::pe_loader>(std::string const&);
#endif
