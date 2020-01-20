
#include "dbjsyslog.h"
#include "syslog/syslog.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

/* note: this includes suffix in a result */
static char* basename(const char* full_path)
{
    char* p = (char *)full_path, * pp = 0;
    while ((p = strchr(p + 1, '\\')))
    {
        pp = p;
    }
    return (pp ? pp + 1 : p);
}

static const char* app_base_name() 
{
    assert(__argv);
    assert(__argv[0]);

       return basename(__argv[0]) ;
}

/* to initialize in this context means
   call init_syslog( ip_and_port ) and then

   openlog( id, option, facility );

   if id == NULL it will be "Anonymous"
   default facility is LOG_USER
   option is ignored, LOG_ODELAY is always used
   see the option flags for opening in syslog.h
   for details
*/

extern void   dbj_syslog_initalize(const char*  ip_and_port  , const char* id )
{
    // if ip_and_port == NULL, localhost is used
    init_syslog(ip_and_port);
    // if id == NULL then id = "Anonymous" which is not good
    if ( id == NULL )
        openlog(app_base_name(), LOG_ODELAY, LOG_USER);
    else
        openlog(id, LOG_ODELAY, LOG_USER);
}

static void   syslog_call(int level_, const char* format_, va_list ap)
{
    if (!is_syslog_initialized()) {
        dbj_syslog_initalize( NULL, NULL);
    }
        vsyslog(level_, format_, ap);
}
/*
to uninitialize means to call closelog() and do some othe cleanup
exit_syslog() does that on exit so no need for users to do anything
*/

extern void  syslog_emergency(const char* format_, ...)
{
    va_list ap;
    va_start(ap, format_);
        syslog_call( LOG_EMERG, format_, ap);
    va_end(ap);
}
extern void  syslog_alert(const char* format_, ...)
{
    va_list ap;
    va_start(ap, format_);
        syslog_call(LOG_ALERT , format_, ap);
    va_end(ap);
}
extern void  syslog_critical(const char* format_, ...)
{
    va_list ap;
    va_start(ap, format_);
        syslog_call(LOG_CRIT, format_, ap);
    va_end(ap);
}
extern void  syslog_error(const char* format_, ...)
{
    va_list ap;
    va_start(ap, format_);
        syslog_call(LOG_ERR, format_, ap);
    va_end(ap);
}
extern void  syslog_warning(const char* format_, ...)
{
    va_list ap;
    va_start(ap, format_);
        syslog_call(LOG_WARNING, format_, ap);
    va_end(ap);
}
extern void  syslog_notice(const char* format_, ...)
{
    va_list ap;
    va_start(ap, format_);
        syslog_call(LOG_NOTICE, format_, ap);
    va_end(ap);
}
extern void  syslog_info(const char* format_, ...)
{
    va_list ap;
    va_start(ap, format_);
        syslog_call(LOG_INFO, format_, ap);
    va_end(ap);
}
extern void  syslog_debug(const char* format_, ...)
{
    va_list ap;
    va_start(ap, format_);
    syslog_call(LOG_DEBUG , format_, ap);
    va_end(ap);
}
