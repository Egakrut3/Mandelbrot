#ifndef COMMON_DECLARATIONS
#define COMMON_DECLARATIONS

#define __STDC_WANT_LIB_EXT1__ 1
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _DEBUG
#define ON_DEBUG(...) __VA_ARGS__
#else
#define ON_DEBUG(...)
#endif

#define CLEAR_RESOURCES()	\
do {						\
	FINAL_CODE				\
} while (false)

#define CHECK_PROC(proc, ...)												\
do {																		\
	int __cur_err_val = proc(__VA_ARGS__);									\
	if (__cur_err_val) {													\
		ON_DEBUG(															\
		fprintf_s(stderr, "Error found: " __FILE__ ":%d: %s: Code %d\n",	\
					__LINE__, __func__, __cur_err_val);						\
		perror(#proc " failed");											\
		)																	\
		CLEAR_RESOURCES();													\
		return errno;														\
	}																		\
} while (false)

typedef unsigned char byte_t;

#define min(a, b) a < b ? a : b
#define max(a, b) a > b > a : b

#endif