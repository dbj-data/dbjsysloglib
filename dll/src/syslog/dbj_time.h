#pragma once
#ifndef DBJ_TIME_INC_
#define DBJ_TIME_INC_
// (c) dbj at dbj dot org
#define _CRT_SECURE_NO_WARNINGS 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h> // to be using <strsafe.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma region  linux time things
// MSVC defines this in winsock2.h!?
typedef struct {
  long tv_sec;
  long tv_usec;
} dbj_timeval;

__inline int gettimeofday(dbj_timeval* tp /*, struct timezone* ignored_*/) {
  // Note: some broken versions only have 8 trailing zero's, the correct epoch
  // has 9 trailing zero's This magic number is the number of 100 nanosecond
  // intervals since January 1, 1601 (UTC) until 00:00:00 January 1, 1970
  static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

  SYSTEMTIME system_time;
  FILETIME file_time;
  uint64_t time;

  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time, &file_time);
  time = ((uint64_t)file_time.dwLowDateTime);
  time += ((uint64_t)file_time.dwHighDateTime) << 32;

  tp->tv_sec = (long)((time - EPOCH) / 10000000L);
  tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
  return 0;
}
#if ((defined _WIN32))
#define gettimeofday win_gettimeofday
#endif

#pragma endregion // linux time things

 #pragma region standard time stamps
/*-------------------------------------------------------------------
https://stackoverflow.com/a/48772690/10870835
*/
__inline char* const dbj_rfc3399(void) {
  time_t now;
  time(&now);
  struct tm* p = localtime(&now);
  static char buf[100] = {0};
  size_t len = strftime(buf, sizeof buf - 1, "%FT%T%z", p);
  // move last 2 digits
  if (len > 1) {
    char minute[] = {buf[len - 2], buf[len - 1], '\0'};
    (void)sprintf(buf + len - 2, ":%s", minute);
  }
  return buf;
  // printf("\"%s\"\n", buf);
}
  /*-------------------------------------------------------------------
https://en.cppreference.com/w/c/chrono/timespec
*/
__inline char* const posix_timestamp_(void) {
  struct timespec ts;
  int ret = timespec_get(&ts, TIME_UTC);
  assert(ret);
  (void)ret;
  static char buff[100] = {0};
  strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
  // now add the nanoseconds part
  ret = sprintf_s(buff, 100, "%s.%09ld UTC", buff, ts.tv_nsec);
  assert(ret);
  (void)ret;
  return buff;
  /*

  printf("Current time: %s.%09ld UTC\n", buff, ts.tv_nsec);
  printf("Raw timespec.time_t: %jd\n", (intmax_t)ts.tv_sec);
  printf("Raw timespec.tv_nsec: %09ld\n", ts.tv_nsec);

  Possible output:

  Current time: 11/24/21 03:10:50.408191283 UTC
  Raw timespec.time_t: 1637723450
  Raw timespec.tv_nsec: 408191283
  */
}
/*-------------------------------------------------------------------
win32 implementation of linux sys/time.h function
*/



// ISO8601 timestamp
// inspired by: https://stackoverflow.com/a/42436648/10870835
// WARNING! reuses internal static buffer
__inline char* const iso8601(void) {
  struct tm tmNow = {0};

  time_t now = time(NULL);  // Get the current time
  _localtime64_s(&tmNow, &now);

  static char bufferTime[26] = {0};
  char bufferTimezoneOffset[6] = {0};
  size_t tsizTime =
      strftime(bufferTime, 26, "%Y-%m-%dT%H:%M:%S",
               &tmNow);  // The current time formatted "2017-02-22T10:00:00"
  
  /*size_t tsizOffset =*/(void)strftime(bufferTimezoneOffset, 6, "%z",
                               &tmNow);  // The timezone offset -0500

  strncpy_s(&bufferTime[tsizTime], 26, bufferTimezoneOffset,
            3);                    // Adds the hour part of the timezone offset
  bufferTime[tsizTime + 3] = ':';  // insert ':'
  strncpy_s(&bufferTime[tsizTime + 4], 26, &bufferTimezoneOffset[3],
            3);         // Adds the minutes part of the timezone offset
  return (bufferTime);  // example: "2022-02-22T10:00:00-05:00"
}

__inline int test_standard_timestamps( void ) {
  for (int k = 0; k < 0xF; ++k) {
    printf("\nISO8601: %s", iso8601());
    //
    printf("\tPOSIX  : %s\n", posix_timestamp_());
    //
    Sleep(1000);
  }
  return 42;
}

#pragma endregion // standard time stamps

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
__inline BOOL dbj_get_time(dbj_time_t* time_arg_) {
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

// 64 is too much, even 32 is more than required
#define DBJ_TIMESTAMP_RFC_3164_BUF_LEN 32
/* THIS IS https://tools.ietf.org/html/rfc3164 FORMAT timestamp
 *   RFC3164 max time resolution is second
 *   size of datagram must be much bigger than time stamp
 *   that goes imprinted on its left
 *   returns a pointer to static time stamp buffer
 *   NULL on error
 * 
 *   NOTE: make no mistake time stamp is important for syslog
 *         if not properly done syslog servers will make mistakes
 *         messages will be interpeted wrongly etc...
 */
__inline const char* const dbj_syslog_time_stamp_rfc3164(void) {
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
  // be sure to use immediately!
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
  // be sure to use immediately!
  return time_stamp;
}

#ifdef __cplusplus
}
// extern "C" {
#endif

#endif  // DBJ_TIME_INC_
