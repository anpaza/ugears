/* C-like private definitions */

#ifndef _CLIKE_DEFS_H
#define _CLIKE_DEFS_H

#if !defined (USE_LIBC) && !defined (USE_CLIKE)
#  define USE_CLIKE     1
#endif

#if defined (USE_LIBC) && defined (USE_CLIKE)
#  error "LIBC and CLIKE cannot be used at the same time"
#endif

#if defined USE_LIBC
// Use CLIKE_P macro to declare all clike function prototypes
#  define CLIKE_P(n)    _##n
#elif defined USE_CLIKE
#  define CLIKE_P(n)    n
#endif

#endif // _CLIKE_DEFS_H
