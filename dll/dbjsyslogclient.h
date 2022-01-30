#pragma once
#ifndef __clang__
#error Obviously, this code requires clang compiler
#endif
/*
 * this is the public DBJ DLL header containing the
 * declaration of the interface struct, dll name
 * and fp type of the factory function returning
 * the pointer to the interface implementation
 * 
 * Remember this dll is built using a def file
 */

#pragma clang system_header

#ifdef _cplusplus
extern "C" {
#endif

// make sure build script creates this dll for this component
#define DBJSYSLOGCLIENT_DLL_NAME "dbjsyslogclient.dll"

// used mainly for sem ver struct
#ifdef _DEBUG
#define DBJ_DLL_CAPTION "dbj syslog client DEBUG build"
#else // RELEASE
#define DBJ_DLL_CAPTION "dbj syslog client release build"
#endif

// the interface declaration
// each ans every user function is to be obtained through the pointer to this
// strict
typedef struct dbjsyslog_client_ dbjsyslog_client;
struct dbjsyslog_client_ {
  // WARNING: if not initialized dbjsysloglob will simply exit
  int (*is_syslog_initialized)(void);
  /*

  syslog client must initialize before first use
  we need to do this manually so that we give the identity to the syslog client
  alternative is using the ini file, nut that is tying the dll to configuration
  i.e. it is not generic

  */
  void (*dbj_syslog_initalize)(const char* /* hostname */,
                               const char* /* id */);

  /*
  exit_syslog() is called on exit so no need for users to do anything
  but: be carefull: onexit() has a limited number of slots avaialble
  */

  void (*emergency)(const char* format_, ...);
  void (*alert)(const char* format_, ...);
  void (*critical)(const char* format_, ...);
  void (*error)(const char* format_, ...);
  void (*warning)(const char* format_, ...);
  void (*notice)(const char* format_, ...);
  void (*info)(const char* format_, ...);
  void (*debug)(const char* format_, ...);
};

// ifp == InterFace Factory F.Pointer type
// returns a pointer to the above interface implementation
// this is just a type representing a function pointer type to it
// this is not factory function declaration, usage example:
//
//  HINSTANCE dll_hinst_ = dbj_dll_load(DBJSYSLOGCLIENT_DLL_NAME);
//
// dbj_dll_get_function returns void *, thus the cast is necessary
//
//  dbjsyslog_client_ifp dll_factory_ =
//  (dbjsyslog_client_ifp)dbj_dll_get_function(
//    &dll_hinst_, DBJCS_INTEFACE_FACTORY_STR);
//
//   dbjsyslog_client* iface_ = dll_factory_();
//
// iface_->dbj_syslog_initalize(0, "PRIME");
//
typedef dbjsyslog_client* (*dbjsyslog_client_ifp)(void);

//
// interface factory function for this component is implemented
// inside the DLL, but name is pre-defined in the def file:
//
// struct dbjsyslog_client_ *interface_factory(void);
//
#ifdef _cplusplus
}
// extern "C"
#endif
