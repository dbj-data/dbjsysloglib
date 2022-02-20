// Copyright 2022 by dbj@dbj.org
/*
20220213    DBJ     made 0.2.0
*/
#ifndef DBJ_SYSLOG_WALL_OF_MACROS_
#define DBJ_SYSLOG_WALL_OF_MACROS_

#include <ctype.h>
#include <stddef.h>

#define dbj_syslog_MAJOR  0
#define dbj_syslog_MINOR  9
#define dbj_syslog_PATCH  0

#define  dbj_syslog_VERSION  dbj_syslog_STRINGIFY(dbj_syslog_MAJOR) "." dbj_syslog_STRINGIFY(dbj_syslog_MINOR) "." dbj_syslog_STRINGIFY(dbj_syslog_PATCH)

#if 0
#ifdef    dbj_syslog_FEATURE_RTTI
# define  dbj_syslog__cpp_rtti  dbj_syslog_FEATURE_RTTI
#elif defined(__cpp_rtti)
# define  dbj_syslog__cpp_rtti  __cpp_rtti
#elif defined(__GXX_RTTI) || defined (_CPPRTTI)
# define  dbj_syslog__cpp_rtti  1
#else
# define  dbj_syslog__cpp_rtti  0
#endif
#endif // 0

// Stringify:

#define dbj_syslog_STRINGIFY(  x )  dbj_syslog_STRINGIFY_( x )
#define dbj_syslog_STRINGIFY_( x )  #x

// Compiler warning suppression:

#if defined (__clang__)
# pragma clang diagnostic ignored "-Waggregate-return"
# pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-comparison"
#elif defined (__GNUC__)
# pragma GCC   diagnostic ignored "-Waggregate-return"
# pragma GCC   diagnostic push
#endif

// Suppress shadow and unused-value warning for sections:

#if defined (__clang__)
# define dbj_syslog_SUPPRESS_WSHADOW    _Pragma( "clang diagnostic push" ) \
                                  _Pragma( "clang diagnostic ignored \"-Wshadow\"" )
# define dbj_syslog_SUPPRESS_WUNUSED    _Pragma( "clang diagnostic push" ) \
                                  _Pragma( "clang diagnostic ignored \"-Wunused-value\"" )
# define dbj_syslog_RESTORE_WARNINGS    _Pragma( "clang diagnostic pop"  )

#elif defined (__GNUC__)
# define dbj_syslog_SUPPRESS_WSHADOW    _Pragma( "GCC diagnostic push" ) \
                                  _Pragma( "GCC diagnostic ignored \"-Wshadow\"" )
# define dbj_syslog_SUPPRESS_WUNUSED    _Pragma( "GCC diagnostic push" ) \
                                  _Pragma( "GCC diagnostic ignored \"-Wunused-value\"" )
# define dbj_syslog_RESTORE_WARNINGS    _Pragma( "GCC diagnostic pop"  )
#else
# define dbj_syslog_SUPPRESS_WSHADOW    /*empty*/
# define dbj_syslog_SUPPRESS_WUNUSED    /*empty*/
# define dbj_syslog_RESTORE_WARNINGS    /*empty*/
#endif

// C++ language version detection (C++20 is speculative):

#ifndef   dbj_syslog_CPLUSPLUS
# if defined(_MSVC_LANG ) && !defined(__clang__)
#  define dbj_syslog_CPLUSPLUS  (_MSC_VER == 1900 ? 201103L : _MSVC_LANG )
# else
#  define dbj_syslog_CPLUSPLUS  __cplusplus
# endif
#endif

#define dbj_syslog_CPP98_OR_GREATER  ( dbj_syslog_CPLUSPLUS >= 199711L )
#define dbj_syslog_CPP11_OR_GREATER  ( dbj_syslog_CPLUSPLUS >= 201103L )
#define dbj_syslog_CPP14_OR_GREATER  ( dbj_syslog_CPLUSPLUS >= 201402L )
#define dbj_syslog_CPP17_OR_GREATER  ( dbj_syslog_CPLUSPLUS >= 201703L )
#define dbj_syslog_CPP20_OR_GREATER  ( dbj_syslog_CPLUSPLUS >= 202000L )


#endif // DBJ_SYSLOG_WALL_OF_MACROS_
