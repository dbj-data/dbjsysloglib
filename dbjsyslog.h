#pragma once
#ifndef DBJ_SYSLOG_LIB_INC
#define DBJ_SYSLOG_LIB_INC

/*
(c) 2019/2020 by dbj.systems, author: dbj@dbj.org
public header for the C code using the dbjsysloglib
*/
/*
ADVISORY:
When building with clang in release mode switch the /LTCG option off, see :
https://github.com/MicrosoftDocs/cpp-docs/issues/376
*/

#ifdef __cplusplus
namespace dbj::syslog {
extern "C" {
#endif

	// SEMantic VERsioning
	typedef enum version {
	 MAJOR = 1,
	 MINOR = 0,
	 PATCH = 0,
	} version;
	// must iniialize before first use
	// WARNING! if not initialized dbjsysloglob will simply exit
	extern int is_syslog_initialized();
	/* to initialize in this context means
	   call init_syslog( ip_and_port = localhost ) 
	   
	   and then 

	   openlog( id, option, facility );

	   if id == NULL it will be "Anonymous"
	   default facility is LOG_USER
	   option is ignored, LOG_ODELAY is always used
	   see the option flags for opening in syslog.h 
	   for details
	*/
	extern void   dbj_syslog_initalize( const char * /* hostname */, const char * /* id */);

	/*
	to uninitialize means to call closelog() and do some othe cleanup
	exit_syslog() does that on exit so no need for users to do anything
	*/
	
	extern void  syslog_emergency(const char* format_, ...);
	extern void  syslog_alert(const char* format_, ...);
	extern void  syslog_critical(const char* format_, ...);
	extern void  syslog_error(const char* format_, ...);
	extern void  syslog_warning(const char* format_, ...);
	extern void  syslog_notice(const char* format_, ...);
	extern void  syslog_info(const char* format_, ...);
	extern void  syslog_debug(const char* format_, ...);

#ifdef __cplusplus
} // extern "C" 
} // namespace dbj::syslog 
#endif

#endif // !DBJ_SYSLOG_LIB_INC

