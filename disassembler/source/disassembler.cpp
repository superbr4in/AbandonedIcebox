#include <cstring>
#include <sstream>

#include <disassembler/disassembler.hpp>

#include "cs_adapter.hpp"

#define HANDLE_CS_ERROR(cs_call)                            \
{                                                           \
    cs_err const error_code = cs_call;                      \
    if (error_code != CS_ERR_OK)                            \
    {                                                       \
        std::ostringstream message;                         \
        message << cs_strerror(error_code)                  \
            << std::endl << #cs_call                        \
            << std::endl << __FILE__ << ':' << __LINE__;    \
                                                            \
        throw std::runtime_error(message.str());            \
    }                                                       \
}

disassembler::disassembler(instruction_set_architecture const architecture)
{
    cs_arch cs_architecture;
    cs_mode cs_mode;
    switch (architecture)
    {
    case instruction_set_architecture::x86_32:
        cs_architecture = CS_ARCH_X86;
        cs_mode = CS_MODE_32;
        break;
    case instruction_set_architecture::x86_64:
        cs_architecture = CS_ARCH_X86;
        cs_mode = CS_MODE_64;
        break;
    }

    csh cs_handle;
    HANDLE_CS_ERROR(
        cs_open(cs_architecture, cs_mode, &cs_handle));
    HANDLE_CS_ERROR(
        cs_option(cs_handle, CS_OPT_DETAIL, CS_OPT_ON));
    handle_ = std::shared_ptr<void const>(
        reinterpret_cast<void const*>(cs_handle), // NOLINT [cppcoreguidelines-pro-type-reinterpret-cast]
        [](void const* const handle)
        {
            csh cs_handle = reinterpret_cast<csh>(handle); // NOLINT [cppcoreguidelines-pro-type-reinterpret-cast]
            cs_close(&cs_handle);
        });
}

instruction disassembler::operator()(std::basic_string_view<std::byte>* const code, std::uint_fast64_t* const address) const
{
    auto const cs_handle =
        reinterpret_cast<csh>(handle_.get()); // NOLINT [cppcoreguidelines-pro-type-reinterpret-cast]

    auto const ref_code = *code;
    auto const ref_address = *address;

    instruction instruction;
    if (auto const cs_instruction = cs::malloc(cs_handle);
        cs::disasm_iter(cs_handle, code, address, cs_instruction.get()))
    {
        instruction.address = cs_instruction->address;
        instruction.code.resize(cs_instruction->size);
        std::memcpy(instruction.code.data(), std::begin(cs_instruction->bytes), cs_instruction->size);
    }
    else
    {
        HANDLE_CS_ERROR(
            cs_errno(cs_handle));

        instruction.address = ref_address;
        instruction.code = { *ref_code.data() };

        *code = std::basic_string_view<std::byte>(
            ref_code.data() + 1, // NOLINT [cppcoreguidelines-pro-bounds-pointer-arithmetic]
            ref_code.size() - 1);
        *address = ref_address + 1;
    }

    return instruction;
}
