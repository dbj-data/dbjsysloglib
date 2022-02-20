
/* ---------------------------------------------------------------------------------
(c) 2021-2022 by dbj@dbj.org https://license_dbj

basically in here we connect interface implementation with actual implementation
*/

#include "dbjsyslogclient.h" // public component header for users to use
#include "core/dbjsyslog.h"  // the implementation is hidden

#include <dbj-dll/dbj-dllimp.h> // for developers implementing the dbj dll components

DBJ_COMPONENT_VERSION_IMPLEMENTATION(0, 1, 0, DBJ_DLL_CAPTION);

DBJ_COMPONENT_UNLOADER_IMPLEMENTATION();
/* 
---------------------------------------------------------------------------------
  private instance of the interface implementation
 */
static dbjsyslog_client implementation_ = {
    // here connect function pointers of the public interface
    // to the implementations
    .is_syslog_initialized = is_syslog_initialized,
    .dbj_syslog_initalize = dbj_syslog_initalize,
    .emergency = syslog_emergency,
    .alert = syslog_alert,
    .critical = syslog_critical,
    .error = syslog_error,
    .warning = syslog_warning,
    .notice = syslog_notice,
    .info = syslog_info,
    .debug = syslog_debug
    // eof interface
};

// each dbj dll "component"
// must have exported "factory function"
// with the predefined name "dbj_dll_interface_factory"
// hint: see the def file in this folder
// that function return single interface implementation, as a pointer
dbjsyslog_client *dbj_dll_interface_factory(void) { return &implementation_; }