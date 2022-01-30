#ifndef DBJ_COMPONENT_INCLUDED
#define DBJ_COMPONENT_INCLUDED
/* (c) 2019 - 2021 by dbj.org   -- https://dbj.org/license_dbj

*/
#include "dbj-dll.h"

// dbj-component working or not in the presence of MT is left to the
// requirements, ie. it is not controlled by dbj dll component design

DBJ_EXTERN_C_BEGIN

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
  bool DBJCS_CAN_UNLOAD_NOW_FUN(void) { return component_counter_ == 0; }

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
  struct dbj_component_version_ DBJCS_COMPONENT_VERSION_FUN(void) {           \
    static struct dbj_component_version_ the_version_ = {MA_, MI_, PA_, {0}}; \
    if (0 == the_version_.description[0]) {                                   \
      strcpy_s(the_version_.description, 0xFF, S_);                           \
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

DBJ_EXTERN_C_END

#endif  // DBJ_COMPONENT_INCLUDED