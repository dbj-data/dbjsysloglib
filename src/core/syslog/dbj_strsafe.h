#pragma once
/*
 (c) 2020 - 2022 by dbj@dbj.org -- LICENSE_DBJ

 "normalization" of "crazy camel" strsafe names

 also the same with "pathcch.h" function, mashed in here too
*/

#include <assert.h>
#define DBJWIN_ASSERT assert

#undef DBJWIN_INLINE
#define DBJWIN_INLINE __inline

#ifdef __cplusplus
#define DBJWIN_EXTERN_C_BEGIN extern "C" {
#define DBJWIN_EXTERN_C_END }
#else
#define DBJWIN_EXTERN_C_BEGIN
#define DBJWIN_EXTERN_C_END
#endif


#ifndef STRICT
#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <PathCch.h>
#include <strsafe.h>
#pragma comment(lib, "Pathcch.lib")

DBJWIN_EXTERN_C_BEGIN

/// ---------------------------------------------------------------------
/// pathcch functions
/// ---------------------------------------------------------------------
/*
returns HRESULT, must use SUCCEEDED or FAILED to check
example:
dbjwin_path_rename_extension( "c:\\readme.txt", sizeof("c:\\readme.txt"), "md"
); this is wide string only, undocumented as such
*/

#undef dbjwin_path_rename_extensionw
#define dbjwin_path_rename_extensionw(OUTBUF_, OUTBUF_SIZE_, NEW_EXTENSION_) \
  PathCchRenameExtension(OUTBUF_, OUTBUF_SIZE_, NEW_EXTENSION_)

/// ---------------------------------------------------------------------
/// strsafe functions
/// ---------------------------------------------------------------------

/*
for some reason strsafe uses its own Win32 typedefs and calls them "Deprecated"
thus we include windows.h
*/

// Used to check if the lenght of the string is smaller or equal to a limit
// if it is, S_OK is returned

#undef dbjwin_strlena
#define dbjwin_strlena StringCchLengthA

/*
------------------------------------------------------------------
*/

#undef dbjwin_strcata
#define dbjwin_strcata(DEST_, DEST_SIZE_, SRC_) \
  StringCchCatA(DEST_, DEST_SIZE_, SRC_)

/*
------------------------------------------------------------------
return 0 on failure
*/
DBJWIN_INLINE size_t dbjwin_strnlena(LPCSTR in_str_, size_t control_size_) {
  size_t rezult_ = 0;
  HRESULT hr_ = dbjwin_strlena(in_str_, control_size_, &rezult_);
#ifdef _DEBUG
  DBJWIN_ASSERT(SUCCEEDED(hr_));
#endif
  if (FAILED(hr_)) return 0;
  return rezult_;
}

/*
------------------------------------------------------------------
*/

#undef dbjwin_strlenw
#define dbjwin_strlenw StringCchLengthW

DBJWIN_INLINE size_t dbjwin_strnlenw(LPCWSTR in_str_, size_t control_size_) {
  size_t rezult_ = 0;
  HRESULT hr_ = dbjwin_strlenw(in_str_, control_size_, &rezult_);
#ifdef _DEBUG
  DBJWIN_ASSERT(SUCCEEDED(hr_));
#endif

  if (FAILED(hr_)) return 0;
  return rezult_;
}

/*
STRSAFEAPI StringCchPrintfA(  STRSAFE_LPSTR  pszDest,  size_t cchDest,
STRSAFE_LPCSTR pszFormat,  ...);
*/
#undef dbjwin_sprintfa
#define dbjwin_sprintfa StringCchPrintfA

#undef dbjwin_sprintfw
#define dbjwin_sprintfw StringCchPrintfW

// https://docs.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcchvprintfa
/*

STRSAFEAPI StringCchVPrintfA(
  STRSAFE_LPSTR  pszDest,
  size_t         cchDest,
  STRSAFE_LPCSTR pszFormat,
  va_list        argList
);

*/
#undef dbjwin_vsprintfa
#define dbjwin_vsprintfa StringCchVPrintfA

#undef dbjwin_vsprintfw
#define dbjwin_vsprintfw StringCchVPrintfW

// STRSAFEAPI StringCchCopyNA([out] STRSAFE_LPSTR pszDest, [in] size_t cchDest,
//                           [in] STRSAFE_PCNZCH pszSrc, [in] size_t cchToCopy);
#undef dbjwin_strncpya
#define dbjwin_strncpya StringCchCopyNA

#undef dbjwin_strncpyw
#define dbjwin_strncpyw StringCchCopyNW

DBJWIN_EXTERN_C_END