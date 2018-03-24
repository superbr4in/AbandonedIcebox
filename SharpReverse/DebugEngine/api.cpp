#include "stdafx.h"

#include "debugger.h"

#include "bin_dump.h"

#define API extern "C" __declspec(dllexport)

API target_machine open(debugger*& handle, const char* bytes, const int32_t size)
{
    handle = new debugger();
    return handle->open(std::vector<char>(bytes, bytes + size));
}
API target_machine open_file(debugger*& handle, const char* file_name)
{
    handle = new debugger();
    return handle->open(create_dump(file_name));
}

API void close(debugger* handle)
{
    handle->close();
    free(handle); // TODO: Necessary?
}

API void debug(debugger* handle, instruction_info& instruction)
{
    instruction = handle->debug();
}

API void inspect_registers(debugger* handle, register_info& register_state)
{
    register_state = handle->inspect_registers();
}
