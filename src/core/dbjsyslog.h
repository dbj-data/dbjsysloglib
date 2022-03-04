#pragma once
#ifndef DBJ_SYSLOG_LIB_INC
#define DBJ_SYSLOG_LIB_INC

#include "../dbjsyslogclient.h"

/*
(c) 2019/2020/2022 by dbj.systems, author: dbj@dbj.org
private header for the C code using the dbjsysloglib implementation
As of 2022-01-30  this is hidden inside a dbj dll component.

ADVISORY:
When building with clang in release mode switch the /LTCG option off, see :
https://github.com/MicrosoftDocs/cpp-docs/issues/376
*/

#ifdef __cplusplus
namespace dbj::syslog {
 "C" {
#endif
   /*
;;
;; (c) 2022 by dbj at dbj dot org
;;
;; syslog defaults
url = localhost
port = 514
;; reserved unused
;; udp or tcp
protocol = udp
;; in bytes
datagramsize=2048
*/
#define DBJSYSLOG_INI_FILE_NAME "dbjsyslogclient.ini"
#define DBJSYSLOG_LOCALHOST "localhost"
#define DBJSYSLOG_DFLT_PORT 514

	 // config is coming from da ini file
   typedef struct configuration_ {
     char url[0xFF];
     int port;
   } configuration;

	// must iniialize before first use
	// WARNING! if not initialized dbjsysloglob will simply exit
	 int is_syslog_initialized();
	/* to initialize in this context means
	   call init_syslog( ip_and_port = localhost ) 
	   openlog( id, option, facility );

	   if id == NULL it will be "Anonymous"
	   default facility is LOG_USER
	   option is ignored, LOG_ODELAY is always used
	   see the option flags for opening in syslog.h 
	   for details

	   since version 2.0.0 ip and port are coming from da ini file
	   or are default to localhost:514
	*/
         void dbj_syslog_initalize( syslog_client_identity );

	/*
	to uninitialize means to call closelog() and do some othe cleanup
	exit_syslog() does that on exit so no need for users to do anything
	*/
	
	 void  syslog_emergency(const char* format_, ...);
	 void  syslog_alert(const char* format_, ...);
	 void  syslog_critical(const char* format_, ...);
	 void  syslog_error(const char* format_, ...);
	 void  syslog_warning(const char* format_, ...);
	 void  syslog_notice(const char* format_, ...);
	 void  syslog_info(const char* format_, ...);
	 void  syslog_debug(const char* format_, ...);

#ifdef __cplusplus
} //  "C" 
} // namespace dbj::syslog 
#endif

#endif // !DBJ_SYSLOG_LIB_INC

