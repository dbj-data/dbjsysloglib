#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../dll/dbjsyslogclient.h"
#include "../dll/dbj-dlluser.h"

//
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hInstance);
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);


  HINSTANCE dll_hinst_ = dbj_dll_load(DBJSYSLOGCLIENT_DLL_NAME);

  // first report on the dbj dll used
  if (dll_hinst_)
    dbj_dll_version_report(dll_hinst_, sizeof(DBJSYSLOGCLIENT_DLL_NAME),
                             DBJSYSLOGCLIENT_DLL_NAME);
  else
    return EXIT_FAILURE;

  // NOTE: errors are already reported; if any
  // NOTE: in debug builds one can see them in the debugger

  // now onto the normal usage
  // 1. obtain the factory function
  dbjsyslog_client_ifp dll_factory_ =
      (dbjsyslog_client_ifp)dbj_dll_get_factory_function(&dll_hinst_,
                                                 DBJCS_INTEFACE_FACTORY_STR);

  assert(dll_factory_);
  // 2. call the factory function to obtain interface
  dbjsyslog_client* iface_ = dll_factory_();
  assert(iface_);
  // 3. call the methods available
  iface_->dbj_syslog_initalize(0, 0);

  iface_->emergency("%s", "Emergency!");
  iface_->alert("%s", "Alert!");
  iface_->critical("%s", "Critical!");
  iface_->error("%s", "Error!");
  iface_->warning("%s", "Warning!");
  iface_->info("%s", "Info!");
  iface_->debug("%s", "Debug!");

  // NOTE! at_exit is called by using crt atexit function deep inside this
  // syslog implementation; probably not a good idea
  // using clang constructor/destructor functions this might be done 
  // better as it will be controled by the user code.
  // above is a bit pedestrian process
  // client init code migt very likely be put in one function; perhaps
  /*
      // call this from a constructor function
       static inline dbjsyslog_client* dbjsyslog (void) 
       {
       static dbjsyslog_client* iface_ = 0;
       if (! iface_ ) {
         dbjsyslog_client_ifp dll_factory_ =
         (dbjsyslog_client_ifp)dbj_dll_get_factory_function(&dll_hinst_, DBJCS_INTEFACE_FACTORY_STR);
          assert(dll_factory_); 
          if (dll_factory_) {
          iface_ = dll_factory_();
          assert(iface_);
          }
         }
          return iface_ ;
       }

       __attribute__((constructor))
       void dbjsyslog_construct(void)
       {
          (void)dbjsyslog() ; // 
       }

       __attribute__((destructor))
       void dbjsyslog_destruct(void)
       {
          dbj_syslog_on_exit() ; // not there yet
       }
  */

  return 42;
}
