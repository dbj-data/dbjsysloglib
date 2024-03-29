#ifndef _SYS_SYSLOG_H
#define _SYS_SYSLOG_H

/*
 * Copyright (c) 1982, 1986, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)syslog.h	8.1 (Berkeley) 6/2/93
 */



#if defined(_WIN32) || defined(_WIN64)
#pragma comment(lib, "ws2_32.lib")
#else
#error This is WIN only
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// dbj: remove non ascii parts from the message part of the datagram
#define DBJ_SYSLOG_CLEAN_MSG

// dbj:
// rfc5424 is a bitch #define SYSLOG_RFC5424
// currently we do not support it
// everything undesrstands RFC3164
#define SYSLOG_RFC3164

/* syslog default is 64KB */
#define SYSLOG_DGRAM_SIZE (1024 * 4)
/*
 * priorities/facilities are encoded into a single 32-bit quantity, where the
 * bottom 3 bits are the priority (0-7) and the top 28 bits are the facility
 * (0-big number).  Both the priorities and the facilities map roughly
 * one-to-one to strings in the syslogd(8) source code.  This mapping is
 * included in this file.
 *
 * priorities (these are ordered)
 */
#define LOG_EMERG 0   /* system is unusable */
#define LOG_ALERT 1   /* action must be taken immediately */
#define LOG_CRIT 2    /* critical conditions */
#define LOG_ERR 3     /* error conditions */
#define LOG_WARNING 4 /* warning conditions */
#define LOG_NOTICE 5  /* normal but significant condition */
#define LOG_INFO 6    /* informational */
#define LOG_DEBUG 7   /* debug-level messages */

#define LOG_PRIMASK 0x07 /* mask to extract priority part (internal) */
                         /* extract priority */
#define LOG_PRI(p) ((p)&LOG_PRIMASK)
#define LOG_MAKEPRI(fac, pri) (((fac) << 3) | (pri))

#ifdef SYSLOG_NAMES
#define INTERNAL_NOPRI 0x10 /* the "no priority" priority */
                            /* mark "facility" */
#define INTERNAL_MARK LOG_MAKEPRI(LOG_NFACILITIES, 0)
typedef struct _code {
  char const *c_name; /* dbj added const */
  int c_val;
} CODE;

CODE prioritynames[] = {
    {"alert", LOG_ALERT},     {"crit", LOG_CRIT},
    {"debug", LOG_DEBUG},     {"emerg", LOG_EMERG},
    {"err", LOG_ERR},         {"error", LOG_ERR},       /* DEPRECATED */
    {"info", LOG_INFO},       {"none", INTERNAL_NOPRI}, /* INTERNAL */
    {"notice", LOG_NOTICE},   {"panic", LOG_EMERG},     /* DEPRECATED */
    {"warn", LOG_WARNING},                              /* DEPRECATED */
    {"warning", LOG_WARNING}, {NULL, -1}};
#endif

/* facility codes */
#define LOG_KERN (0 << 3)      /* kernel messages */
#define LOG_USER (1 << 3)      /* random user-level messages */
#define LOG_MAIL (2 << 3)      /* mail system */
#define LOG_DAEMON (3 << 3)    /* system daemons */
#define LOG_AUTH (4 << 3)      /* security/authorization messages */
#define LOG_SYSLOG (5 << 3)    /* messages generated internally by syslogd */
#define LOG_LPR (6 << 3)       /* line printer subsystem */
#define LOG_NEWS (7 << 3)      /* network news subsystem */
#define LOG_UUCP (8 << 3)      /* UUCP subsystem */
#define LOG_CRON (9 << 3)      /* clock daemon */
#define LOG_AUTHPRIV (10 << 3) /* security/authorization messages (private) */
#define LOG_FTP (11 << 3)      /* ftp daemon */

/* other codes through 15 reserved for system use */
#define LOG_LOCAL0 (16 << 3) /* reserved for local use */
#define LOG_LOCAL1 (17 << 3) /* reserved for local use */
#define LOG_LOCAL2 (18 << 3) /* reserved for local use */
#define LOG_LOCAL3 (19 << 3) /* reserved for local use */
#define LOG_LOCAL4 (20 << 3) /* reserved for local use */
#define LOG_LOCAL5 (21 << 3) /* reserved for local use */
#define LOG_LOCAL6 (22 << 3) /* reserved for local use */
#define LOG_LOCAL7 (23 << 3) /* reserved for local use */

#define LOG_NFACILITIES 24 /* current number of facilities */
#define LOG_FACMASK 0x03f8 /* mask to extract facility part */
                           /* facility of pri */
#define LOG_FAC(p) (((p)&LOG_FACMASK) >> 3)

#ifdef SYSLOG_NAMES
CODE facilitynames[] = {
    {"auth", LOG_AUTH},      {"authpriv", LOG_AUTHPRIV},
    {"cron", LOG_CRON},      {"daemon", LOG_DAEMON},
    {"ftp", LOG_FTP},        {"kern", LOG_KERN},
    {"lpr", LOG_LPR},        {"mail", LOG_MAIL},
    {"mark", INTERNAL_MARK},                         /* INTERNAL */
    {"news", LOG_NEWS},      {"security", LOG_AUTH}, /* DEPRECATED */
    {"syslog", LOG_SYSLOG},  {"user", LOG_USER},
    {"uucp", LOG_UUCP},      {"local0", LOG_LOCAL0},
    {"local1", LOG_LOCAL1},  {"local2", LOG_LOCAL2},
    {"local3", LOG_LOCAL3},  {"local4", LOG_LOCAL4},
    {"local5", LOG_LOCAL5},  {"local6", LOG_LOCAL6},
    {"local7", LOG_LOCAL7},  {NULL, -1}};
#endif

/*
 * arguments to setlogmask.
 */
#define LOG_MASK(pri) (1 << (pri)) /* mask for one priority */
#define LOG_UPTO(pri)                                    \
  ((1 << ((pri) + 1)) - 1) /* all priorities through pri \
                            */

/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#define LOG_PID 0x01    /* log the pid with each message */
#define LOG_CONS 0x02   /* log on the console if errors in sending */
#define LOG_ODELAY 0x04 /* delay open until first syslog() (default) */
#define LOG_NDELAY 0x08 /* don't delay open */
#define LOG_NOWAIT 0x10 /* don't wait for console forks: DEPRECATED */
// #define	LOG_PERROR	0x20	/* log to stderr as well */

#define SYSLOG_PORT 514

// dbj added
extern int is_syslog_initialized();

/* Close desriptor used to write to system logger.  */
extern void closelog(void);

/* Open connection to system logger.  */
extern void openlog(const char *__ident, int __option, int __facility);

/* Set the log mask level.  */
extern int setlogmask(int __mask);

/* Generate a log message using FMT string and option arguments.  */
extern void syslog(int __pri, const char *__fmt, ...);

// hidden: extern void vsyslog(int /*__pri*/, const char * /* __fmt */,
// va_list);

/*
   init_syslog() *must* be called before calling any of the above
   functions.  exit_syslog() will be also called as destructor by clang.

   However, it is not an error and is encouraged to call
   exit_syslog() before the application exits.

   During operation, the application is free to call exit_syslog()
   followed by init_syslog() to re-initialize the library. i.e. if
   a different syslog host is to be used.

   Initializes the syslog library and sets the syslog host.  The
   hostname parameter is of the form "<hostname>[:<port>]".  The
   <port> may be a numeric port or it may be a name of a service.
   If the <port> is specified using a service name, it will be
   looked up using getservbyname().

   On failure, the hostname and port will be set to "localhost"
   and SYSLOG_PORT respectively. For example:

   localhost:514

*/
extern void init_syslog(const char *hostname);

// as of 2022 FEB 13 we hide this one
// users can ot reach it any more
void exit_syslog(void);

#ifdef __cplusplus
}
#endif

#endif /* syslog.h */
