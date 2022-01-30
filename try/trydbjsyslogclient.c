#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../dll/dbjsyslogclient.h"
#include "../dll/dbj-light-loader.h"

//
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hInstance);
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);

  HINSTANCE dll_hinst_ = dbj_dll_load(DBJSYSLOGCLIENT_DLL_NAME);

  if (dll_hinst_)
    dbj_light_version_report(dll_hinst_, sizeof(DBJSYSLOGCLIENT_DLL_NAME),
                             DBJSYSLOGCLIENT_DLL_NAME);
  else
    return EXIT_FAILURE;

  // NOTE: errors are already reported; if any
  // NOTE: in debug builds one can see them in the debugger

  // now onto the normal usage
  // 1. obtain the interface factory function
  dbjsyslog_client_ifp dll_factory_ =
      (dbjsyslog_client_ifp)dbj_dll_get_function(&dll_hinst_, DBJCS_FACTORYNAME);

  assert(dll_factory_);
  // 2. call the factory to obtain interface
  dbjsyslog_client* iface_ = dll_factory_();
  // 3. call the methods available
  iface_->dbj_syslog_initalize(0, "PRIME");

  iface_->emergency("%s", "Emergency!");
  iface_->alert("%s", "Alert!");
  iface_->critical("%s", "Critical!");
  iface_->error("%s", "Error!");
  iface_->warning("%s", "Warning!");
  iface_->info("%s", "Info!");
  iface_->debug("%s", "Debug!");

  // NOTE! at_exit is called by using crt atexit function

  return 42;
}
