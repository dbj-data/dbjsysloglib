#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../dll/dbjsyslogclient.h" // component interface declaration

#include <dbj-dll/dbj-dlluser.h>

extern int multi_threading_driver_(dbjsyslog_client* syslog_);
    //
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hInstance);
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);

 // this is standard dbj dll component usage
 // dll is dynamicaly loaded
  HINSTANCE dll_hinst_ = dbj_dll_load(DBJSYSLOGCLIENT_DLL_NAME);

  if (dll_hinst_)
  // report on the dll used is optional
    dbj_dll_version_report(dll_hinst_, DBJSYSLOGCLIENT_DLL_NAME);
  else
    return EXIT_FAILURE;

  // NOTE: errors are already reported; if any
  // NOTE: in debug builds one can see them in the debugger
  // Now onto the component usage
  // 1. obtain the factory function
  dbjsyslog_client_ifp dll_factory_ =
      (dbjsyslog_client_ifp)dbj_dll_get_factory_function(&dll_hinst_);
  assert(dll_factory_);
  
  // 2. call the factory function to obtain interface
  dbjsyslog_client* iface_ = dll_factory_();
  assert(iface_);

  // will use the localhost syslog server with the default identity
  // of this component base name with PID added
  iface_->dbj_syslog_initalize(0, "dbjsyslog MT testing");
  iface_->info("Using dbj syslog: %s", dbj_syslog_VERSION );

  multi_threading_driver_(iface_); // proceed with the multi_threading_driver_

  // 3. call the methods available
  //iface_->dbj_syslog_initalize(0, 0);
  //iface_->emergency("%s", "Emergency!");
  //iface_->alert("%s", "Alert!");
  //iface_->critical("%s", "Critical!");
  //iface_->error("%s", "Error!");
  //iface_->warning("%s", "Warning!");
  //iface_->info("Using dbj syslog: %s", dbj_syslog_VERSION );
  //iface_->debug("%s", "Debug!");

  // that is obviously pedestrian way of using it
  // users will probably use one or more macros in reality

  // remember syslog already posts this executable basename and PID with each message
  iface_->info("%s", "Leaving wWinMain()");

    return 42;
}
