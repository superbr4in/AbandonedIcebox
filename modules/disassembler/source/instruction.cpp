#include <capstone/capstone.h>
#include <disassembler/instruction.hpp>

bool instruction::order_by_address::operator()(instruction const& instruction_1, instruction const& instruction_2) const
{
    return instruction_1.address < instruction_2.address;
}

bool instruction::order_by_address::operator()(instruction const& instruction, std::uint_fast64_t const address) const
{
    return instruction.address < address;
}
bool instruction::order_by_address::operator()(std::uint_fast64_t const address, instruction const& instruction) const
{
    return address < instruction.address;
}

std::unordered_set<std::uint_fast64_t> instruction::get_called_addresses() const
{
    std::unordered_set<std::uint_fast64_t> addresses;

    switch (architecture)
    {
    case instruction_set_architecture::x86_32:
    case instruction_set_architecture::x86_64:
        switch (id)
        {
        case X86_INS_CALL:
            // TODO
            break;
        };
        break;
    default:
        throw std::logic_error("Invalid architecture");
    }

    return addresses;
}
std::unordered_set<std::uint_fast64_t> instruction::get_jumped_addresses() const
{
    std::unordered_set<std::uint_fast64_t> addresses;

    switch (architecture)
    {
    case instruction_set_architecture::x86_32:
    case instruction_set_architecture::x86_64:
        switch (id)
        {
        case X86_INS_INT3:
        case X86_INS_INVALID:
        case X86_INS_RET:
        case X86_INS_RETF:
        case X86_INS_RETFQ:
            break;
        case X86_INS_JA:
        case X86_INS_JAE:
        case X86_INS_JB:
        case X86_INS_JBE:
        case X86_INS_JCXZ:
        case X86_INS_JE:
        case X86_INS_JG:
        case X86_INS_JGE:
        case X86_INS_JL:
        case X86_INS_JLE:
        case X86_INS_JNE:
        case X86_INS_JNO:
        case X86_INS_JNP:
        case X86_INS_JNS:
        case X86_INS_JO:
        case X86_INS_JP:
        case X86_INS_JS:
            addresses.insert(address + size);
        case X86_INS_JMP:
            // TODO
            break;
        default:
            addresses.insert(address + size);
            break;
        };
        break;
    default:
        throw std::logic_error("Invalid architecture");
    }

    return addresses;
}

static_assert(std::is_destructible_v<instruction>);

static_assert(std::is_move_constructible_v<instruction>);
static_assert(std::is_move_assignable_v<instruction>);

static_assert(std::is_copy_constructible_v<instruction>);
static_assert(std::is_copy_assignable_v<instruction>);
