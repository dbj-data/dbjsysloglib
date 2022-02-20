#define _CRT_SECURE_NO_WARNINGS 1
/*
        DBJ Changes :
        -- always show the process ID, asked or not
        -- if identity (aka tag) is not given it will be called: "Anonymous"

        NOTE!

        These public functions are not used by end users, they are hidden behind

        dbjsylog[.h|.c]

        Thus to make solutions in here simple there is no locking.
        Locking is implemented in : dbjsylog.c

        TODO: use <strsafe.h>
*/
#ifndef __clang__
#error Please use clang compiler. clang-cl is part of Visual Studio
#endif
#if __STDC_VERSION__ < 201112L
#error please set the C language minimum to C11
#endif
#include <stdint.h>
#include <stdlib.h>

#include "dbj_strsafe.h"
#include "dbj_time.h"

// string.h
void* memcpy(void* dest, const void* src, size_t count);
void* memset(void* dest, int ch, size_t count);
// C11
errno_t memset_s(void* dest, rsize_t destsz, int ch, rsize_t count);

/*
 * Copyright (c) 2008 Secure Endpoints Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Secure Endpoints Inc. nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission from Secure Endpoints Inc..
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Based on code by Alexander Yaworsky
 *
 */
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <assert.h>  // dbj: _Static_assert, C11
#include <stdio.h>
// dbj: using strsafe.h #include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define SYSLOG_NAMES

#include "syslog.h"

// dbj added
#if (!defined(SYSLOG_RFC3164)) && (!defined(SYSLOG_RFC5424))
#error Please define SYSLOG_RFC3164 or SYSLOG_RFC5424
#endif

static BOOL syslog_opened = FALSE;

static int syslog_mask = 0xFF;
static char syslog_ident[128] = "";
static int syslog_facility = LOG_USER;
static char syslog_procid_str[20] = {0};

static SOCKADDR_IN syslog_hostaddr = {0};
static SOCKET syslog_socket = INVALID_SOCKET;
static char local_hostname[MAX_COMPUTERNAME_LENGTH + 1];

// dbj todo: ini file reader to be added and used
static char syslog_hostname[MAX_COMPUTERNAME_LENGTH + 1] = "localhost";
static unsigned short syslog_port = SYSLOG_PORT;

static int datagramm_size = 0;

volatile BOOL initialized = FALSE;
static BOOL wsa_initialized = FALSE;

/*
 * ----------------------------------------------------------------------------------
 * public function implementations are bellow
 */
BOOL is_syslog_initialized() { return initialized; }

void init_syslog(const char* hostname) {
  WSADATA wsd;
  char* service;

  if (initialized) return;

  if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
    // dbj: fail in debug builds
    assert(FALSE && "Can't initialize WinSock 2.2\n");
    // dbj: proceed uninitalized in release builds
    perror("Can't initialize WinSock 2.2\n");
    /* we do not let the rest of the initialization code go through,
       thus none of the syslog calls would succeed. */
    wsa_initialized = FALSE;
    return;
  } else {
    wsa_initialized = TRUE;
  }
  // DBJ: this is C11
  _Static_assert(sizeof(syslog_hostname) > 1, "syslog_hostname is too short");

  if (hostname)
    strcpy_s(syslog_hostname, sizeof(syslog_hostname), hostname);
  else
    strcpy_s(syslog_hostname, sizeof(syslog_hostname), "Unknown");

  service = strchr(syslog_hostname, ':');

  if (service) {
    int tp;

    *service++ = '\0';

    if ((tp = atoi(service)) <= 0) {
      struct servent* se;

      se = getservbyname(service, "udp");

      syslog_port = (se == NULL) ? SYSLOG_PORT : se->s_port;
    } else {
      syslog_port = (unsigned short)tp;
    }
  } else {
    syslog_port = SYSLOG_PORT;
  }

  initialized = TRUE;

  // dbj removed this :atexit(exit_syslog);
  // see the clang destructor bellow
}

// as of today 2022 FEB 13 we hide this one here
// users can  reach it only if they know it exist
__attribute__((destructor)) void exit_syslog(void) {
  if (!initialized) return;

  closelog();

  if (wsa_initialized) WSACleanup();

  initialized = FALSE;
}

static void init_logger_addr() {
  struct hostent* phe = NULL;

  memset(&syslog_hostaddr, 0, sizeof(SOCKADDR_IN));
  syslog_hostaddr.sin_family = AF_INET;

  if (syslog_hostname[0] == '\0') goto use_default;

  phe = gethostbyname(syslog_hostname);
  if (!phe) goto use_default;

  memcpy(&syslog_hostaddr.sin_addr.s_addr, phe->h_addr, phe->h_length);

  syslog_hostaddr.sin_port = htons(syslog_port);
  return;

use_default:
  syslog_hostaddr.sin_addr.S_un.S_addr = htonl(0x7F000001);
  syslog_hostaddr.sin_port = htons(SYSLOG_PORT);
}

/******************************************************************************
 * closelog
 *
 * Close desriptor used to write to system logger.
 */
void closelog() {
  if (!initialized) return;

  if (syslog_opened) {
    closesocket(syslog_socket);
    syslog_socket = INVALID_SOCKET;
    syslog_opened = FALSE;
  }
}

/******************************************************************************
 * openlog
 *
 * Open connection to system logger.
 */
void openlog(const char* ident, int option, int facility) {
  BOOL failed = FALSE;
  SOCKADDR_IN sa_local = {0};
  DWORD n = 0;
  int size = 0;

  /*DBJ added */
  if (!ident) ident = "Anonymous";

  if (!initialized) {
    assert(0 && "Warning: dbj syslog not initialized?");
    perror("Warning: dbj syslog not initialized?");
    return;
  }

  if (syslog_opened) goto done;

  failed = TRUE;

  syslog_facility = facility ? facility : LOG_USER;

  /*
  DBJ Changed: always show the process ID, asked or not
  */
  /*if( option & LOG_PID )*/
  dbjwin_sprintfa(syslog_procid_str, sizeof(syslog_procid_str), "[pid:%lu]",
                  GetCurrentProcessId());
  /*else
      syslog_procid_str[0] = '\0'; */

  n = sizeof(local_hostname);
  if (!GetComputerNameA(local_hostname, &n)) goto done;

  syslog_socket = INVALID_SOCKET;

  init_logger_addr();

  for (n = 0;; n++) {
    syslog_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == syslog_socket) goto done;

    memset(&sa_local, 0, sizeof(SOCKADDR_IN));
    sa_local.sin_family = AF_INET;
    if (bind(syslog_socket, (SOCKADDR*)&sa_local, sizeof(SOCKADDR_IN)) == 0)
      break;
    closesocket(syslog_socket);
    syslog_socket = INVALID_SOCKET;
    if (n == 100) goto done;
    Sleep(0);
  }

  /* get size of datagramm */
  size = sizeof(datagramm_size);
  if (getsockopt(syslog_socket, SOL_SOCKET, SO_MAX_MSG_SIZE,
                 (char*)&datagramm_size, &size))
    goto done;
  if (datagramm_size - strlen(local_hostname) - (ident ? strlen(ident) : 0) <
      64)
    goto done;
  if (datagramm_size > SYSLOG_DGRAM_SIZE) datagramm_size = SYSLOG_DGRAM_SIZE;

  if (ident) strcpy_s(syslog_ident, sizeof(syslog_ident), ident);

  syslog_facility = (facility ? facility : LOG_USER);
  failed = FALSE;

done:
  if (failed) {
    if (syslog_socket != INVALID_SOCKET) closesocket(syslog_socket);
  }
  syslog_opened = !failed;
}

/******************************************************************************
 * setlogmask
 *
 * Set the log mask level.
 */
int setlogmask(int mask) {
  int ret;

  if (!initialized) {
    assert(0 && "Warning: dbj syslog not initialized?");
    perror("Warning: dbj syslog not initialized?");
    return 0;
  }

  ret = syslog_mask;
  if (mask) syslog_mask = mask;

  return ret;
}

void vsyslog(int pri, const char* fmt, va_list ap);
/******************************************************************************
 * syslog
 *
 * Generate a log message using FMT string and option arguments.
 */
void syslog(int pri, const char* fmt, ...) {
  if (!initialized) /* dbj added */
  {
    assert(0 && "Warning: dbj syslog not initialized?");
    perror("Warning: dbj syslog not initialized?");
    return;
  }

  // Caution! the message must be smaller than SYSLOG_DGRAM_SIZE
  // char  message_[SYSLOG_DGRAM_SIZE] = {0};

  va_list ap;
  va_start(ap, fmt);
  vsyslog(pri, fmt, ap);
  va_end(ap);
}

void syslog_send(int pri, const char* message_);

// not in the header
void vsyslog(int pri, const char* fmt, va_list ap) {
  if (!initialized) /* dbj added */
  {
    assert(0 && "ERROR: dbj syslog not initialized?");
    perror("Warning: dbj syslog not initialized?");
    return;
  }

  // Caution! the message must be smaller than SYSLOG_DGRAM_SIZE
  char message_[SYSLOG_DGRAM_SIZE] = {0};

  // va_start(ap, fmt);
  // or -- https://linux.die.net/man/3/vsnprintf
  if (vsprintf_s(message_, sizeof(message_), fmt, ap) < 0) {
    assert(0 && "vsprintf_s() failed?\n\n" __FILE__);
    perror("vsprintf_s() failed?\n\n" __FILE__);
    exit(1);
  }
  // va_end(ap);

  syslog_send(pri, message_);
}

/******************************************************************************
 *
 * Generate a log message using FMT and using arguments pointed to by AP.
 */
static void syslog_send(int priority_, const char* message_) {
  assert(message_);

  char datagramm[SYSLOG_DGRAM_SIZE] = {0};
  char* p = 0;

  if (!initialized) return;

  if (!(LOG_MASK(LOG_PRI(priority_)) & syslog_mask)) goto done;

  openlog(NULL, 0, priority_ & LOG_FACMASK);
  if (!syslog_opened) goto done;

  if (!(priority_ & LOG_FACMASK)) priority_ |= syslog_facility;

#ifdef SYSLOG_RFC3164
  // NOTE: time stamp format is IMPORTANT!
  HRESULT len = dbjwin_sprintfa(
      datagramm, sizeof(datagramm), "<%d>%s %s %s %s: %s", priority_,
      dbj_syslog_time_stamp_rfc3164() , local_hostname,
      syslog_procid_str, syslog_ident, message_);
#elif defined(SYSLOG_RFC5424)
  HRESULT len = dbjwin_sprintfa(
      datagramm, sizeof(datagramm), "<%d>1 %s %s %s %s - - %s", priority_,
      dbj_rfc3399(), local_hostname, syslog_procid_str, syslog_ident, message_);
#else
#error SYSLOG_RFC3164 or SYSLOG_RFC5424 have to be defined
#endif

  assert(len == S_OK);
  (void)len;

#ifdef DBJ_SYSLOG_CLEAN_MSG

  /* dbj comment: this does clean them all into spaces */
  while ((p = strchr(datagramm, '\n'))) {
    if (p) *p = ' '; /* 0; dbj replaced 0 with space */
  }
  while ((p = strchr(datagramm, '\t'))) {
    if (p) *p = ' '; /* 0; dbj replaced 0 with space */
  }
  while ((p = strchr(datagramm, '\r'))) {
    if (p) *p = ' '; /* 0; dbj replaced 0 with space */
  }
  while ((p = strchr(datagramm, '\v'))) {
    if (p) *p = ' '; /* 0; dbj replaced 0 with space */
  }
  while ((p = strchr(datagramm, '\f'))) {
    if (p) *p = ' '; /* 0; dbj replaced 0 with space */
  }
#endif  // DBJ_SYSLOG_CLEAN_MSG

  sendto(syslog_socket, datagramm, (int)strlen(datagramm), 0,
         (SOCKADDR*)&syslog_hostaddr, sizeof(SOCKADDR_IN));
done:
  __noop;
}
