#ifndef _H_DIASM8086_
#define _H_DIASM8086_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

typedef enum Data_Type {
	Mem,
	Imm,
	Explicit_Imm,
} Data_Type;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define CONCAT_BYTES(high, low) ((u16)(high) << 8) + (u8)(low)
#define PRINT_BITS(x)													\
	do {																\
		for (int i = (sizeof (x)) * 8; i >= 0; i--) printf("%d", (x >> i) & 1); \
		printf("\n");													\
	} while (0)

#define FLAG(buf, n) buf & (0x1 << (n - 1))
#define FIELD(buf, left_offset, size) (buf >> (left_offset)) & (u8)(0xFFFF >> (4*4 - size))

#define IF_EXIT(cond, fmt, ...)						\
	do {											\
		if (cond) {									\
			fprintf(stderr, fmt, ## __VA_ARGS__);	\
			exit(EXIT_FAILURE);						\
		}											\
	} while (0)										
		

#if 0
#define HERE() printf("; %s()\n", __func__)
#else
#define HERE()
#endif

#define UNIMPLEMENTED() \
	do { printf("%s:%d: Unimplmented: in %s()\n", __FILE__, __LINE__, __func__); abort(); } while(0)


#endif // _H_DIASM8086_
