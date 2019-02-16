#include <catch2/catch.hpp>
#include <disassembler/disassembler.hpp>

std::byte operator""_b(unsigned long long value) // NOLINT [google-runtime-int]
{
    return std::byte(value);
}

TEST_CASE("operator()(std::basic_string_view<std::byte>*, std::uint_fast64_t*) const")
{
    instruction_set_architecture architecture;

    std::uint_fast64_t start_address;
    std::vector<std::byte> bytes;
    std::vector<instruction> instructions;

    SECTION("x86")
    {
        architecture =
            GENERATE( // NOLINT
                instruction_set_architecture::x86_32,
                instruction_set_architecture::x86_64);

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
                { architecture,  494, 0x00, 1 },
                { architecture, 1301, 0x01, 2 },
                { architecture,  264, 0x03, 5 },
                { architecture,  215, 0x08, 2 },
                { architecture,  223, 0x0A, 1 },
                { architecture,    8, 0x0B, 3 }
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
                { architecture, 494, 0x17, 1 },
                { architecture, 494, 0x18, 1 }
            };
        }
    }

    disassembler const disassembler(architecture);

    std::uint_fast64_t next_address = start_address;
    std::basic_string_view<std::byte> next_code(bytes.data(), bytes.size());
    for (auto const& instruction : instructions)
    {
        auto const disassembled_instruction = disassembler.iterate(&next_address, &next_code);

        CHECK(disassembled_instruction.id == instruction.id);
        CHECK(disassembled_instruction.address == instruction.address);
        CHECK(disassembled_instruction.size == instruction.size);

        CHECK(next_address == instruction.address + instruction.size);

        auto const byte_pos = instruction.address - start_address;

        CHECK(next_code.data() ==
            &bytes.at(byte_pos) + instruction.size); // NOLINT [cppcoreguidelines-pro-bounds-pointer-arithmetic]
        CHECK(next_code.size() == bytes.size() - byte_pos - instruction.size);
    }
}

TEST_CASE("Exceptions")
{
    auto const architecture =
        GENERATE( // NOLINT
            instruction_set_architecture::x86_32,
            instruction_set_architecture::x86_64);

    disassembler const disassembler(architecture);

    SECTION("operator()(std::basic_string_view<std::byte>*, std::uint_fast64_t*) const")
    {
        std::vector<std::byte> bytes;

        std::uint_fast64_t address;
        std::basic_string_view<std::byte> code;

        SECTION("Empty code range")
        {
            address = 0x00;
            code = std::basic_string_view<std::byte>(nullptr, 0);
        }

        SECTION("Invalid instruction")
        {
            SECTION("(bad)")
            {
                bytes = { 0xFF_b };
            }
            SECTION(".byte 0x7F")
            {
                bytes = { 0x7F_b };
            }

            address = 0x00;
            code = std::basic_string_view<std::byte>(bytes.data(), bytes.size());
        }

        CHECK_THROWS(disassembler.iterate(&address, &code));
    }
}
