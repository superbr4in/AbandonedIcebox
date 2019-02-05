#include <catch2/catch.hpp>
#include <disassembler/disassembler.hpp>

std::byte operator""_b(unsigned long long value) // NOLINT [google-runtime-int]
{
    return std::byte(value);
}

TEST_CASE("operator()(std::basic_string_view<std::byte>*, std::uint_fast64_t*) const")
{
    architecture architecture;
    std::uint_fast64_t start_address;
    std::vector<std::byte> bytes;
    std::vector<instruction> instructions;

    SECTION("x86")
    {
        architecture =
            GENERATE( // NOLINT
                architecture::x86_32,
                architecture::x86_64);

        SECTION("Arbitrary instructions")
        {
            start_address = 0x00;
            bytes =
            {
                0x90_b,                                 // nop
                0x87_b, 0xC0_b,                         // xchg eax, eax
                0xE9_b, 0xFB_b, 0xFF_b, 0xFF_b, 0xFF_b, // jmp 0x00
                0xFF_b, 0xC3_b,                         // inc ebx
                0xCC_b,                                 // int3
                0x83_b, 0xC3_b, 0x00_b,                 // add ebx, 0x00
            };
            instructions =
            {
                { 0x00, { &bytes.at( 0), 1 } },
                { 0x01, { &bytes.at( 1), 2 } },
                { 0x03, { &bytes.at( 3), 5 } },
                { 0x08, { &bytes.at( 8), 2 } },
                { 0x0A, { &bytes.at(10), 1 } },
                { 0x0B, { &bytes.at(11), 3 } }
            };
        }
        SECTION("Arbitrary start address")
        {
            start_address = 0x17;
            bytes =
            {
                0x90_b, // nop
                0x90_b  // nop
            };
            instructions =
            {
                { 0x17, { &bytes.at(0), 1 } },
                { 0x18, { &bytes.at(1), 1 } }
            };
        }
        SECTION("Invalid instructions")
        {
            start_address = 0x00;
            bytes =
            {
                0xFF_b, // (bad)
                0x7F_b  // .byte 0x7F
            };
            instructions =
            {
                { 0x00, { &bytes.at(0), 1 } },
                { 0x01, { &bytes.at(1), 1 } }
            };
        }
    }

    disassembler const disassembler(architecture);

    std::uint_fast64_t next_address = start_address;
    std::basic_string_view<std::byte> next_code(bytes.data(), bytes.size());
    for (auto const& instruction : instructions)
    {
        auto const disassembled_instruction = disassembler(&next_address, &next_code);

        CHECK(disassembled_instruction.address == instruction.address);
        CHECK(disassembled_instruction.code.data() == instruction.code.data());
        CHECK(disassembled_instruction.code.size() == instruction.code.size());

        CHECK(next_address == instruction.address + instruction.code.size());

        auto const byte_pos = instruction.address - start_address;

        CHECK(next_code.data() ==
            &bytes.at(byte_pos) + instruction.code.size()); // NOLINT [cppcoreguidelines-pro-bounds-pointer-arithmetic]
        CHECK(next_code.size() == bytes.size() - byte_pos - instruction.code.size());
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
            std::uint_fast64_t address = 0x00;
            std::basic_string_view<std::byte> code(nullptr, 0);

            CHECK_THROWS(disassembler(&address, &code));
        }
    }
}
