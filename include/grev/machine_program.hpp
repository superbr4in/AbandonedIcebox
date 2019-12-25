#pragma once

#include <forward_list>
#include <map>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include <grev/machine_architecture.hpp>

namespace grev
{
    class machine_program
    {
        std::vector<std::uint8_t> data_;

        machine_architecture architecture_;

        std::uint32_t base_address_;
        std::uint32_t entry_point_address_;

        std::map<std::uint32_t, std::span<std::uint8_t const>> memory_segments_;

        std::forward_list<machine_program> imports_;

        std::unordered_map<std::string, std::uint32_t> export_map_;

        std::unordered_map<std::uint32_t, machine_program const*> import_map_;
        std::unordered_map<std::uint32_t, std::uint32_t> import_reals_;
        std::unordered_map<std::uint32_t, std::string> import_names_;

        machine_program();

    public:

        /*!
         *  Constructs a new program using unformatted machine code.
         *  \param [in] data Machine code data
         *  \param [in] architecture Instruction set architecture
         */
        machine_program(std::vector<std::uint8_t> data, machine_architecture architecture);

        machine_architecture architecture() const;
        std::uint32_t entry_point_address() const;

        std::span<std::uint8_t const> operator[](std::uint32_t address) const;

        std::optional<machine_program> load_imported(std::uint32_t address) const;

        std::string const& import_name(std::uint32_t address) const;

        template <typename Loader>
        static machine_program load(std::string const& file_name);

    private:

        static std::vector<std::uint8_t> load_data(std::string const& file_name);

        static std::string directory_name(std::string file_name);
    };
}

#ifndef LINT
#include <grev/machine_program.tpp>
#endif
