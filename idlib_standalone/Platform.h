#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <cstdint>
#include <cstring>

using byte = uint8_t;
using dword = uint32_t;

#define ID_INLINE inline
#define MAX_STRING_CHARS 1024
#define BIT( n ) ( 1 << ( n ) )

#define INTSIGNBITNOTSET(i)		((~((const unsigned int)(i))) >> 31)

#define MAX_WORLD_SIZE (128 * 1024)
#define MAX_WORLD_COORD (MAX_WORLD_SIZE / 2)
#define MIN_WORLD_COORD (-MAX_WORLD_SIZE / 2)

#define ON_EPSILON 0.1f

#endif // __PLATFORM_H__
