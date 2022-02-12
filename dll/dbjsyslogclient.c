
/* ---------------------------------------------------------------------------------
(c) 2021-2022 by dbj@dbj.org https://license_dbj

basically in here we connect interface implementation with actual implementation
*/

#include "dbjsyslogclient.h" // public component header for users
#include "src/dbjsyslog.h"  // the implementation

#include <dbj-dll/dbj-dllimp.h>


// max len is 0xFF - 1
// #define DBJ_DLL_CAPTION "dbj syslog client"

//#define DBJ_COMPONENT_VERSION_IMPLEMENTATION_(MA_, MI_, PA_, S_)                  \
//  struct dbj_component_version_ dbj_component_version_(void) {                    \
//    static struct dbj_component_version_ the_version_ = {MA_, MI_, PA_, {0}};     \
//    if (0 == &the_version_.description[0]) {                                      \
//      static_assert(caption_size < sizeof(the_version_.description));             \
//      memcpy(the_version_.description, caption, sizeof(the_version_.description); \
//    }                                                                             \
//    return the_version_;                                                          \
//  }

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
// must have exported factory function
// with the predefined name "dbj_dll_interface_factory"
// hint: see the def file in this folder
dbjsyslog_client *dbj_dll_interface_factory(void) { return &implementation_; }