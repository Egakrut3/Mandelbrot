#ifndef COMMON_DECLARATIONS
#define COMMON_DECLARATIONS

#include <iso646.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
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

#define PRINT_LINE()											\
do {															\
	fprintf(stderr, __FILE__ ":%d: %s\n", __LINE__, __func__);	\
} while (false);

#define CHECK_FUNC(func, ...)											\
do {																	\
	int __cur_err_val = func(__VA_ARGS__);								\
	if (__cur_err_val) {												\
		ON_DEBUG(														\
		fprintf(stderr, "Error with code %d found: ", __cur_err_val);	\
		PRINT_LINE();													\
		perror(#func " failed");										\
		)																\
		CLEAR_RESOURCES();												\
		return __cur_err_val;											\
	}																	\
} while (false)

typedef unsigned char byte_t;

#define min(a, b) a < b ? a : b
#define max(a, b) a > b ? a : b

#endif