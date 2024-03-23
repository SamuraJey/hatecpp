#pragma once
#include <stddef.h>
struct BlockHeader {
    size_t size;
    union {
        struct
        {
            BlockHeader *prev, *next;
        };
        char data[1];
    };
};