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

// https://docs.microsoft.com/en-gb/windows/win32/winsock/windows-sockets-error-codes-2?redirectedfrom=MSDN
#ifndef WSAEACCESS
#define WSAEACCESS 10013
#endif  // WSAEACCESS

#include "dbj-win/dbj_strsafe.h"
#include "dbj-win/dbj_time.h"

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

static struct {
  BOOL syslog_opened;
  int syslog_mask;
  char syslog_ident[128];
  int syslog_facility;
  char syslog_procid_str[20];
  SOCKADDR_IN syslog_hostaddr;
  SOCKET syslog_socket;
  char local_hostname[MAX_COMPUTERNAME_LENGTH + 1];
  char syslog_hostname[MAX_COMPUTERNAME_LENGTH + 1];
  unsigned short syslog_port;
  int datagramm_size;
  volatile BOOL initialized;
  BOOL wsa_initialized;
} GLOB = {.syslog_opened = FALSE,
          .syslog_mask = 0xFF,
          .syslog_ident = "",
          .syslog_facility = LOG_USER,
          .syslog_procid_str = {0},
          .syslog_hostaddr = {0},
          .syslog_socket = INVALID_SOCKET,
          .local_hostname = {0},
          .syslog_hostname = "localhost",
          .syslog_port = SYSLOG_PORT,
          .datagramm_size = 0,
          .initialized = FALSE,
          .wsa_initialized = FALSE
};

/*
 * ----------------------------------------------------------------------------------
 * public function implementations are bellow
 */
BOOL is_syslog_initialized() { return GLOB.initialized; }

void init_syslog(const char* hostname) {
  WSADATA wsd;
  char* service;

  if (GLOB.initialized) return;

  if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
    // dbj: fail in debug builds
    assert(FALSE && "Can't initialize WinSock 2.2\n");
    // dbj: proceed uninitalized in release builds
    perror("Can't initialize WinSock 2.2\n");
    /* we do not let the rest of the initialization code go through,
       thus none of the syslog calls would succeed. */
    GLOB.wsa_initialized = FALSE;
    return;
  } else {
    GLOB.wsa_initialized = TRUE;
  }
  // DBJ: this is C11
  _Static_assert(sizeof(GLOB.syslog_hostname) > 1, "GLOB.syslog_hostname is too short");

  if (hostname)
    strcpy_s(GLOB.syslog_hostname, sizeof(GLOB.syslog_hostname), hostname);
  else
    strcpy_s(GLOB.syslog_hostname, sizeof(GLOB.syslog_hostname), "Unknown");

  service = strchr(GLOB.syslog_hostname, ':');

  if (service) {
    int tp;

    *service++ = '\0';

    if ((tp = atoi(service)) <= 0) {
      struct servent* se;

      se = getservbyname(service, "udp");

      GLOB.syslog_port = (se == NULL) ? SYSLOG_PORT : se->s_port;
    } else {
      GLOB.syslog_port = (unsigned short)tp;
    }
  } else {
    GLOB.syslog_port = SYSLOG_PORT;
  }

  GLOB.initialized = TRUE;

  // dbj removed this :atexit(exit_syslog);
  // see the clang destructor bellow
}

// as of today 2022 FEB 13 we hide this one here
// users can  reach it only if they know it exist
__attribute__((destructor)) void exit_syslog(void) {
  if (!GLOB.initialized) return;

  closelog();

  if (GLOB.wsa_initialized) WSACleanup();

  GLOB.initialized = FALSE;
}

static void init_logger_addr() {
  struct hostent* phe = NULL;

  memset(&GLOB.syslog_hostaddr, 0, sizeof(SOCKADDR_IN));
  GLOB.syslog_hostaddr.sin_family = AF_INET;

  if (GLOB.syslog_hostname[0] == '\0') goto use_default;

  phe = gethostbyname(GLOB.syslog_hostname);
  if (!phe) goto use_default;

  memcpy(&GLOB.syslog_hostaddr.sin_addr.s_addr, phe->h_addr, phe->h_length);

  GLOB.syslog_hostaddr.sin_port = htons(GLOB.syslog_port);
  return;

use_default:
  GLOB.syslog_hostaddr.sin_addr.S_un.S_addr = htonl(0x7F000001);
  GLOB.syslog_hostaddr.sin_port = htons(SYSLOG_PORT);
}

/******************************************************************************
 * closelog
 *
 * Close desriptor used to write to system logger.
 */
void closelog() {
  if (!GLOB.initialized) return;

  if (GLOB.syslog_opened) {
    closesocket(GLOB.syslog_socket);
    GLOB.syslog_socket = INVALID_SOCKET;
    GLOB.syslog_opened = FALSE;
  }
}

/******************************************************************************
 * openlog: Open connection to system logger.
 * BEWARE: this is called repeatedly thus adjust the logic to that
 */
void openlog(const char* ident, int option, int facility) {
  BOOL failed = FALSE;
  SOCKADDR_IN sa_local = {0};
  int size = 0;

  assert( GLOB.initialized);

  if (!GLOB.syslog_opened) {

        failed = TRUE;
    dbjwin_sprintfa(GLOB.syslog_procid_str, sizeof(GLOB.syslog_procid_str),
                    "[pid:%lu]", GetCurrentProcessId());

        DWORD n = sizeof(GLOB.local_hostname);
    if (!GetComputerNameA(GLOB.local_hostname, &n)) goto done;

    GLOB.syslog_socket = INVALID_SOCKET;

    init_logger_addr();

    for (n = 0;; n++) {
      GLOB.syslog_socket = socket(AF_INET, SOCK_DGRAM, 0);
      if (INVALID_SOCKET == GLOB.syslog_socket) goto done;

      memset(&sa_local, 0, sizeof(SOCKADDR_IN));
      sa_local.sin_family = AF_INET;
      if (bind(GLOB.syslog_socket, (SOCKADDR*)&sa_local, sizeof(SOCKADDR_IN)) ==
          0)
        break;
      closesocket(GLOB.syslog_socket);
      GLOB.syslog_socket = INVALID_SOCKET;
      if (n == 100) goto done;
      Sleep(0);
    }

      /* get size of datagramm */
    size = sizeof(GLOB.datagramm_size);
    if (getsockopt(GLOB.syslog_socket, SOL_SOCKET, SO_MAX_MSG_SIZE,
                   (char*)&GLOB.datagramm_size, &size))
      goto done;
    if (GLOB.datagramm_size - strlen(GLOB.local_hostname) -
            (ident ? strlen(ident) : 0) <
        64)
      goto done;
    if (GLOB.datagramm_size > SYSLOG_DGRAM_SIZE)
      GLOB.datagramm_size = SYSLOG_DGRAM_SIZE;

    if (ident) strcpy_s(GLOB.syslog_ident, sizeof(GLOB.syslog_ident), ident);

    failed = FALSE;

  } // eof not initialized

  if (GLOB.syslog_opened) goto done;


  GLOB.syslog_facility = facility ? facility : LOG_USER;

done:
  if (failed) {
    if (GLOB.syslog_socket != INVALID_SOCKET) closesocket(GLOB.syslog_socket);
  }
  GLOB.syslog_opened = !failed;
}

/******************************************************************************
 * setlogmask
 *
 * Set the log mask level.
 */
int setlogmask(int mask) {
  int ret;

  if (!GLOB.initialized) {
    assert(0 && "Warning: dbj syslog not GLOB.initialized?");
    perror("Warning: dbj syslog not GLOB.initialized?");
    return 0;
  }

  ret = GLOB.syslog_mask;
  if (mask) GLOB.syslog_mask = mask;

  return ret;
}

void vsyslog(int pri, const char* fmt, va_list ap);
/******************************************************************************
 * syslog
 *
 * Generate a log message using FMT string and option arguments.
 */
void syslog(int pri, const char* fmt, ...) {
  if (!GLOB.initialized) /* dbj added */
  {
    assert(0 && "Warning: dbj syslog not GLOB.initialized?");
    perror("Warning: dbj syslog not GLOB.initialized?");
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
// va_start(ap, fmt);
// va_end(ap);
// make sure that is done in the caller
void vsyslog(int pri, const char* fmt, va_list ap) {
  if (!GLOB.initialized) /* dbj added */
  {
    assert(0 && "ERROR: dbj syslog not GLOB.initialized?");
    perror("Warning: dbj syslog not GLOB.initialized?");
    return;
  }

  // Caution! the message must be smaller than SYSLOG_DGRAM_SIZE
  char message_[SYSLOG_DGRAM_SIZE] = {0};
  if (S_OK != dbjwin_vsprintfa(message_, sizeof(message_), fmt, ap)) {
    assert(0 && __FILE__ ": dbjwin_vsprintfa() failed?");
    exit(EXIT_FAILURE);
  }

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

  assert(GLOB.initialized);
  if (!GLOB.initialized) return;

  if (!(LOG_MASK(LOG_PRI(priority_)) & GLOB.syslog_mask)) goto done;

  // reopen for different priority_ & LOG_FACMASK
  openlog(NULL, 0, priority_ & LOG_FACMASK);

  assert(GLOB.syslog_opened);
  if (!GLOB.syslog_opened) goto done;

  if (!(priority_ & LOG_FACMASK)) priority_ |= GLOB.syslog_facility;

#ifdef SYSLOG_RFC3164
  // NOTE: time stamp format is IMPORTANT!
  HRESULT len = dbjwin_sprintfa(datagramm, sizeof(datagramm),
                                "<%d>%s %s %s %s: %s", priority_,
                                dbj_syslog_time_stamp_rfc3164(), GLOB.local_hostname,
                                GLOB.syslog_procid_str, GLOB.syslog_ident, message_);
#elif defined(SYSLOG_RFC5424)
  HRESULT len = dbjwin_sprintfa(
      datagramm, sizeof(datagramm), "<%d>1 %s %s %s %s - - %s", priority_,
      dbj_rfc3399(), GLOB.local_hostname, GLOB.syslog_procid_str, GLOB.syslog_ident, message_);
#else
#error SYSLOG_RFC3164 or SYSLOG_RFC5424 have to be defined
#endif

  if (len != S_OK) {
    assert(0 && __FILE__ ": dbjwin_sprintfa() failed");
    goto done;
  }

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

  int retcode = sendto(GLOB.syslog_socket, datagramm, (int)strlen(datagramm), 0,
                       (SOCKADDR*)&GLOB.syslog_hostaddr, sizeof(SOCKADDR_IN));

  if (WSAEACCESS == WSAGetLastError()) {
    assert(0 && "Winsock permission problem.");
    closesocket(GLOB.syslog_socket);
    WSACleanup();
    return;
  }

  if (retcode == SOCKET_ERROR) {
    assert(0 && __FILE__ ": sendto failed with error  SOCKET_ERROR ");
    closesocket(GLOB.syslog_socket);
    WSACleanup();
    return;
  }

done:
  __noop;
}
