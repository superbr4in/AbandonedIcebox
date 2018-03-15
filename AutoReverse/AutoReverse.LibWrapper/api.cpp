#include "stdafx.h"

#include "debug_32.h"
#include "debugger.h"

#define API extern "C" __declspec(dllexport)

API debugger* open(const char* file_name)
{
    return new debugger(file_name);
}

API void close(
    debugger* handle)
{
    handle->close();
}

API void debug_32(debugger* handle, debug_32& debug)
{
    debug = handle->debug();
}
