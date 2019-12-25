#pragma once

#include <list>
#include <span>

#include <grev/execution.hpp>
#include <grev/machine_architecture.hpp>

struct _reil_arg_t;
struct _reil_inst_t;

namespace grev
{
    class reil_disassembler
    {
        machine_architecture architecture_;
        void* handle_;

        mutable std::list<_reil_inst_t> instructions_;

        mutable std::forward_list<execution_path> update_paths_;
        mutable execution_state temporary_state_;

    public:

        explicit reil_disassembler(machine_architecture architecture);
        ~reil_disassembler();

        reil_disassembler(reil_disassembler const& other);
        reil_disassembler(reil_disassembler&& other) noexcept;

        reil_disassembler& operator=(reil_disassembler other) noexcept;

        std::forward_list<execution_path> operator()(std::uint32_t* address, std::span<std::uint8_t const>* code) const;

    private:

        execution_path& path() const;

        void jump(_reil_arg_t const& argument, z3::expression value) const;

        z3::expression get_value(_reil_arg_t const& argument) const;
        void set_value(_reil_arg_t const& argument, z3::expression value) const;
    };
}
