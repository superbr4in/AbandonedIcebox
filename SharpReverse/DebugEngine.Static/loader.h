#pragma once

/**
 * \brief Initializer for machine code emulation
 */
class loader
{
public:

    virtual ~loader() = default;

    /**
     * \brief Initializes a disassembler and an emulator according to a set of machine code.
     */
    virtual int load(std::vector<char> bytes, csh& cs, uc_engine*& uc) = 0;

    virtual uint64_t scale() const = 0;

    virtual std::vector<int> regs() const = 0;
    virtual int ip_index() const = 0;

    virtual bool find_sec(uint64_t sec_address, std::string& owner, std::string& description) = 0;
    virtual bool find_proc(uint64_t proc_address, std::string& dll_name, std::string& proc_name) = 0;
};

/**
 * \brief Important properties of a PE file header
 */
struct header_pe
{
    WORD machine;

    ULONGLONG image_base;
    ULONGLONG stack_commit;

    DWORD entry_point;

    std::array<IMAGE_DATA_DIRECTORY, 16> data_directories;

    std::vector<IMAGE_SECTION_HEADER> section_headers;

    /**
     * \brief Inspects a range of bytes for a valid PE header and initializes all fields if successful.
     */
    int inspect(const char* buffer);
};

/**
 * \brief Initializer for machine code emulation of PE binaries
 */
class loader_pe : public loader
{
    WORD machine_ { };
    
    std::set<std::string> imported_dlls_ { };

    std::map<uint64_t, std::pair<std::string, std::string>> secs_ { };
    std::map<uint64_t, std::pair<std::string, std::string>> procs_ { };

    void init_section(uc_engine* uc, std::string owner, std::string desc, uint64_t address, const void* buffer, size_t size);

    void import_dlls(uc_engine* uc, header_pe header, bool sub);

public:

    int load(std::vector<char> bytes, csh& cs, uc_engine*& uc) override;

    uint64_t scale() const override;

    std::vector<int> regs() const override;
    int ip_index() const override;

    bool find_sec(uint64_t sec_address, std::string& owner, std::string& description) override;
    bool find_proc(uint64_t proc_address, std::string& dll_name, std::string& proc_name) override;
};
