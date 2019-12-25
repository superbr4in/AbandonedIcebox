#include <fstream>

#include <libgen.h>

#include <catch2/catch.hpp>

#include <grev/machine_process.hpp>
#include <grev-lift/reil_disassembler.hpp> // TODO Mockup
#include <grev-load/pe_loader.hpp>

#include "test.hpp"

#define ADD_EAX_EBX 0x01, 0xD8
#define ADD_EAX(v) 0x83, 0xC0, std::uint8_t(v)
#define CALL(v) 0xE8, std::uint8_t{v}, 0x00, 0x00, 0x00
#define CALL_DWORD_PTR_EAX 0xFF, 0x10
#define CMP_DWORD_PTR_EAX(v) 0x83, 0x38, std::uint8_t(v)
#define INT3 0xCC
#define JE(v) 0x74, std::uint8_t(v)
#define JMP(v) 0xEB, std::uint8_t(v)
#define JMP_EAX 0xFF, 0xE0
#define JNE(v) 0x75, std::uint8_t(v)
#define MOV_EAX(v) 0xB8, std::uint8_t(v), 0x00, 0x00, 0x00
#define MOV_EBX(v) 0xBB, std::uint8_t(v), 0x00, 0x00, 0x00
#define MOV_DWORD_PTR_ESP(s, v) 0xC7, 0x44, 0x24, std::uint8_t(s), std::uint8_t(v), 0x00, 0x00, 0x00
#define NOP 0x90
#define RET 0xC3
#define RET_(v) 0xC2, std::uint8_t(v), 0x00

std::vector<std::vector<std::uint32_t>> get_path_addresses(std::forward_list<grev::execution_path> const& paths)
{
    std::vector<std::vector<std::uint32_t>> path_addresses;
    for (auto const& path : paths)
        path_addresses.push_back(path.addresses());

    return path_addresses;
}

TEST_CASE("Path inspection", "[grev::machine_process]")
{
    grev::machine_architecture architecture;
    std::vector<std::uint8_t> data;

    std::vector<std::vector<std::uint32_t>> expected_path_addresses;

    SECTION("x86_32")
    {
        architecture = grev::machine_architecture::x86_32;

        SECTION("A")
        {
            data = GENERATE(
                std::vector<std::uint8_t>
                {
                    RET
                },
                std::vector<std::uint8_t>
                {
                    RET,
                    INT3
                });

            expected_path_addresses =
            {
                { 0 }
            };
        }
        SECTION("B")
        {
            data =
            {
                JMP(1), // --,
                INT3,   //   |
                RET     // <-'
            };

            expected_path_addresses =
            {
                { 0, 3 }
            };
        }
        SECTION("C")
        {
            data =
            {
                JNE(1), // --, IF
                NOP,    //   | THEN
                RET     // <-'
            };

            expected_path_addresses =
            {
                { 0, 2, 3 },
                { 0,    3 }
            };
        }
        SECTION("D")
        {
            data =
            {
                JNE(1), // --, IF
                RET,    //   | THEN
                RET     // <-' ELSE
            };

            expected_path_addresses =
            {
                { 0, 2 },
                { 0, 3 }
            };
        }
        SECTION("E")
        {
            data =
            {
                JNE(3), // ---, 0 IF
                JMP(2), // --,| 2 THEN
                INT3,   //   ||
                NOP,    // <-|' 5 ELSE
                RET     // <-'  6
            };

            expected_path_addresses =
            {
                { 0, 2, 6 },
                { 0, 5, 6 }
            };
        }
        SECTION("F")
        {
            data =
            {
                JNE(3), // ---, 0 IF
                NOP,    //    | 2 THEN
                RET,    // <-,| 3
                INT3,   //   ||
                JMP(-4) // <-'' 5 ELSE
            };

            expected_path_addresses =
            {
                { 0, 2, 3 },
                { 0, 5, 3 }
            };
        }
        SECTION("G")
        {
            data =
            {
                JMP(-2) // <- WHILE
            };

            expected_path_addresses =
            {
                { 0 }
            };
        }
        SECTION("I")
        {
            data =
            {
                MOV_EAX(8),
                JMP_EAX, // --,
                INT3,    //   |
                RET      // <-'
            };

            expected_path_addresses =
            {
                { 0, 5, 8 }
            };
        }
        SECTION("J")
        {
            data =
            {
                MOV_EAX(11),
                JMP(1),  // --,
                INT3,    //   |
                JMP_EAX, // <-',
                INT3,    //    |
                RET      // <--'
            };

            expected_path_addresses =
            {
                { 0, 5, 8, 11 }
            };
        }
        SECTION("K")
        {
            data =
            {
                MOV_EAX(18),
                JMP(1),      // --,
                INT3,        //   |
                MOV_EAX(19), // <-'
                JMP(1),      // --,
                INT3,        //   |
                JMP_EAX,     // <-',
                INT3,        //    |
                RET          // <--'
            };

            expected_path_addresses =
            {
                { 0, 5, 8, 13, 16, 19 }
            };
        }
        SECTION("L")
        {
            data =
            {
                MOV_EAX(8),
                JMP(1),     // --,
                INT3,       //   |
                ADD_EAX(9), // <-'
                JMP(1),     // --,
                INT3,       //   |
                JMP_EAX,    // <-',
                INT3,       //    |
                RET         // <--'
            };

            expected_path_addresses =
            {
                { 0, 5, 8, 11, 14, 17 }
            };
        }
        SECTION("M")
        {
            data =
            {
                MOV_EAX(12),
                MOV_EBX(6),
                JMP(1),      // --,
                INT3,        //   |
                ADD_EAX_EBX, // <-'
                JMP_EAX,     // --,
                INT3,        //   |
                RET          // <-'
            };

            expected_path_addresses =
            {
                { 0, 5, 10, 13, 15, 18 }
            };
        }
        SECTION("N")
        {
            data =
            {
                JNE(8),      // ---,  0 IF
                MOV_EAX(18), //    |  2 THEN
                JMP(6),      // --,|  7
                INT3,        //   ||
                MOV_EAX(19), // <-|' 10 ELSE
                JMP_EAX,     // <-', 15
                INT3,        //    |
                RET,         // <--' 18
                RET          // <--' 19
            };

            expected_path_addresses =
            {
                { 0,  2, 7, 15, 18 },
                { 0, 10,    15, 19 }
            };
        }
        SECTION("O")
        {
            data =
            {
                CALL(1), // 0
                RET,     // 5
                RET      // 6
            };

            expected_path_addresses =
            {
                { 0, 6, 5 }
            };
        }
        SECTION("P")
        {
            data =
            {
                MOV_EAX(8),             // 0
                CALL_DWORD_PTR_EAX,     // 5
                RET,                    // 7
                0x0C, 0x00, 0x00, 0x00, // 8
                RET                     // 12
            };

            expected_path_addresses =
            {
                { 0, 5, 12, 7 }
            };
        }
        SECTION("Q")
        {
            data =
            {
                CALL(1), // 0
                RET,     // 5
                RET_(4)  // 6
            };

            expected_path_addresses =
            {
                { 0, 6, 5 }
            };
        }
        SECTION("R")
        {
            data =
            {
                MOV_EAX(12),           //  0
                CMP_DWORD_PTR_EAX(4),  //  5
                JE(1),                 //  8
                NOP,                   // 10
                RET,                   // 11
                0x04, 0x00, 0x00, 0x00 // 12
            };

            expected_path_addresses =
            {
                { 0, 5, 8, 11 }
            };
        }
    }
    // TODO x86_64, etc.

    grev::reil_disassembler const disassembler(architecture); // TODO Mockup
    grev::machine_program const program(data, architecture);

    auto const execution = grev::machine_process(program, { }).execute(disassembler);
    auto const actual_path_addresses = get_path_addresses(execution.paths);

    CHECK(matches(actual_path_addresses, expected_path_addresses));
}
TEST_CASE("Real path inspection", "[grev::machine_process]")
{
    SECTION("x86_32")
    {
        grev::reil_disassembler const disassembler(grev::machine_architecture::x86_32);

        SECTION("helloworld_32.exe")
        {
            auto const prog = grev::machine_program::load<grev::pe_loader>("/home/superbr4in/hello_world_32/hello_world_32.exe");
            grev::machine_process const process(prog,
                {
                    { 0x0040336C, std::vector<std::uint8_t> { 0x00, 0x00, 0x00, 0x00 } },
                    { 0x00403370, std::vector<std::uint8_t> { 0x00, 0x00, 0x13, 0x00 } },
                    { 0x0040337C, std::vector<std::uint8_t> { 0x01, 0x00, 0x00, 0x00 } },
                    { 0x00403380, std::vector<std::uint8_t> { 0x00, 0x00, 0x00, 0x00 } },
                    { 0x7FFE0014, std::vector<std::uint8_t> { 0x3C, 0x31, 0x72, 0xED } },
                    { 0x7FFE0018, std::vector<std::uint8_t> { 0xC1, 0xAD, 0xD5, 0x01 } },
                    { 0x7FFE001C, std::vector<std::uint8_t> { 0xC1, 0xAD, 0xD5, 0x01 } },
                    { 0x7FFE0300, std::vector<std::uint8_t> { 0xF0, 0xE4, 0x90, 0x7C } }
                });
            CHECK(matches(get_path_addresses(process.execute(disassembler).paths), std::vector<std::vector<std::uint32_t>>
            {
                // Incomplete, tailored to current functionality TODO
                {
                    0x004012AE, 0x00401729, 0x0040172B, 0x0040172C, 0x0040172E, 0x00401731, 0x00401736, 0x0040173A, 0x0040173E,
                    0x0040173F, 0x00401740, 0x00401745, 0x0040174A, 0x0040174C, 0x0040175B, 0x0040175C, 0x0040175F, 0x00401760,
                    0x000025DC, 0x7C8017EB, 0x7C8017EC, 0x7C8017EE, 0x7C8017F3, 0x7C8017F9, 0x7C8017FF, 0x7C801801, 0x7C801804,
                    0x7C801806, 0x7C801809, 0x7C80180A, 0x00401766, 0x00401769, 0x0040176C, 0x000025C6, 0x7C8099B6, 0x7C8099B9,
                    0x00401772, 0x00401774, 0x000025B0, 0x7C8097BE, 0x7C8097C1, 0x0040177A, 0x0040177C, 0x000025A0, 0x7C809333,
                    0x7C809335, 0x7C809338, 0x7C80933C, 0x00401782, 0x00401784, 0x00401787, 0x00401788, 0x00002586, 0x7C80A4B9,
                    0x7C80A4BA, 0x7C80A4BC, 0x7C80A4BD, 0x7C80A4BE, 0x7C80A4C1, 0x7C80A4C2, 0x7C80A4C5, 0x7C90D890, 0x7C90D895,
                    0x7C90D89A, 0x7C90E4F0, 0x7C90E4F2, 0x7C90E4F4, 0x7C90D89C, 0x7C80A4CB, 0x7C80A4CD, 0x7C80A4D3, 0x7C80A4D7,
                    0x7C80A4D8, 0x7C80A4D9, 0x7C80A4DA, 0x7C80A4DB, 0x7C80A4DC, 0x7C80A4DD, 0x7C80A4DF, 0x7C80A4E0, 0x7C80A4E1,
                    0x0040178E, 0x00401791, 0x00401794, 0x00401796, 0x00401798, 0x004017A1, 0x004017A3, 0x004017B1, 0x004017B7,
                    0x004017B9, 0x004017BF, 0x004017C0, 0x004017C1, 0x004017C2, 0x004017C3, 0x004012B3, 0x0040106B, 0x0040106D,
                    0x00401072, 0x00401680, 0x00401685, 0x0040168C, 0x00401690, 0x00401694, 0x00401698, 0x0040169A, 0x0040169B,
                    0x0040169C, 0x0040169D, 0x004016A2, 0x004016A5, 0x004016A7, 0x004016A8, 0x004016AB, 0x004016AE, 0x004016B1,
                    0x004016B8, 0x004016BB, 0x004016BE, 0x004016C4, 0x00401077, 0x00401079, 0x0040107F, 0x0040108C, 0x0040108F,
                    0x00401095, 0x00401098, 0x0040109B, 0x004010A0, 0x004010A1, 0x004010A2, 0x004010A3, 0x000024BC, 0x7C809836,
                    0x7C80983A, 0x7C80983E, 0x7C809842, 0x004010A9, 0x004010AB, 0x004010C6, 0x004010C8, 0x004010C9, 0x004010CE,
                    0x004010D0, 0x004010DC, 0x004010E1, 0x004010E3, 0x004010E5, 0x004010EB, 0x004010F0, 0x004010F5, 0x00401672,
                    0x000023AE, 0x78AB228C, 0x78AB228D, 0x78AB228F, 0x78AB2290, 0x78AB2293, 0x78AB2295, 0x78AB2298, 0x78AB22AB,
                    0x78AB22AC, 0x78AB22AD, 0x004010FA, 0x004010FB, 0x004010FC, 0x004010FE, 0x00401117, 0x0040111C, 0x0040111E,
                    0x00401120, 0x00401125, 0x0040112A, 0x0040166C, 0x000023A2, 0x78AB226E, 0x78AB226F, 0x78AB2271, 0x78AB2272,
                    0x78AB2275, 0x78AB2278, 0x78AB2287, 0x78AB2288, 0x78AB2289, 0x0040112F, 0x00401130, 0x00401131, 0x0040113B,
                    0x0040113E, 0x00401140, 0x00401141, 0x00401142, 0x0000249E, 0x7C809822, 0x7C809826, 0x7C809828, 0x7C80982C,
                    0x7C80982E, 0x00401148, 0x0040114E, 0x00401169, 0x0040116E, 0x00401174, 0x00401176, 0x0040117C, 0x00401182,
                    0x00401188, 0x00401000, 0x00401005, 0x00002338, 0x78B056B6, 0x78B056BB, 0x78AB0C80, 0x78AB0C85, 0x78AB0C8C,
                    0x78AB0C90, 0x78AB0C94, 0x78AB0C98, 0x78AB0C9A, 0x78AB0C9B, 0x78AB0C9C, 0x78AB0C9D, 0x78AB0CA2, 0x78AB0CA5,
                    0x78AB0CA7, 0x78AB0CA8, 0x78AB0CAB, 0x78AB0CAE, 0x78AB0CB1, 0x78AB0CB8, 0x78AB0CBB, 0x78AB0CBE, 0x78AB0CC4,
                    0x78B056C0, 0x78B056C2, 0x78B056C4, 0x78B056C7, 0x78B056CA, 0x78B056CC, 0x78B056E3, 0x78AB1EC6, 0x78AB1ECB,
                    0x78B056E8, 0x78B056EA, 0x78B056EB, 0x78B056ED, 0x78B056EE, 0x78B056F0, 0x78ABAB59, 0x78ABAB5B, 0x78ABAB5C,
                    0x78ABAB5E, 0x78ABAB61, 0x78ABAB64, 0x78ABAB6A, 0x78ABAB6D, 0x78ABAB6E, 0x78ABAB6F, 0x78ABAB70, 0x78ABAB71,
                    0x78ABAB72, 0x78ABAB73, 0x78ABAB76, 0x78ABAB7D, 0x78ABAB7E, 0x78ABAB7F, 0x78B056F5, 0x78B056F6, 0x78B056F7,
                    0x78B056FA, 0x78B056FB, 0x78B056FC, 0x78B056FD, 0x78B056FE, 0x78B056FF, 0x78B05701, 0x78B05702, 0x78AC0DAC,
                    0x78AC0DAE, 0x78AC0DAF, 0x78AC0DB1, 0x78AC0DB2, 0x78AC0DB5, 0x78AC0DB6, 0x78ABA594, 0x78ABA596, 0x78ABA597,
                    0x78ABA599, 0x78ABA59C, 0x78ABA59E, 0x78ABA5A4, 0x78ABA5A7, 0x78ABA5A8, 0x78AC0DBB, 0x78AC0DBC, 0x78AC01EE,
                    0x78AC01F0, 0x78AC01F1, 0x78AC01F3, 0x78AC01F6, 0x78AC01F9, 0x78AC01FF, 0x78AC0201, 0x78AC0207, 0x78AC020D,
                    0x78AE0359, 0x78AB07B5, 0x78AB0698, 0x78AB069A, 0x78AB069B, 0x78AB069C, 0x78AB069D, 0x78AB069E, 0x78AB069F,
                    0x78AB06A0, 0x78AB06A1, 0x78AB06A2, 0x78AB06A8, 0x78AB06AA, 0x78AB067B, 0x78AB067D, 0x78AB067E, 0x78AB0684,
                    0x000B080C, 0x7C8097D2, 0x7C8097D3, 0x7C8097D5, 0x7C8097DB, 0x7C8097DE, 0x7C8097E1, 0x7C8097E2, 0x7C8097E3,
                    0x7C8097E4, 0x7C8097E5, 0x7C8097E6, 0x7C8097E7, 0x7C8097EB, 0x7C8097F2, 0x7C8097F3, 0x78AB068A, 0x78AB068C,
                    0x78AB068E, 0x78AB068F, 0x78AB0690, 0x78AB0691, 0x78AB0692, 0x78AB0693, 0x78AB0694, 0x78AB0696, 0x78AB0697,
                    0x78AB06AF, 0x78AB06B0, 0x78AB06B1, 0x78AB06B3, 0x78AB06B5, 0x78AB06B6, 0x78AB06B7, 0x78AB06B8, 0x78AB06B9,
                    0x78AB06BA, 0x78AB06BB, 0x78AB06BC, 0x78AB06BD, 0x78AB06BE, 0x78AB06BF, 0x78AB06C0, 0x78AB06C1, 0x78AB06C2,
                    0x78AB06C3, 0x78AB06C5, 0x78AB06C6, 0x00403370
                }
            }));
        }
    }
}
