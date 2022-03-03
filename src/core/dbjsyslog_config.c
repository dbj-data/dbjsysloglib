// inih should be on da path
#include "dbj-win/dbj_strsafe.h"
#include "dbjsyslog.h"
#include "inih/ini.h"

// BEWARE: consume the result immediately!
static inline LPSTR inifile_full_path(const char inifilename_[static 1]) {
  static char buffy[MAX_PATH] = {0};

  if (0 == GetModuleFileNameA(NULL, buffy, MAX_PATH)) {
    assert(FALSE && __FILE__ " : GetModuleFileNameA() failed");
  }

  char* last_slash = strrchr(buffy, '\\');

  assert(last_slash && __FILE__ "Last slash not found?" );

  memset(last_slash, 0, &buffy[MAX_PATH] - last_slash);

  if (S_OK != dbjwin_sprintfa(buffy, 1024, "%s\\%s", buffy, inifilename_)) {
    assert(FALSE && __FILE__ " : dbjwin_sprintfa() failed");
  }
  return buffy;
}

/* set defaults here */
static configuration config_ = {{0}, 514};

__attribute__((constructor)) static void set_config_defaults(void) {
  (void)dbjwin_strncpya(config_.url, 0xFF, "localhost", 0xF);
  config_.port = 514;
}
// ---------------------------------------------------------------------------
// section name is "" not null if not used in ini file
__inline int ini_section_name_value_handler(void* user, const char* section,
                                            const char* name,
                                            const char* value) {
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

  /*
  undefined ini file section name results in empty string here, not NULL
  */

  if (MATCH("", "port")) {
    config_.port = atoi(value);
    return 1;  // signal all is OK
  }

  if (MATCH("", "url")) {
    HRESULT res = dbjwin_strncpya(config_.url, sizeof(config_.url), value,
                                  dbjwin_strnlena(value, sizeof(config_.url)));
    if (res != S_OK) return 0;  // error
    return 1;                   // signal all is OK
  }

  return 0; /* signal the unknown section/name, error */

#undef MATCH
}

// ----------------------------------------------------------------------------
// note: argc,argv are zeroed in here, do not use!
configuration dbjsyslog_config_read(int argc, char* argv[]) {
  // Returns 0 on success
  int retval_ = ini_parse(inifile_full_path(INI_FILE_NAME),
                          ini_section_name_value_handler, 0);
  // ignore ini usage error, defaults are hard coded, are they not?
  (void)retval_;
  return config_;
}