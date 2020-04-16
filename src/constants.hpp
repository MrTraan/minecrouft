#pragma once

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef long long s64;
typedef int s32;
typedef short s16;
typedef char s8;

// Integer Multiplication overflow checking
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)(-1))
#endif
#define U16_MAX ((u16)(-1))
#define U32_MAX ((u32)(-1))
#define U64_MAX ((u64)(-1))

#define IS_SIZE_T_MUL_SAFE(a, b) (!(((a) != 0) && ((SIZE_MAX / (a)) < (b))))
#define IS_U16_MUL_SAFE(a, b) (!(((a) != 0) && ((U16_MAX / (a)) < (b))))
#define IS_U32_MUL_SAFE(a, b) (!(((a) != 0) && ((U32_MAX / (a)) < (b))))
#define IS_U64_MUL_SAFE(a, b) (!(((a) != 0) && ((U64_MAX / (a)) < (b))))

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 255;
