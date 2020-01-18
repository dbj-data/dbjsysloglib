#pragma once
#ifndef DBJ_CPP_SYSLOG_INC
#define DBJ_CPP_SYSLOG_INC

// this is from MSVC STD LIB 
// it actually does not depend on C++20 __cplusplus
// which is yet undefined as of 2020 Q1

#if !defined(DBJ_HAS_CXX17) && !defined(DBJ_HAS_CXX20)

#if defined(_MSVC_LANG)
#define DBJ__STL_LANG _MSVC_LANG
#else
#define DBJ__STL_LANG __cplusplus
#endif

#if DBJ__STL_LANG > 201703L
#define DBJ_HAS_CXX17 1
#define DBJ_HAS_CXX20 1
#elif DBJ__STL_LANG > 201402L
#define DBJ_HAS_CXX17 1
#define DBJ_HAS_CXX20 0
#else // DBJ__STL_LANG <= 201402L
#define DBJ_HAS_CXX17 0
#define DBJ_HAS_CXX20 0
#error This code requires C++17 or better
#endif 

#undef DBJ__STL_LANG
#endif // !defined(DBJ_HAS_CXX17) && !defined(DBJ_HAS_CXX20)

/*
(c) 2019/2020 by dbj.systems, author: dbj@dbj.org

Public header for using dbjsysloglib from C++.
Please see the README.MD for base documentation

NOTE: syslog() is UNIX affair, and is blisfully unaware of wchar_t
*/

#include "dbjsyslog.h"

#include <mutex>

namespace dbj::log
{

namespace detail
{

	struct lock_unlock final
{

	mutable std::mutex mux_;

	lock_unlock() noexcept { mux_.lock(); }
	~lock_unlock() { mux_.unlock(); }
};
} // namespace detail

template <typename... T>
void syslog_emergency(const char *format_, T... args)
{
	detail::lock_unlock padlock;
	dbj::syslog::syslog_emergency( format_, args...);
}

template <typename... T>
void syslog_alert(const char *format_, T... args)
{
	detail::lock_unlock padlock;
	dbj::syslog::syslog_alert(format_, args...);
}

template <typename... T>
void syslog_critical(const char *format_, T... args)
{
	detail::lock_unlock padlock;
	dbj::syslog::syslog_critical( format_, args...);
}

template <typename... T>
void syslog_error(const char *format_, T... args)
{
	detail::lock_unlock padlock;
	dbj::syslog::syslog_error( format_, args...);
}

template <typename... T>
void syslog_warning(const char *format_, T... args)
{
	detail::lock_unlock padlock;
	dbj::syslog::syslog_warning(format_, args...);
}

template <typename... T>
void syslog_notice(const char *format_, T... args)
{
	detail::lock_unlock padlock;
	dbj::syslog::syslog_notice(format_, args...);
}

template <typename... T>
void syslog_info(const char *format_, T... args)
{
	detail::lock_unlock padlock;
	dbj::syslog::syslog_info( format_, args...);
}

template <typename... T>
void syslog_debug(const char *format_, T... args)
{
	detail::lock_unlock padlock;
	dbj::syslog::syslog_debug( format_, args...);
}

} // namespace dbj::log

// macros are bad but still very effective for decoupling
#if defined(DBJ_SYSLOG)

#define DBJ_LOG_ERR(...) ::dbj::log::syslog_error(__VA_ARGS__)
#define DBJ_LOG_CRT(...) ::dbj::log::syslog_critical(__VA_ARGS__)
#define DBJ_LOG_LRT(...) ::dbj::log::syslog_alert(__VA_ARGS__)
#define DBJ_LOG_WRG(...) ::dbj::log::syslog_warning(__VA_ARGS__)
#define DBJ_LOG_MCY(...) ::dbj::log::syslog_emergency(__VA_ARGS__)
#define DBJ_LOG_DBG(...) ::dbj::log::syslog_debug(__VA_ARGS__)
#define DBJ_LOG_INF(...) ::dbj::log::syslog_info(__VA_ARGS__)
#define DBJ_LOG_NTC(...) ::dbj::log::syslog_notice(__VA_ARGS__)
#else
#define DBJ_LOG_ERR __noop
#define DBJ_LOG_CRT __noop
#define DBJ_LOG_LRT __noop
#define DBJ_LOG_WRG __noop
#define DBJ_LOG_MCY __noop
#define DBJ_LOG_DBG __noop
#define DBJ_LOG_INF __noop
#define DBJ_LOG_NTC __noop
#endif


#endif // DBJ_CPP_SYSLOG_INC
