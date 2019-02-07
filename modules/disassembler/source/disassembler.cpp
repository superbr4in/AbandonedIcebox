#include <sstream>

#include <capstone/capstone.h>
#include <disassembler/disassembler.hpp>

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
        throw std::logic_error(message.str());                                                                                 \
    }                                                                                                                          \
}

struct disassembler::handle
{
    csh cs;

    explicit handle(instruction_set_architecture const architecture) :
        cs()
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

        HANDLE_CS_ERROR(
            cs_open(cs_architecture, cs_mode, &cs));
        HANDLE_CS_ERROR(
            cs_option(cs, CS_OPT_DETAIL, CS_OPT_ON));
    }
    ~handle()
    {
        cs_close(&cs);
    }
};

disassembler::disassembler(instruction_set_architecture const architecture) :
    handle_(std::make_shared<handle>(architecture)) { }

instruction disassembler::operator()(std::uint_fast64_t* const address, std::basic_string_view<std::byte>* const code) const
{
    struct cs_insn_deleter
    {
        void operator()(cs_insn* const cs_instruction)
        {
            cs_free(cs_instruction, 1);
        }
    };

    if (code->empty())
        throw std::invalid_argument("Empty code range");

    auto cs_address = *address;
    auto const* cs_code =
        reinterpret_cast<std::uint_fast8_t const*>(code->data()); // NOLINT [cppcoreguidelines-pro-type-reinterpret-cast]
    auto cs_size = code->size();

    std::unique_ptr<cs_insn, cs_insn_deleter> const cs_instruction(cs_malloc(handle_->cs));

    if (!cs_disasm_iter(handle_->cs, &cs_code, &cs_size, &cs_address, cs_instruction.get()))
        throw std::runtime_error("Invalid instruction");

    HANDLE_CS_ERROR(
        cs_errno(handle_->cs));

    *address = cs_address;
    *code = std::basic_string_view<std::byte>(
        reinterpret_cast<std::byte const*>(cs_code), // NOLINT [cppcoreguidelines-pro-type-reinterpret-cast]
        cs_size);

    return instruction
    {
        .id = cs_instruction->id,

        .address = cs_instruction->address,
        .size = cs_instruction->size
    };
}

static_assert(std::is_destructible_v<disassembler>);

static_assert(std::is_move_constructible_v<disassembler>);
static_assert(std::is_move_assignable_v<disassembler>);

static_assert(std::is_copy_constructible_v<disassembler>);
static_assert(std::is_copy_assignable_v<disassembler>);
