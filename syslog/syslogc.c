/*
	DBJ Changes : 
	-- always show the process ID, asked or not
	-- if identity (aka tag) is not given it will be called: "Anonymous"
*/
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
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define SYSLOG_NAMES
#include "syslog.h"

// dbj added
static const char* month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static BOOL        syslog_opened = FALSE;

static int         syslog_mask = 0xFF;
static char        syslog_ident[ 128 ] = "";
static int         syslog_facility = LOG_USER;
static char        syslog_procid_str[ 20 ];

static SOCKADDR_IN syslog_hostaddr;
static SOCKET      syslog_socket = INVALID_SOCKET;
static char        local_hostname[ MAX_COMPUTERNAME_LENGTH + 1 ];

static char        syslog_hostname[ MAX_COMPUTERNAME_LENGTH + 1 ] = "localhost";
static unsigned short syslog_port = SYSLOG_PORT;

static int   datagramm_size;

volatile BOOL initialized = FALSE;
BOOL wsa_initialized = FALSE;
CRITICAL_SECTION cs_syslog;

/* DBJ added */
BOOL is_syslog_initialized() {
	return initialized;
}

void init_syslog(const char * hostname)
{
    WSADATA wsd;
    char * service;

    if ( initialized )
        return;

    if( WSAStartup( MAKEWORD( 2, 2 ), &wsd ) ) {
        fprintf(stderr, "Can't initialize WinSock\n");
        /* we let the rest of the initialization code go through,
           although none of the syslog calls would succeed. */
    } else {
        wsa_initialized = TRUE;
    }

    if (hostname)
        strcpy_s(syslog_hostname, sizeof(syslog_hostname), hostname);
    else
        strcpy_s(syslog_hostname, sizeof(syslog_hostname), "");

    service = strchr(syslog_hostname, ':');

    if (service) {
        int tp;

        *service++ = '\0';

        if ((tp = atoi(service)) <= 0) {
            struct servent * se;

            se = getservbyname(service, "udp");

            syslog_port = (se == NULL)? SYSLOG_PORT: se->s_port;
        } else {
            syslog_port = (unsigned short) tp;
        }
    } else {
        syslog_port = SYSLOG_PORT;
    }

    InitializeCriticalSection(&cs_syslog);
    initialized = TRUE;

    atexit(exit_syslog);
}

void exit_syslog(void)
{
    if ( !initialized )
        return;

    closelog();

    if ( wsa_initialized )
        WSACleanup();

    DeleteCriticalSection(&cs_syslog);
    initialized = FALSE;
}

static void init_logger_addr()
{
    struct hostent * phe = NULL;

    memset( &syslog_hostaddr, 0, sizeof(SOCKADDR_IN) );
    syslog_hostaddr.sin_family = AF_INET;

    if (syslog_hostname[0] == '\0')
        goto use_default;

    phe = gethostbyname( syslog_hostname );
    if( !phe )
        goto use_default;

    memcpy( &syslog_hostaddr.sin_addr.s_addr, phe->h_addr, phe->h_length );

    syslog_hostaddr.sin_port = htons( syslog_port );
    return;

use_default:
    syslog_hostaddr.sin_addr.S_un.S_addr = htonl( 0x7F000001 );
    syslog_hostaddr.sin_port = htons( SYSLOG_PORT );
}

/******************************************************************************
 * closelog
 *
 * Close desriptor used to write to system logger.
 */
void closelog()
{
    if ( !initialized )
        return;

    EnterCriticalSection(&cs_syslog);
    if( syslog_opened ) {
        closesocket( syslog_socket );
        syslog_socket = INVALID_SOCKET;
        syslog_opened = FALSE;
    }
    LeaveCriticalSection(&cs_syslog);
}

/******************************************************************************
 * openlog
 *
 * Open connection to system logger.
 */
void openlog(const char* ident, int option, int facility )
{
    BOOL failed = FALSE;
    SOCKADDR_IN sa_local;
    DWORD n;
    int size;

	/*DBJ added */
	if (!ident)	ident = "Anonymous";

    if (!initialized)
    {
        perror("Warning: dbj syslog not initialized?");
        return;
    }

    EnterCriticalSection(&cs_syslog);

    if( syslog_opened )
        goto done;

    failed = TRUE;

    syslog_facility = facility? facility : LOG_USER;

	/* 
	DBJ Changed: always show the process ID, asked or not
	*/
    /*if( option & LOG_PID )*/
        sprintf_s( syslog_procid_str, sizeof(syslog_procid_str), "[pid:%lu]", GetCurrentProcessId() );
    /*else
        syslog_procid_str[0] = '\0'; */

    n = sizeof(local_hostname);
    if( ! GetComputerNameA( local_hostname, &n ) )
        goto done;

    syslog_socket = INVALID_SOCKET;

    init_logger_addr();

    for( n = 0;; n++ )
    {
        syslog_socket = socket( AF_INET, SOCK_DGRAM, 0 );
        if( INVALID_SOCKET == syslog_socket )
            goto done;

        memset( &sa_local, 0, sizeof(SOCKADDR_IN) );
        sa_local.sin_family = AF_INET;
        if( bind( syslog_socket, (SOCKADDR*) &sa_local, sizeof(SOCKADDR_IN) ) == 0 )
            break;
        closesocket( syslog_socket );
        syslog_socket = INVALID_SOCKET;
        if( n == 100 )
            goto done;
        Sleep(0);
    }

    /* get size of datagramm */
    size = sizeof(datagramm_size);
    if( getsockopt( syslog_socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*) &datagramm_size, &size ) )
        goto done;
    if( datagramm_size - strlen(local_hostname) - (ident? strlen(ident) : 0) < 64 )
        goto done;
    if( datagramm_size > SYSLOG_DGRAM_SIZE )
        datagramm_size = SYSLOG_DGRAM_SIZE;

    if (ident)
        strcpy_s(syslog_ident, sizeof(syslog_ident), ident);


    syslog_facility = (facility ? facility : LOG_USER);
    failed = FALSE;

 done:
    if( failed ) {
        if( syslog_socket != INVALID_SOCKET )
            closesocket( syslog_socket );
    }
    syslog_opened = !failed;

    LeaveCriticalSection(&cs_syslog);
}

/******************************************************************************
 * setlogmask
 *
 * Set the log mask level.
 */
int setlogmask( int mask )
{
    int ret;

    if (!initialized)
    {
        perror("Warning: dbj syslog not initialized?");
        return 0;
    }

    EnterCriticalSection(&cs_syslog);

    ret = syslog_mask;
    if( mask )
        syslog_mask = mask;

    LeaveCriticalSection(&cs_syslog);

    return ret;
}

/******************************************************************************
 * syslog
 *
 * Generate a log message using FMT string and option arguments.
 */
void syslog( int pri, const char *fmt, ... )
{
    if (!initialized) /* dbj added */
    {
        perror("Warning: dbj syslog not initialized?");
        return;
    }

	// Caution! the message must be smaller than SYSLOG_DGRAM_SIZE
	char  message_[SYSLOG_DGRAM_SIZE] = {0};

	va_list ap;
	va_start( ap, fmt );
    vsyslog( pri, fmt, ap );
	va_end(ap);
}

void syslog_send(int pri, const char* message_);

void vsyslog(int pri, const char* fmt, va_list ap)
{
    if (!initialized) /* dbj added */
    {
        perror("Warning: dbj syslog not initialized?");
        return;
    }

    // Caution! the message must be smaller than SYSLOG_DGRAM_SIZE
    char  message_[SYSLOG_DGRAM_SIZE] = { 0 };

    // va_start(ap, fmt);
    // or -- https://linux.die.net/man/3/vsnprintf
    if ( vsprintf_s(message_, sizeof(message_), fmt, ap) < 0 )
    {
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
static void syslog_send( int pri, const char * message_ )
{

    char  datagramm[ SYSLOG_DGRAM_SIZE ];
    SYSTEMTIME stm;
    int len;
    char *p;

    if ( !initialized )
        return;

    EnterCriticalSection(&cs_syslog);

    if( !(LOG_MASK( LOG_PRI( pri )) & syslog_mask) )
        goto done;

    openlog( NULL, 0, pri & LOG_FACMASK );
    if( !syslog_opened )
        goto done;

    if( !(pri & LOG_FACMASK) )
        pri |= syslog_facility;

    GetLocalTime( &stm );

/* THIS IS https://tools.ietf.org/html/rfc3164 FORMAT */
	len = sprintf_s( datagramm, sizeof(datagramm),
                     "<%d>%s %2d %02d:%02d:%02d %s %s %s: %s",
                     pri,
                     month[ stm.wMonth - 1 ], stm.wDay, stm.wHour, stm.wMinute, stm.wSecond,
                     local_hostname, syslog_procid_str, syslog_ident,  message_ );
/* RFC5424 format */
/*
	len = sprintf_s(datagramm, sizeof(datagramm),
		"<%d>1 %4d-%02d-%02dT%02d-%02d-%02d.03%d %s %s %s :",
		pri,
		stm.wYear, stm.wMonth, stm.wDay, stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds,
		local_hostname, syslog_ident, syslog_procid_str);
*/
	/* dbj comment: this does clean them all into spaces */
	while ((p = strchr(datagramm, '\n'))) {
		if (p)
			*p = ' '; /* 0; dbj replaced 0 with space */
	}
	while ((p = strchr(datagramm, '\t'))) {
		if (p)
			*p = ' '; /* 0; dbj replaced 0 with space */
	}
	while ((p = strchr(datagramm, '\r'))) {
		if (p)
			*p = ' '; /* 0; dbj replaced 0 with space */
	}
	while ((p = strchr(datagramm, '\v'))) {
		if (p)
			*p = ' '; /* 0; dbj replaced 0 with space */
	}
	while ((p = strchr(datagramm, '\f'))) {
		if (p)
			*p = ' '; /* 0; dbj replaced 0 with space */
	}

    sendto( syslog_socket, datagramm, (int)strlen(datagramm), 0, (SOCKADDR*) &syslog_hostaddr, sizeof(SOCKADDR_IN) );
 done:
    LeaveCriticalSection(&cs_syslog);
}

