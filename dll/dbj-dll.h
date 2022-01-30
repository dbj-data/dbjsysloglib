#pragma once
#ifndef __clang__
#error Obviously, this code requires clang compiler. How about clang-cl ?
#else
#pragma clang system_header
#endif  // __clang__
/*
(c)  2021-2022 by dbj at dbj dot org, https://dbj.org/license_dbj

this might be the only dll load/unload infrastructure one might need
it all depends on if and what and when is OS caching

Obviously users can use dbj dll with no "special" header, if they wish to do so.

*/
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <assert.h>
#include <intrin.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#define DBJ_EXTERN_C_BEGIN extern "C" {
#define DBJ_EXTERN_C_END }
#else  // ! __cplusplus
#define DBJ_EXTERN_C_BEGIN
#define DBJ_EXTERN_C_END
#endif  // !__cplusplus

/*

DBJCS == DBJ Component System

one dbj component == one dll == one component interface == one struct

same def for all dll's:

EXPORTS
dbj_component_can_unload_now    PRIVATE
interface_factory               PRIVATE
dbj_component_version           PRIVATE

NOTE:

A module-definition or DEF file (*.def) is a text file containing one or more
module statements that describe various attributes of a DLL.
If you are not using the __declspec(dllexport) keyword to export the DLL's
functions, the DLL requires a DEF file, so  that users can call them.

BEWARE:

Visual Studio will hapilly build dll without a def file. And withot
__declspec(dllexport)

TO BUILD DBJ DLL YOU MUST!

Create the DEF file yourself and add it to your project. Then go to Project >
Properties > Linker > Input > Module Definition File and enter the name of the
DEF file. Repeat this step for each configuration and platform, or do it all at
once by selecting Configuration = All Configurations, and Platform = All
Platforms.

*/
#define DBJCS_CAN_UNLOAD_NOW_STR "dbj_component_can_unload_now"
#define DBJCS_INTEFACE_FACTORY_STR "interface_factory"
#define DBJCS_COMPONENT_VERSION_STR "dbj_component_version"

#define DBJCS_CAN_UNLOAD_NOW_FUN dbj_component_can_unload_now
#define DBJCS_INTEFACE_FACTORY_FUN interface_factory
#define DBJCS_COMPONENT_VERSION_FUN dbj_component_version

#ifdef _DEBUG
/*
this means we need to debug from Visual Studio (or VS Code using inner
terminal), to actually see the debug output which in turn means we do not need
to build console testing apps which in turn is moving us closer to real life
runtimes inside services or win32 desktop apps or some such thing
*/
static inline void DBJ_DBG_PRINT(const char *format_, ...) {
  char buffy[1024] = {0}; // magical number; not exemplary
  va_list args = 0;
  va_start(args, format_);
  int nBuf = _vsnprintf(buffy, 1024, format_, args);
  if (nBuf > -1)
    OutputDebugStringA(buffy);
  else
    OutputDebugStringA(__FILE__ " OutputDebugStringA buffer overflow\n");
  va_end(args);
}
#else
#define DBJ_DBG_PRINT(format_, ...) ((void)0)
#endif

// TERROR == Terminating Errpr
#define DBJ_TERROR(format_, ...)         \
  do {                                   \
    DBJ_DBG_PRINT(format_, __VA_ARGS__); \
    exit(EXIT_FAILURE);                  \
  } while (0);
