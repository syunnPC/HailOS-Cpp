#pragma once

using u8 = unsigned char;
using i8 = signed char;
using u16 = unsigned short;
using i16 = short;
using u32 = unsigned int;
using i32 = int;
using u64 = unsigned long;
using i64 = long;
using addr_t = u64;
using size_t = u64;

#define PACKED __attribute__((packed))

static_assert(sizeof(u16) == 2, "sizeof(u16) == 2 not satisfied");
static_assert(sizeof(u32) == 4, "sizeof(u32) == 4 not satisfied");
static_assert(sizeof(u64) == 8, "sizeof(u64) == 8 not satisfied");

#ifndef CHAR_BIT
#define CHAR_BIT 8
#else
static_assert(CHAR_BIT == 8, "CHAR_BIT must be 8.");
#endif