#include <catch2/catch.hpp>
#include <disassembler/disassembler.hpp>

std::byte operator""_b(unsigned long long value) // NOLINT [google-runtime-int]
{
    return std::byte(value);
}

TEST_CASE("operator()(std::basic_string_view<std::byte>*, std::uint_fast64_t*) const")
{
    architecture architecture;
    std::vector<std::byte> bytes;
    std::uint_fast64_t start_address;
    std::vector<instruction> instructions;

    SECTION("x86")
    {
        architecture =
            GENERATE( // NOLINT
                architecture::x86_32,
                architecture::x86_64);

        SECTION("Arbitrary instructions")
        {
            bytes =
            {
                0x90_b,                                 // nop
                0x87_b, 0xC0_b,                         // xchg eax, eax
                0xE9_b, 0xFB_b, 0xFF_b, 0xFF_b, 0xFF_b, // jmp 0x00
                0xFF_b, 0xC3_b,                         // inc ebx
                0xCC_b,                                 // int3
                0x83_b, 0xC3_b, 0x00_b,                 // add ebx, 0x00
            };
            start_address = 0x00;
            instructions =
            {
                { 0x00, { 0x90_b } },
                { 0x01, { 0x87_b, 0xC0_b } },
                { 0x03, { 0xE9_b, 0xFB_b, 0xFF_b, 0xFF_b, 0xFF_b } },
                { 0x08, { 0xFF_b, 0xC3_b } },
                { 0x0A, { 0xCC_b } },
                { 0x0B, { 0x83_b, 0xC3_b, 0x00_b } },
            };
        }
        SECTION("Arbitrary start address")
        {
            bytes =
            {
                0x90_b, // nop
                0x90_b  // nop
            };
            start_address = 0x17;
            instructions =
            {
                { 0x17, { 0x90_b } },
                { 0x18, { 0x90_b } }
            };
        }
        SECTION("Invalid instructions")
        {
            bytes =
            {
                0xFF_b, // (bad)
                0x7F_b  // .byte 0x7F
            };
            start_address = 0x00;
            instructions =
            {
                { 0x00, { 0xFF_b } },
                { 0x01, { 0x7F_b } }
            };
        }
    }

    disassembler const disassembler(architecture);

    std::basic_string_view<std::byte> next_code(bytes.data(), bytes.size());
    std::uint_fast64_t next_address = start_address;
    for (auto const& instruction : instructions)
    {
        auto const [disassembled_address, disassembled_code] = disassembler(&next_code, &next_address);

        CHECK(disassembled_address == instruction.address);
        CHECK(disassembled_code == instruction.code);

        auto const byte_pos = instruction.address - start_address;

        CHECK(next_code.data() ==
            &bytes.at(byte_pos) + instruction.code.size()); // NOLINT [cppcoreguidelines-pro-bounds-pointer-arithmetic]
        CHECK(next_code.size() == bytes.size() - byte_pos - instruction.code.size());

        CHECK(next_address == instruction.address + instruction.code.size());
    }
}

TEST_CASE("Exceptions")
{
    auto const architecture =
        GENERATE( // NOLINT
            architecture::x86_32,
            architecture::x86_64);

    disassembler const disassembler(architecture);

    SECTION("operator()(std::basic_string_view<std::byte>*, std::uint_fast64_t*) const")
    {
        SECTION("Empty code range")
        {
            std::basic_string_view<std::byte> code(nullptr, 0);
            std::uint_fast64_t address = 0x00;

            CHECK_THROWS(disassembler(&code, &address));
        }
    }
}
