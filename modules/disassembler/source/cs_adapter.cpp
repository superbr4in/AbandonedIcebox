#include "cs_adapter.hpp"

void cs::free::operator()(cs_insn* const instruction) const
{
    cs_free(instruction, 1);
};

std::unique_ptr<cs_insn, cs::free> cs::malloc(csh const handle)
{
    return std::unique_ptr<cs_insn, free>(cs_malloc(handle));
}

bool cs::disasm_iter(csh const handle, std::basic_string_view<std::byte>* const code, std::uint_fast64_t* const address,
    cs_insn* const instruction)
{
    auto const* data =
        reinterpret_cast<uint8_t const*>(code->data()); // NOLINT [cppcoreguidelines-pro-type-reinterpret-cast]
    auto size = code->size();

    auto result = cs_disasm_iter(handle, &data, &size, address, instruction);

    *code = std::basic_string_view<std::byte>(
        reinterpret_cast<std::byte const*>(data), // NOLINT [cppcoreguidelines-pro-type-reinterpret-cast]
        size);

    return result;
}
