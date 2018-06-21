#pragma once

#include "disassembler.h"
#include "emulator.h"

#include "loader.h"

#define MAX_TRACE 5

// --- TODO Q&D
struct stack_representation
{
    uint64_t sp, bp;
    std::vector<uint8_t> memory;
};
// ---

enum debug_point
{
    dp_break,
    dp_skip,
    dp_take
};

// Low-level debugger of executable binaries
class debugger
{
    loader& loader_;

    std::unique_ptr<disassembler> disassembler_;
    std::unique_ptr<emulator> emulator_;

    instruction_x86 next_instruction_;

    std::map<uint64_t, debug_point> debug_points_;

    std::map<uint64_t, size_t> counter_;

    std::vector<std::vector<uint8_t>> byte_trace_;
    std::map<uint64_t, size_t> byte_trace_pointer_; // TODO: Unordered Map ?

public:
    
    // Uses a loader to make some machine code ready for debugging.
    explicit debugger(loader& loader, std::vector<uint8_t> code);

    instruction_x86 next_instruction() const;

    instruction_x86_live run(size_t count = 0);

    // Emulates the next machine code instruction.
    instruction_x86_live step_into();

    int step_back();

    int set_debug_point(uint64_t address, debug_point point);
    int remove_debug_point(uint64_t address);

    int jump_to(uint64_t address);
    int get_raw(uint64_t virtual_address, uint64_t& raw_address, size_t& section_index, std::string& section_name) const;

    int skip();
    int take();

    bool is_debug_point(uint64_t address) const;

    int get_bytes(uint64_t address, size_t count, std::vector<uint8_t>& bytes);

    // --- TODO Q&D
    stack_representation get_stack() const;
    void set_stack(stack_representation stack) const;

    uint64_t image_base() const;
    std::vector<code_section> sections() const;
    // ---

// private: TODO Q&D

    instruction_x86 disassemble_at(uint64_t address) const;
};
