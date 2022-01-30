#pragma once

#include "dbj-dll.h"

#pragma region infrastructure

struct dbj_component_version_
{
    unsigned major;
    unsigned minor;
    unsigned patch;
    char description[0xFF];
};

typedef struct dbj_component_version_ (*DBJ_COMPONENT_SEMVER_FP)(void);
/* 
unloading is also exactly the same for every DBJ Component aka DLL
because each one of them implements this function
But then, should we bother and unload manually at all? Or just leave it to the OS.
*/
typedef bool (*DBJ_COMPONENT_UNLOAD_FP)(void);

/*
return dll handle
one is hoping OS is caching the dll in memory for the next call
returns null on error
*/
static inline HINSTANCE dbj_dll_load(
    /* 
    we search in the application folder or in the system32
    just a file name, not a path! 
    we deliberately do not allow paths for reason of security
    */
    const char dll_file_name_[static 1])
{
    HINSTANCE dll_handle_ = LoadLibraryExA(
        dll_file_name_,
        NULL,
        /* we limit the locations of dll#s we load deliberately! */
        LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);

    if (NULL == dll_handle_)
    {
        DBJ_DBG_PRINT("DLL Loader error: [%d]. "
                  "By design looking only in system folders and application folder."
                  " Could not find the DLL by name: %s",
                  GetLastError(), dll_file_name_);
    }
    // no exitm thus client has ti check the result 
    return dll_handle_;
}

/*
one is hoping OS is caching the dll + function, in memory for the next call;
returns null on error
*/
static inline void *dbj_dll_get_function(HINSTANCE *dll_handle_, char const fun_name_[static 1])
{
    // Funny fact: GetProcAddress has no unicode equivalent
    FARPROC result =
        GetProcAddress((HINSTANCE)*dll_handle_, fun_name_);
    if (result == 0)
    {
        DBJ_DBG_PRINT("\nFailed to find the address for a function: '%s'\n", fun_name_);
    }
    return result;
}

// returns EINVAL on erro, 0 on OK
static inline int dbj_light_version_report(HINSTANCE dll_handle_, const unsigned name_len, const char dll_name[const static name_len])
{
    // using the version info is exactly the same for every DBJ Component
    DBJ_COMPONENT_SEMVER_FP get_version =
        (DBJ_COMPONENT_SEMVER_FP)dbj_dll_get_function(
            &dll_handle_, DBJCS_COMPONENT_VERSION_STR);

    if (!get_version)
    {
      DBJ_DBG_PRINT(
          "\nCould not find function by name "
          "'%s' " DBJCS_COMPONENT_VERSION_STR);
        return EINVAL;
    }
    struct dbj_component_version_ info_ = get_version();
    DBJ_DBG_PRINT("\n %s version info", dll_name);
    DBJ_DBG_PRINT("\nMajor: %d, minor: %d, patch: %d", info_.major, info_.minor, info_.patch);
    DBJ_DBG_PRINT("\nDescription: %s", info_.description);
    return 0;
}

// returns EXIT_FAILURE on error
// returns EXIT_SUCCESS on OK
static inline int dbj_light_unload_dll(HINSTANCE dll_handle_, const unsigned name_len, const char dll_name[const static name_len])
{
    // unloading is also exactly the same for every DBJ Component
    DBJ_COMPONENT_UNLOAD_FP can_unload =
        (DBJ_COMPONENT_UNLOAD_FP)dbj_dll_get_function(&dll_handle_,
                                                      DBJCS_CAN_UNLOAD_NOW_STR);
    if (!can_unload)
    {
      DBJ_DBG_PRINT("\nCan not get the function by name : %s\n",
                    DBJCS_CAN_UNLOAD_NOW_STR);
        return EXIT_FAILURE;
    }

    if (can_unload())
        if (!FreeLibrary(dll_handle_))
        {
            DBJ_DBG_PRINT("\ndbjdll_load FreeLibrary failed. The DLL name is: %s\n", dll_name);
            return EXIT_FAILURE;
        }
    return EXIT_SUCCESS;
}

/*

DBJ DLL factory function returns a pointer to the inteface implementations
It differs between dbj DLL's becuase it returns a pointer to the struct
that represents the component interface.

obtain the pointer to the interface factory, example:

  DBJ_DLL_IFP( particular_client_ifp, interface_var_name, handle_to_dbj_dll );

*/
#define DBJ_DLL_IFP(FP_, N_, DLLHANDLE_) \
  FP_ N_ = (FP_)dbj_dll_get_function(&DLLHANDLE_, DBJCS_INTEFACE_FACTORY_STR)

/*
 
 So how do we obtain the type of a method on DBJ DLL interface?  (if we need one)

 __typeof__ is 20+ years old, GCC extension not in the ISO C standard; yet

 to obtain and hold, a functon pointer to a method on some DBJ DLL interface we use
  20+ years old GCC extension not in the ISO C standard; yet

  Example: we have the interface pointer (from above) but we want a type of some method
  lets assume 

  struct some_dbj_dll_interface {
  const char *(*itoa)(int);
  } ;

  DBJ_DLL_IFP( factory_fp, factory_, dll_handle ); 

  struct some_dbj_dll_interface * interface_  =  factory_();
  
  DBJ_TYPE_OF(interface_->itoa) itoa_func = interface_->itoa;

  After that we just use itoa_func()
*/
#define DBJ_TYPE_OF(IFACE_PTR_METHOD_NAME) \
    __typeof__(IFACE_PTR_METHOD_NAME)

#pragma endregion // infrastructure
