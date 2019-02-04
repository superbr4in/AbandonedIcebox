#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <disassembler/disassembler.hpp>

std::byte operator""_b(unsigned long long value) // NOLINT [google-runtime-int]
{
    return std::byte(value);
}

void test_disassembling(disassembler const& disassembler, std::vector<std::byte> const& bytes,
    std::uint_fast64_t const start_address, std::vector<instruction> const& instructions)
{
    std::basic_string_view<std::byte> next_code(bytes.data(), bytes.size());
    std::uint_fast64_t next_address = start_address;
    for (auto const& instruction : instructions)
    {
        auto const [actual_address, actual_code] = disassembler(&next_code, &next_address);

        CHECK(actual_address == instruction.address);
        CHECK(actual_code == instruction.code);

        auto const byte_pos = instruction.address - start_address;

        CHECK(next_code.data() ==
            &bytes.at(byte_pos) + instruction.code.size()); // NOLINT [cppcoreguidelines-pro-bounds-pointer-arithmetic]
        CHECK(next_code.size() == bytes.size() - byte_pos - instruction.code.size());

        CHECK(next_address == instruction.address + instruction.code.size());
    }
}

TEST_CASE("x86")
{
    auto const disassembler_x86 =
        GENERATE( // NOLINT
            disassembler(architecture::x86_32),
            disassembler(architecture::x86_64));

    std::vector<std::byte> const bytes
    {
        0x90_b,                                 // nop
        0x87_b, 0xC0_b,                         // xchg eax, eax
        0xE9_b, 0xFB_b, 0xFF_b, 0xFF_b, 0xFF_b, // jmp 0x03
        0xFF_b,                                 // (bad)
        0xFF_b, 0xC3_b,                         // inc ebx
        0xCC_b,                                 // int3
        0x83_b, 0xC3_b, 0x00_b,                 // add ebx, 0x00
        0x7F_b                                  // .byte 0x7F
    };

    auto const [start_address, instructions_x86] =
        GENERATE( // NOLINT
            std::pair<std::uint_fast64_t, std::vector<instruction>>
            {
                0x00,
                {
                    { 0x00, { 0x90_b } },
                    { 0x01, { 0x87_b, 0xC0_b } },
                    { 0x03, { 0xE9_b, 0xFB_b, 0xFF_b, 0xFF_b, 0xFF_b } },
                    { 0x08, { 0xFF_b } },
                    { 0x09, { 0xFF_b, 0xC3_b } },
                    { 0x0B, { 0xCC_b } },
                    { 0x0C, { 0x83_b, 0xC3_b, 0x00_b } },
                    { 0x0F, { 0x7F_b } }
                }
            },
            std::pair<std::uint_fast64_t, std::vector<instruction>>
            {
                0x17,
                {
                    { 0x17, { 0x90_b } },
                    { 0x18, { 0x87_b, 0xC0_b } },
                    { 0x1A, { 0xE9_b, 0xFB_b, 0xFF_b, 0xFF_b, 0xFF_b } },
                    { 0x1F, { 0xFF_b } },
                    { 0x20, { 0xFF_b, 0xC3_b } },
                    { 0x22, { 0xCC_b } },
                    { 0x23, { 0x83_b, 0xC3_b, 0x00_b } },
                    { 0x26, { 0x7F_b } }
                }
            });

    test_disassembling(disassembler_x86, bytes, start_address, instructions_x86);
}
