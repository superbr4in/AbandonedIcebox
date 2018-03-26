#include "stdafx.h"

#include "bin_dump.h"
#include "pe_debugger.h"

#define API extern "C" __declspec(dllexport)

API int debugger_open(pe_debugger*& handle, const char* bytes, const int32_t size)
{
    handle = new pe_debugger();
    return handle->open(std::vector<char>(bytes, bytes + size));
}
API int debugger_open_file(pe_debugger*& handle, const char* file_name)
{
    std::vector<char> bytes;
    create_dump(file_name, bytes);
    
    handle = new pe_debugger();
    return handle->open(bytes);
}

API int debugger_close(pe_debugger* handle)
{
    const auto res = handle->close();
    free(handle); // TODO: Necessary?
    return res;
}

API BOOL targets_64(pe_debugger* handle)
{
    auto a = handle->header().targets_64();
    return a;
}

API int debug(pe_debugger* handle, instruction_info& ins_info)
{
    return handle->debug(ins_info);
}

API int inspect_registers(pe_debugger* handle, register_info& reg_info)
{
    return handle->inspect_registers(reg_info);
}
