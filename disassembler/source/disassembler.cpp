#include <cstring>
#include <sstream>

#include <disassembler/disassembler.hpp>

#include "cs_adapter.hpp"

#define HANDLE_CS_ERROR(cs_call)                                                                                               \
{                                                                                                                              \
    cs_err const cs_error = cs_call;                                                                                           \
    if (cs_error != CS_ERR_OK)                                                                                                 \
    {                                                                                                                          \
        std::ostringstream message;                                                                                            \
        message          << __FILE__ << ':' << __LINE__ << ':'                                                                 \
            << std::endl << #cs_call                                                                                           \
            << std::endl << cs_strerror(cs_error);                                                                             \
                                                                                                                               \
        throw std::runtime_error(message.str());                                                                               \
    }                                                                                                                          \
}

disassembler::disassembler(architecture const architecture)
{
    cs_arch cs_architecture;
    cs_mode cs_mode;
    switch (architecture)
    {
    case architecture::x86_32:
        cs_architecture = CS_ARCH_X86;
        cs_mode = CS_MODE_32;
        break;
    case architecture::x86_64:
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
    if (code->empty())
        throw std::invalid_argument("The code range cannot be empty.");

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

        *code = std::basic_string_view<std::byte>(
            ref_code.data() + 1, // NOLINT [cppcoreguidelines-pro-bounds-pointer-arithmetic]
            ref_code.size() - 1);
        *address = ref_address + 1;

        instruction.address = ref_address;
        instruction.code = { *ref_code.data() };
    }

    return instruction;
}

static_assert(std::is_destructible_v<disassembler>);

static_assert(std::is_move_constructible_v<disassembler>);
static_assert(std::is_move_assignable_v<disassembler>);

static_assert(std::is_copy_constructible_v<disassembler>);
static_assert(std::is_copy_assignable_v<disassembler>);
