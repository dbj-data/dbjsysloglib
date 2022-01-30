#ifndef DBJ_COMPONENT_INCLUDED
#define DBJ_COMPONENT_INCLUDED
/* (c) 2019 - 2021 by dbj.org   -- https://dbj.org/license_dbj

DBJCS == DBJ Component System

one dbj component == one dll == one component interface == one struct

same def for all dll's:

EXPORTS
dbj_component_can_unload_now    PRIVATE
interface_factory           PRIVATE
dbj_component_version           PRIVATE

NOTE:

A module-definition or DEF file (*.def) is a text file containing one or more 
module statements that describe various attributes of a DLL. 
If you are not using the __declspec(dllexport) keyword to export the DLL's functions, 
the DLL requires a DEF file.

DITTO: 

Visual Studio will hapilly build dll without a def files.

YOU MUST!

Create the DEF file yourself and add it to your project. Then go to Project >
Properties > Linker > Input > Module Definition File and enter the name of the
DEF file. Repeat this step for each configuration and platform, or do it all at
once by selecting Configuration = All Configurations, and Platform = All
Platforms.

*/

#ifdef __cplusplus
#define DBJ_EXTERN_C_BEGIN extern "C" {
#define DBJ_EXTERN_C_END }
#else  // ! __cplusplus
#define DBJ_EXTERN_C_BEGIN
#define DBJ_EXTERN_C_END
#endif  // !__cplusplus

// #include <dbj_capi/ccommon.h>
// #include <dbj_capi/dbj_windows_include.h>
#include <intrin.h>
#include <stdbool.h>
#include <string.h>

// dbj-component working or not in the presence of MT is left to the
// requirements, ie. it is not controlled by dbj-component design

DBJ_EXTERN_C_BEGIN

/*
one dbj component == one dll == one component interface == one struct

same def for all component dll's:

EXPORTS
dbj_component_can_unload_now    PRIVATE
interface_factory           PRIVATE
dbj_component_version           PRIVATE

each is exported in a def file for each component
and is always the same name
regardless of the return type or arguments

*/
#define DBJCS_CAN_UNLOAD_NAME dbj_component_can_unload_now
#define DBJCS_FACTORYNAME interface_factory
#define DBJCS_SEMVER_NAME dbj_component_version

// dbj_component_can_unload_now is part of each dbj-component definition
// use this macro to implement it inside a component dll
// dbj_component_can_unload_now() is required by dbj-component-loader
#define DBJ_COMPONENT_UNLOADER_IMPLEMENTATION(...)                \
  static volatile long component_counter_ = 0;                    \
                                                                  \
  __attribute__((constructor)) static void component_ctor(void) { \
    _InterlockedIncrement(&component_counter_);                   \
  }                                                               \
                                                                  \
  __attribute__((destructor)) static void component_dtor() {      \
    _InterlockedDecrement(&component_counter_);                   \
  }                                                               \
  bool DBJCS_CAN_UNLOAD_NAME(void) { return component_counter_ == 0; }

#pragma region component semver

// each dbj-component has to exhibit semantic versioning 3 values
// and description string up to 0xFF chars long
// this structure is required for client code
struct dbj_component_version_ {
  unsigned major;
  unsigned minor;
  unsigned patch;
  char description[0xFF];
};

// use this macro to implement versioning inside a component
#define DBJ_COMPONENT_VERSION_IMPLEMENTATION(MA_, MI_, PA_, S_)               \
  struct dbj_component_version_ DBJCS_SEMVER_NAME(void) {                     \
    static struct dbj_component_version_ the_version_ = {MA_, MI_, PA_, {0}}; \
    if (0 == &the_version_.description[0]) {                                  \
      memcpy(the_version_.description, S_, 0xFF);                             \
    }                                                                         \
    return the_version_;                                                      \
  }

#pragma endregion  // component semver

/*
function footprints for these two are always the same
for them to be used client code need funcion pointer
declarations

thus these two are always the same and declared in here
and nowehere else
*/
typedef struct dbj_component_version_ (*DBJ_COMPONENT_SEMVER_FP)(void);
typedef bool (*DBJ_COMPONENT_UNLOAD_FP)(void);

/*
factory function differs becuase it returns a pointer to the struct
that represents the component interface
We could made it return void *, but that would impose potentialy run-time
consuming casting
*/

DBJ_EXTERN_C_END

#endif  // DBJ_COMPONENT_INCLUDED