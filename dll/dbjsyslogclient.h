#pragma once
#ifndef __clang__
#error Obviously, this code requires clang compiler
#endif

#pragma clang system_header

#include <errno.h>

//  make sure build script creates this dll for this component
#define DBJSYSLOGCLIENT_DLL_NAME "dbjsyslogclient.dll"

// the interface
typedef struct dbjsyslog_client_ dbjsyslog_client;
struct dbjsyslog_client_
{
	// WARNING: if not initialized dbjsysloglob will simply exit
	int (*is_syslog_initialized)(void);
	/* 

	   openlog( id, option, facility );

	   if id == NULL it will be "Anonymous"
	   default facility is LOG_USER
	   option is ignored, LOG_ODELAY is always used
	   see the option flags for opening in syslog.h 
	   for details
	
    syslog client must initialize before first use
    we need to do this manually so that we give the identity to the syslog client 
    
    */
	void   (*dbj_syslog_initalize)( const char * /* hostname */, const char * /* id */);

	/*
	to uninitialize means to call closelog() and do some othe cleanup
	exit_syslog() does that on exit so no need for users to do anything
	*/

 	 void  (*emergency)(const char* format_, ...);
	 void  (*alert)(const char* format_, ...);
	 void  (*critical)(const char* format_, ...);
	 void  (*error)(const char* format_, ...);
	 void  (*warning)(const char* format_, ...);
	 void  (*notice)(const char* format_, ...);
	 void  (*info)(const char* format_, ...);
	 void  (*debug)(const char* format_, ...);
};

// ifp == InterFace Factory F.Pointer type
// returns a pointer to the interface implementation 
typedef dbjsyslog_client *(*dbjsyslog_client_ifp)(void);

//
// interface factory function for this component is implemented
// inside the DLL, by name as defined in the def file:
//
// struct dbjsyslog_client_ *interface_factory(void);
//

