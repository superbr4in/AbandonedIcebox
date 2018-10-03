#pragma once

#include "emulator.h"

// --- TODO Q&D
struct code_section
{
    std::string name;

    uint64_t raw_address;
    size_t raw_size;

    uint64_t virtual_address;
    size_t virtual_size;
};
// ---

// Initializer for machine code emulation
class loader
{
protected:
    
    emulator* emulator_;

public:

    loader();

    std::unique_ptr<emulator> get_emulator() const;

    virtual ~loader() = default;

    virtual std::string label_at(uint64_t address) const = 0;

    // Initializes a new emulator according to a set of machine code.
    virtual uint16_t load(std::vector<uint8_t> bytes) = 0;

    // Provides certanity that a specified address contains mapped memory.
    // Returns 'true' if previously unmapped memory is now mapped, otherwise 'false'.
    virtual bool ensure_availability(uint64_t address) = 0;

    virtual uint64_t to_raw_address(uint64_t virtual_address, size_t& section_index, std::string& section_name) const = 0;
    
    // --- TODO Q&D
    virtual uint64_t image_base() const = 0;
    virtual std::vector<code_section> sections() const = 0;
    // ---

protected:

    void initialize_environment(size_t stack_size, double stack_fill, uint64_t entry_address) const;

    TPL static T parse_to(std::vector<uint8_t>::const_iterator& iterator);
};

#include "loader_tpl.cpp"

// Initialize for raw machine code emulation
class loader_raw : public loader
{
public:

    std::string label_at(uint64_t address) const override;

    uint16_t load(std::vector<uint8_t> bytes) override;

    bool ensure_availability(uint64_t address) override;

    uint64_t to_raw_address(uint64_t virtual_address, size_t& section_index, std::string& section_name) const override;

    // --- TODO Q&D
    uint64_t image_base() const override;
    std::vector<code_section> sections() const override;
    // ---

    static std::vector<uint8_t> create_aid(uint16_t machine, uint64_t base_address, const std::vector<uint8_t>& bytes);
};

// Initializer for machine code emulation of PE binaries
class loader_pe : public loader
{
    // Important properties of a PE file header
    struct header_pe
    {
        WORD machine { };

        ULONGLONG image_base { };
        ULONGLONG stack_commit { };

        DWORD entry_point { };

        IMAGE_DATA_DIRECTORY import_directory { };

        std::vector<IMAGE_SECTION_HEADER> section_headers;

        header_pe();
        explicit header_pe(const std::vector<uint8_t>& buffer);
    };

    bool defer_imports_;

    header_pe header_;

    std::map<std::string, header_pe> imported_dlls_;
    std::map<uint64_t, std::string> deferred_dlls_;

    std::map<uint64_t, std::string> labels_;

    std::map<uint64_t, std::map<std::string, IMAGE_IMPORT_DESCRIPTOR>> import_descriptors_;

public:

    loader_pe();

    std::string label_at(uint64_t address) const override;

    uint16_t load(std::vector<uint8_t> bytes) override;

    bool ensure_availability(uint64_t address) override;

    uint64_t to_raw_address(uint64_t virtual_address, size_t& section_index, std::string& section_name) const override;

    // --- TODO Q&D
    uint64_t image_base() const override;
    std::vector<code_section> sections() const override;
    // ---

private:
    
    int import_single_dll(uint64_t base, std::string dll_name, bool sub);
    void import_all_dlls(header_pe header, bool sub);
};