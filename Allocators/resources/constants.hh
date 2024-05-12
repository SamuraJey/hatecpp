#ifndef CONSTANTS_H
#define CONSTANTS_H

#define POOL_BUFFER_SIZE (size_t)1024
#define LINKED_BUFFER_SIZE (size_t)1024 * 1024 * 3
#define BUDDY_MAX_LEVEL 22  // 2^22 = 1024 * 1024 * 4
#define DEBUG false

// debug code wrapper macro

#if DEBUG
#define DBG(code) code
#else
#define DBG(code)
#endif

#endif  // CONSTANTS_H
