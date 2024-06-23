/// @file common.h
///
/// common.h
///
#ifndef _COMMON_H_
#define _COMMON_H_

#include <wx/platform.h>
//#include "version.h"
#include "debugreport.h"

#if defined(__WXMSW__)

// ignore warning
#ifndef __GNUC__
//#pragma warning(disable:4482)
#pragma warning(disable:4996)
#endif

#else

//#include "tchar.h"
//#include "typedef.h"

#if defined(__WXOSX__)

wchar_t *_wgetenv(const wchar_t *);
int _wsystem(const wchar_t *);

#elif defined(__WXGTK__)

wchar_t *_wgetenv(const wchar_t *);
int _wsystem(const wchar_t *);

#endif
#endif

#define COLOR_DENSITY_MAX	7

#endif /* _COMMON_H_ */
