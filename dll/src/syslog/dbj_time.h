#pragma once
// (c) dbj at dbj dot org
#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <stdint.h>
#include <stdio.h> // to be using <strsafe.h>
#include <string.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct dbj_time_ {
  int64_t ticks;

  char* month_tag;
  /* WORD is unsigned short */
  unsigned short year;
  unsigned short month;
  unsigned short day;
  unsigned short hour;
  unsigned short minute;

  int64_t seconds;       
  int64_t milliseconds;
  int64_t microseconds;  
  int64_t nanoseconds;   // 1 tick is 100 nanoseconds as per REMARKS here shorturl.at/vzTW3
} dbj_time_t;

// Gets the current number of ticks from QueryPerformanceCounter.
// return 0 if the call to QueryPerformanceCounter fails.
// asserts in debug builds
static inline BOOL dbj_get_time(dbj_time_t* time_arg_) {
  assert(time_arg_);

  LARGE_INTEGER ticks;
  if (!QueryPerformanceCounter(&ticks)) {
    assert(FALSE && "QueryPerformanceCounter() failed\n");
    perror("QueryPerformanceCounter() failed\n");
    return FALSE;
  }

  time_arg_->ticks = ticks.QuadPart;
// as per REMARKS here 
// https://docs.microsoft.com/en-us/dotnet/api/system.timespan.ticks?redirectedfrom=MSDN&view=net-6.0#System_TimeSpan_Ticks
  time_arg_->nanoseconds = ticks.QuadPart / 100 ;

  // if you want microseconds use:  (double)ticks.QuadPart / 1000000.0;
  time_arg_->microseconds = ticks.QuadPart / 1000000;

  // from that call to this call some nanosecs MIGHT have passed
  // thus we will have to reimplement this to compute from ticks
  // but that is very tricky because of leap years and calendar in general
  // although the time span might be less than a nanosecond .. so lets see
  SYSTEMTIME syst_ = {0};
  GetSystemTime(&syst_);

  time_arg_->year = syst_.wYear;
  time_arg_->month = syst_.wMonth;
  time_arg_->day = syst_.wDay;
  time_arg_->hour = syst_.wHour;
  time_arg_->minute = syst_.wMinute;
  time_arg_->seconds = syst_.wSecond;
  time_arg_->milliseconds = syst_.wMilliseconds;

  static char* const month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  time_arg_->month_tag = month[syst_.wMonth - 1];

  return TRUE;
}

// 64 is too much
#define DBJ_TIMESTAMP_RFC_3164_BUF_LEN 64
/* THIS IS https://tools.ietf.org/html/rfc3164 FORMAT timestamp
 *   RFC3164 max time resolution is second
 *   size of datagram must be much bigger than time stamp
 *   that goes imprinted on its left
 *   returns a pointer to static time stamp buffer
 *   NULL on error
 */
static inline const char* const dbj_syslog_time_stamp_rfc3164(void) {
  static char time_stamp[DBJ_TIMESTAMP_RFC_3164_BUF_LEN] = {0};
  dbj_time_t dbj_time = {0};
  if (FALSE == dbj_get_time(&dbj_time)) return FALSE;
  /* THIS IS https://tools.ietf.org/html/rfc3164 FORMAT */
  int len = sprintf_s(time_stamp, DBJ_TIMESTAMP_RFC_3164_BUF_LEN,
                      "%s %2d %02d:%02d:%02d", dbj_time.month_tag, dbj_time.day,
                      dbj_time.hour, dbj_time.minute, dbj_time.seconds);
  if (len < 1) {
    assert(FALSE && "sprintf_s has failed\n");
    perror("sprintf_s has failed\n");
    return 0;
  }
  // be usre to use it immediately!
  return time_stamp;
}

#define DBJ_TIMESTAMP_RFC_5424_BUF_LEN 32

// https://datatracker.ietf.org/doc/html/rfc5424#page-11
static inline const char * const dbj_syslog_time_stamp_rfc5424(void) 
{
  static char time_stamp[DBJ_TIMESTAMP_RFC_5424_BUF_LEN] = {0};
  dbj_time_t dbj_time = {0};
  if (FALSE == dbj_get_time(&dbj_time)) return FALSE;

  /* THIS IS https://datatracker.ietf.org/doc/html/rfc5424#page-11 FORMAT

 full datagram is this:

 len = sprintf_s(datagramm, sizeof(datagramm),
                  "<%d>1 %4d-%02d-%02dT%02d-%02d-%02d.03%d %s %s %s :",
                  pri,
                  stm.wYear, stm.wMonth, stm.wDay, stm.wHour, stm.wMinute,
     stm.wSecond, stm.wMilliseconds, local_hostname, syslog_ident,
     syslog_procid_str);

   */
  int len = sprintf_s(time_stamp, DBJ_TIMESTAMP_RFC_5424_BUF_LEN,
                      "%4d-%02d-%02dT%02d:%02d:%02d.%03d", dbj_time.year,
                      dbj_time.month, dbj_time.day, dbj_time.hour,
                      dbj_time.minute, dbj_time.seconds, dbj_time.milliseconds);
  if (len < 1) {
    assert(FALSE && "sprintf_s has failed\n");
    perror("sprintf_s has failed\n");
    return 0;
  }
  // be usre to use it immediately!
  return time_stamp;
}

#ifdef __cplusplus
}
// extern "C" {
#endif
