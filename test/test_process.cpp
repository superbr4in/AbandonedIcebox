#include <catch2/catch.hpp>

#include <decompilation/process.hpp>

struct instruction_info
{
    std::uint64_t address;
    std::size_t size;

    std::vector<std::pair<std::string, std::string>> impact;
};
struct process_info
{
    std::vector<std::vector<instruction_info>> blocks;
    std::vector<std::pair<std::uint64_t, std::vector<std::uint64_t>>> block_map;
};

template <typename ContainerExpected, typename ContainerActual, typename Compare>
void assert_content(ContainerExpected const& expected, ContainerActual actual, Compare const& compare)
{
    for (auto const& e : expected)
    {
        auto a = actual.begin();
        for (; a != actual.end(); ++a)
        {
            if (compare(e, *a))
                break;
        }

        REQUIRE(a != actual.end());

        actual.erase(a);
    }

    CHECK(actual.empty());
}

void test_process_x86_32(std::vector<std::uint8_t> const& data, process_info const& expected)
{
    std::unique_ptr<dec::process> actual;
    REQUIRE_NOTHROW(actual = std::make_unique<dec::process>(data, dec::instruction_set_architecture::x86_32));

    auto const actual_blocks = actual->blocks();
    auto const actual_block_map = actual->block_map();

    actual.reset();

    auto const impact_entry_compare =
        [](auto const& expected_expression_pair, auto const& actual_expression_pair)
        {
            return
                expected_expression_pair.first == actual_expression_pair.first.to_string() &&
                expected_expression_pair.second == actual_expression_pair.second.to_string();
        };
    auto const block_map_entry_compare =
        [](auto const& expected_block_map_entry, auto const& actual_block_map_entry)
        {
            return expected_block_map_entry.first == actual_block_map_entry.first;
        };

    SECTION("blocks")
    {
        REQUIRE(actual_blocks.size() == expected.blocks.size());

        auto block_index = 0;
        for (auto const& actual_block : actual_blocks)
        {
            auto const& expected_block = expected.blocks.at(block_index);
            REQUIRE(actual_block.size() == expected_block.size());

            auto instruction_index = 0;
            for (auto const& actual_instruction : actual_block)
            {
                auto const& expected_instruction = expected.blocks.at(block_index).at(instruction_index);

                CHECK(actual_instruction.address == expected_instruction.address);
                CHECK(actual_instruction.size == expected_instruction.size);

                assert_content(expected_instruction.impact, actual_instruction.impact, impact_entry_compare);

                ++instruction_index;
            }

            ++block_index;
        }
    }
    SECTION("block_map")
    {
        assert_content(expected.block_map, actual_block_map, block_map_entry_compare);

        for (auto const& [expected_address, expected_succeeding_addresses] : expected.block_map)
            assert_content(expected_succeeding_addresses, actual_block_map.find(expected_address)->second, std::equal_to { });
    }
}

TEST_CASE("A")
{
    auto const& [data, expected] = GENERATE( // NOLINT
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0xCC, // int3
                0xCC  // int3
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 1,

                            .impact = { }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0xC3, // ret
                0xCC  // int3
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0x90, // nop
                0xC3  // ret
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 1,

                            .impact = { }
                        },
                        instruction_info
                        {
                            .address = 1,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { } }
                }
            }
        });

    test_process_x86_32(data, expected);
}
TEST_CASE("B")
{
    auto const& [data, expected] = GENERATE( // NOLINT
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0xEB, 0x00, // jmp--,
                0xC3        // ret<-'
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        },
                        instruction_info
                        {
                            .address = 2,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0x74, 0x00, // je---,
                0xC3        // ret<-'
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        },
                        instruction_info
                        {
                            .address = 2,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0xEB, 0x01, // jmp--,
                0xCC,       // int3 |
                0xC3        // ret<-'
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 3,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { 3 } },
                    { 3, { } }
                }
            }
        });

    test_process_x86_32(data, expected);
}
TEST_CASE("C")
{
    auto const& [data, expected] = GENERATE( // NOLINT
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0x75, 0x01, // jne--, IF
                0x90,       // nop  | THEN
                0xC3        // ret<-'
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 2,
                            .size = 1,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 3,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { 2, 3 } },
                    { 2, { 3 } },
                    { 3, { } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0x75, 0x01, // jne--, IF
                0xC3,       // ret  | THEN
                0xC3        // ret<-' ELSE
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 2,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 3,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { 2, 3 } },
                    { 2, { } },
                    { 3, { } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0x75, 0x03, // jne---, IF
                0xEB, 0x02, // jmp--,| THEN
                0xCC,       // int3 ||
                0x90,       // nop<-|' ELSE
                0xC3        // ret<-'
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 2,
                            .size = 2,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 5,
                            .size = 1,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 6,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { 2, 5 } },
                    { 2, { 6 } },
                    { 5, { 6 } },
                    { 6, { } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0x75, 0x03, // jne---, IF
                0x90,       // nop   | THEN
                0xC3,       // ret<-,|
                0xCC,       // int3 ||
                0xEB, 0xFC  // jmp<-'' ELSE
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 2,
                            .size = 1,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 3,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 5,
                            .size = 2,

                            .impact = { }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { 2, 5 } },
                    { 2, { 3 } },
                    { 3, { } },
                    { 5, { 3 } }
                }
            }
        });

    test_process_x86_32(data, expected);
}
TEST_CASE("D")
{
    auto const& [data, expected] = GENERATE( // NOLINT
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0xEB, 0xFE // jmp<-
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { 0 } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0x74, 0xFE, // je<-
                0xC3        // ret
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 2,

                            .impact = { }
                        }
                    },
                    {
                        instruction_info
                        {
                            .address = 2,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { 0, 2 } },
                    { 2, { } }
                }
            }
        });

    test_process_x86_32(data, expected);
}
TEST_CASE("E")
{
    auto const& [data, expected] = GENERATE( // NOLINT
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0xB8, 0x08, 0x00, 0x00, 0x00, // mov eax, 8
                0xFF, 0xE0,                   // jmp eax
                0xCC,                         // int3
                0xC3                          // ret
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 5,

                            .impact =
                            {
                                { "R_EAX", "#x0000000000000008" }
                            }
                        },
                        instruction_info
                        {
                            .address = 5,
                            .size = 2,

                            .impact = { }
                        }
                    },
//                    {
//                        instruction_info
//                        {
//                            .address = 8,
//                            .size = 1,
//
//                            .impact =
//                            {
//                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
//                            }
//                        }
//                    }
                },
                .block_map =
                {
                    { 0, { /*8*/ } },
//                    { 8, { } }
                }
            }
        },
        std::pair
        {
            std::vector<std::uint8_t>
            {
                0xA1, 0x1B, 0x00, 0x00, 0x00,       // mov eax, [27]
                0x89, 0x1D, 0x1C, 0x00, 0x00, 0x00, // mov [28], ebx
                0xC3                                // ret
            },
            process_info
            {
                .blocks =
                {
                    {
                        instruction_info
                        {
                            .address = 0,
                            .size = 5,

                            .impact =
                            {
                                { "R_EAX", "(bvmem #x000000000000001b)" }
                            }
                        },
                        instruction_info
                        {
                            .address = 5,
                            .size = 6,

                            .impact =
                            {
                                { "(bvmem #x000000000000001c)", "R_EBX" }
                            }
                        },
                        instruction_info
                        {
                            .address = 11,
                            .size = 1,

                            .impact =
                            {
                                { "R_ESP", "(bvadd #x0000000000000004 R_ESP)" }
                            }
                        }
                    }
                },
                .block_map =
                {
                    { 0, { } },
                }
            }
        });

    test_process_x86_32(data, expected);
}
