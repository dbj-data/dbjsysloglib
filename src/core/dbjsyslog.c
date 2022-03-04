

#include "dbjsyslog.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "dbj-win/dbj_strsafe.h"
#include "syslog/syslog.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <crtdbg.h>
#include <shellapi.h>

extern configuration dbjsyslog_config_read(void);

/*
 * safe and slow-er: lock on each entry, unlock on each leave
 * since winsock is udp that is not that bad as one might be lead to believe
 */
static BOOL critical_section_syslog_initialized_ = FALSE;
static CRITICAL_SECTION critical_section_syslog_;

#define DBJ_LOCK EnterCriticalSection(&critical_section_syslog_)
#define DBJ_UNLOCK LeaveCriticalSection(&critical_section_syslog_)

// Initialize the critical section one time only.
__attribute__((constructor)) static void intialize_critical_section(void) {
  if (!InitializeCriticalSectionAndSpinCount(&critical_section_syslog_,
                                             0x00000400)) {
    assert(0 && "InitializeCriticalSectionAndSpinCount failed?");
    exit(EXIT_FAILURE);
  }
  // depending on the quality of clang constructor/destructor implementation
  // we can do it just like this in here
  // and set it to false in destructor bellow
  critical_section_syslog_initialized_ = TRUE;
}

// also delete it one time only
__attribute__((destructor)) static void delete_critical_section(void) {
  assert(critical_section_syslog_initialized_);
  DeleteCriticalSection(&critical_section_syslog_);
  critical_section_syslog_initialized_ = FALSE;
}
/*
----------------------------------------------------------------------------------
*/
#if 0
static inline char** __cdecl dbjwin_command_line_to_utf8_argv(int* o_argc) {
  int argc_ = 0;
  char** argv = 0;
  char* args = 0;

  LPWSTR* w_argv = CommandLineToArgvW(GetCommandLineW(), &argc_);
  _ASSERTE(w_argv != NULL);

  size_t size = wcslen(GetCommandLineW()) * 4;
  argv = (char**)calloc(1, (argc_ + 1) * sizeof(char*) + size);
  args = (char*)&argv[argc_ + 1];
  int n;
  for (int i = 0; i < argc_; ++i) {
    n = WideCharToMultiByte(CP_UTF8, 0, w_argv[i], -1, args, (int)size, NULL,
                            NULL);
    _ASSERTE(n != 0);
    argv[i] = args;
    size -= n;
    args += n;
  }
  LocalFree(w_argv);

  *o_argc = argc_;
  return argv;
}
#endif  // 0

/* note: this includes suffix in a result */
static char* basename(const char* full_path) {
  char *p = (char*)full_path, *pp = 0;
  while ((p = strchr(p + 1, '\\'))) {
    pp = p;
  }
  return (pp ? pp + 1 : p);
}

/* there must be a quicker and simpler way to do this in Windows? */
static const char* app_base_name() {
  //  int argc_ = 0;
  CHAR szFileNameA[MAX_PATH] = {0};
  int rez_ = GetModuleFileNameA(NULL, szFileNameA, MAX_PATH);
  assert(rez_ > 0);
  (void)rez_;
  return basename(szFileNameA);
  // return basename(dbjwin_command_line_to_utf8_argv(&argc_)[0]);
}

/* to initialize in this context means
   call init_syslog( ip_and_port ) and then

   openlog( id, option, facility );

   if id == NULL it will be this module base name
*/

void dbj_syslog_initalize(syslog_client_identity id) {
  DBJ_LOCK;
  /*
   * syslog client is one per process, not one per thread
   */
  if (is_syslog_initialized()) {
    goto done;
  }
  // read from ini file ... or use defaults
  configuration config_ = dbjsyslog_config_read();

  char ip_and_port[0xFF] = {0};

  if (S_OK != dbjwin_sprintfa(ip_and_port, sizeof(ip_and_port), "%s:%d",
                              config_.url, config_.port)) {
    assert(0 && "dbjwin_sprintfa failed");
    exit(EXIT_FAILURE);
  }

  // if ip_and_port == NULL, localhost is used
  // here will be passing datagram size coming from da ini file
  init_syslog(ip_and_port);

  // Facility is alway LOG_USER, and Option is always LOG_ODELAY
  // that is considered "normal" for component to identify its syslogs
  // Look up the latest RFC on the Option definition and decide
  // we can take these two from the ini file, but why?
  if (id.val == NULL) id.val = (char*)app_base_name();  // oops?!

  assert((id.val && (id.val[0] != '\x0')) && __FILE__
         " : id can not be empty here?");

  openlog(id.val, LOG_ODELAY, LOG_USER);

done:
  DBJ_UNLOCK;
}

// hidden:
void vsyslog(int /*__pri*/, const char* /* __fmt */, va_list);

static void syslog_call(int level_, const char* format_, va_list ap) {
  assert(is_syslog_initialized());
#ifndef _DEBUG
  if (!is_syslog_initialized()) {
    dbj_syslog_initalize((syslog_client_identity){NULL});
  }
#endif

  vsyslog(level_, format_, ap);
}
/*
to uninitialize means to call closelog() and do some other cleanup
exit_syslog() does that on exit so no need for users to do anything
*/

void syslog_emergency(const char* format_, ...) {
  DBJ_LOCK;
  va_list ap;
  va_start(ap, format_);
  syslog_call(LOG_EMERG, format_, ap);
  va_end(ap);
  DBJ_UNLOCK;
}
void syslog_alert(const char* format_, ...) {
  DBJ_LOCK;
  va_list ap;
  va_start(ap, format_);
  syslog_call(LOG_ALERT, format_, ap);
  va_end(ap);
  DBJ_UNLOCK;
}
void syslog_critical(const char* format_, ...) {
  DBJ_LOCK;
  va_list ap;
  va_start(ap, format_);
  syslog_call(LOG_CRIT, format_, ap);
  va_end(ap);
  DBJ_UNLOCK;
}
void syslog_error(const char* format_, ...) {
  DBJ_LOCK;
  va_list ap;
  va_start(ap, format_);
  syslog_call(LOG_ERR, format_, ap);
  va_end(ap);
  DBJ_UNLOCK;
}
void syslog_warning(const char* format_, ...) {
  DBJ_LOCK;
  va_list ap;
  va_start(ap, format_);
  syslog_call(LOG_WARNING, format_, ap);
  va_end(ap);
  DBJ_UNLOCK;
}
void syslog_notice(const char* format_, ...) {
  DBJ_LOCK;
  va_list ap;
  va_start(ap, format_);
  syslog_call(LOG_NOTICE, format_, ap);
  va_end(ap);
  DBJ_UNLOCK;
}
void syslog_info(const char* format_, ...) {
  DBJ_LOCK;
  va_list ap;
  va_start(ap, format_);
  syslog_call(LOG_INFO, format_, ap);
  va_end(ap);
  DBJ_UNLOCK;
}
void syslog_debug(const char* format_, ...) {
  DBJ_LOCK;
  va_list ap;
  va_start(ap, format_);
  syslog_call(LOG_DEBUG, format_, ap);
  va_end(ap);
  DBJ_UNLOCK;
}
