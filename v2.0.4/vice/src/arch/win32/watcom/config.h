#ifndef _CONFIG_H_
#define _CONFIG_H_

#define UNSTABLE

#pragma warning(disable:4996)

#define HAVE_ALLOCA             1
#define RETSIGTYPE              void
#define HAS_JOYSTICK            1
#define HAVE_MOUSE              1
#define HAVE_HARDSID            1
#define HAVE_RS232              1

#define HAS_LONGLONG_INTEGER    1
#define HAS_UNLOCKRESOURCE      1
#define SIZEOF_UNSIGNED_INT     4
#define SIZEOF_UNSIGNED_LONG    4
#define SIZEOF_UNSIGNED_SHORT   2
#define HAVE_ATEXIT             1
#define HAVE_MEMMOVE            1
#define HAVE_STRERROR           1
#define HAVE_FCNTL_H            1
#define HAVE_LIMITS_H           1
#define HAVE_COMMCTRL_H         1
#define HAVE_SHLOBJ_H           1
#define HAVE_DIRECT_H           1
#define HAVE_ERRNO_H            1
#define HAVE_IO_H               1
#define HAVE_PROCESS_H          1
#define HAVE_SYS_TYPES_H        1
#define HAVE_SYS_STAT_H         1
#define HAVE_SIGNAL_H           1
#define HAVE_WINIOCTL_H         1
#define DWORD_IS_LONG           1
#define HAVE_OPENCBM            1
#define HAVE_FFMPEG             1
#define HAVE_CRTDBG             1
#define HAS_TRANSLATION         1
#define HAVE_HTONL              1
#define HAVE_HTONS              1
#define HAVE_STRNCASECMP        1
#define HAVE_PROCESS_H          1
#define HAVE_GUIDLIB            1
#define HAVE_GETCWD             1

#define __i386__                1

#define inline

#define _ANONYMOUS_UNION

#define MSVC_RC                 1

#define strcasecmp(s1, s2)      _stricmp(s1, s2)
#define HAVE_STRCASECMP         1

#define snprintf _snprintf

#define __u_char unsigned char
#define _P_WAIT   P_WAIT
#define _spawnvp spawnvp
#define _dup2 dup2

/* begin: for FFMPEG: common.h */
#define CONFIG_WIN32
#define _WIN32

#define WATCOM_COMPILE          1
#define _MSC_VER                1

#define int64_t_C(c)     (c ## i64)
#define uint64_t_C(c)    (c ## u64)
/* end: for FFMPEG: common.h */

#endif
