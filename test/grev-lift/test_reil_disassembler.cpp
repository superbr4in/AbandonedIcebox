#include <catch2/catch.hpp>

#include <grev-lift/reil_disassembler.hpp>

#include "test.hpp"

TEST_CASE("Disassembling", "[grev::reil_disassembler]")
{
    grev::machine_architecture architecture;

    auto const address = GENERATE(as<std::uint32_t>(),
        0, 1, 17, 1639);
    std::vector<std::uint8_t> data;

    grev::execution_state initial_state;

    std::unordered_map<grev::z3::expression, grev::z3::expression> expected_state;
    std::unordered_map<grev::z3::expression, grev::z3::expression> expected_jumps;

    SECTION("x86_32")
    {
        architecture = grev::machine_architecture::x86_32;

        SECTION("A")
        {
            SECTION("ret")
            {
                data = { 0xC3 };

                expected_state.emplace(grev::z3::expression(32, "R_ESP"), grev::z3::expression(32, "R_ESP") + grev::z3::expression(32, 4));
                expected_jumps.emplace(grev::z3::expression::boolean_true(), grev::z3::expression(32, "R_ESP").dereference(32));
            }
        }
        SECTION("B")
        {
            SECTION("B1")
            {
                SECTION("int3")
                {
                    data = { 0xCC };
                }
                SECTION("nop")
                {
                    data = { 0x90 };
                }
                SECTION("mov eax, [27]")
                {
                    data = { 0xA1, 0x1B, 0x00, 0x00, 0x00 };

                    expected_state.emplace(grev::z3::expression(32, "R_EAX"), grev::z3::expression(32, 27).dereference(32));
                }
                SECTION("mov [27], eax")
                {
                    data = { 0xA3, 0x1B, 0x00, 0x00, 0x00 };

                    expected_state.emplace(grev::z3::expression(32, 27).dereference(32), grev::z3::expression(32, "R_EAX"));
                }
                SECTION("mov ebx, [eax]")
                {
                    data = { 0x8b, 0x18 };

                    expected_state.emplace(grev::z3::expression(32, "R_EBX"), grev::z3::expression(32, "R_EAX").dereference(32));
                }
                SECTION("mov [eax], ebx")
                {
                    data = { 0x89, 0x18 };

                    expected_state.emplace(grev::z3::expression(32, "R_EAX").dereference(32), grev::z3::expression(32, "R_EBX"));
                }
            }
            SECTION("B2")
            {
                initial_state.define(grev::z3::expression(32, "R_EAX"), grev::z3::expression(32, 26));

                SECTION("mov eax, 27")
                {
                    data = { 0xb8, 0x1b, 0x00, 0x00, 0x00 };

                    expected_state.emplace(grev::z3::expression(32, "R_EAX"), grev::z3::expression(32, 27));
                }
            }

            expected_jumps.emplace(grev::z3::expression::boolean_true(), grev::z3::expression(32, address + data.size()));
        }
        SECTION("C")
        {
            SECTION("C1")
            {
                SECTION("je 27")
                {
                    data = { 0x74, 0x19 };

                    expected_jumps.emplace(
                        grev::z3::expression(1, "R_ZF"),
                        grev::z3::expression(32, address + 27));
                    expected_jumps.emplace(
                        ~grev::z3::expression(1, "R_ZF"),
                        grev::z3::expression(32, address + data.size()));
                }
            }
        }
    }
    // TODO x86_64, etc.

    grev::reil_disassembler const reil_disassembler(architecture);

    auto updated_address = address;
    std::span<std::uint8_t const> updated_data{data};

    auto const actual_execution = reil_disassembler(&updated_address, &updated_data);

    CHECK(updated_address == address + data.size());
    CHECK(updated_data.empty());

    auto actual_state = actual_execution.front().state();
    // TODO same state for every path?
    for (auto const& path : actual_execution)
    {
        REQUIRE(matches(
            *reinterpret_cast<std::unordered_map<grev::z3::expression, grev::z3::expression> const*>(&actual_state), // TODO
            *reinterpret_cast<std::unordered_map<grev::z3::expression, grev::z3::expression> const*>(&path.state())));
    }

    actual_state = initial_state + actual_state; // TODO Check only returned actual_state

    std::unordered_map<grev::z3::expression, grev::z3::expression> actual_jumps;
    for (auto const& path : actual_execution)
        actual_jumps.emplace(path.condition(), *path.jump());

    CHECK(matches(
        *reinterpret_cast<std::unordered_map<grev::z3::expression, grev::z3::expression> const*>(&actual_state), // TODO
        expected_state));
    CHECK(matches(actual_jumps, expected_jumps));
}
