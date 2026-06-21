#ifndef MINESWEEPER_LIB_MS_BITSET_H
#define MINESWEEPER_LIB_MS_BITSET_H

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Fixed-purpose bitset for tracking which tile indices have been visited
 * during a flood fill. Direct indexing, no hashing, can't fail to insert.
 *
 * Capacity is measured in bits. The underlying storage is
 * ceil(capacity / CHAR_BIT) bytes. Out-of-range indices are caller bugs
 * and are asserted in debug builds.
 */
typedef struct MS_Bitset {
    unsigned char *bits;
    size_t         capacity; /* in bits */
} MS_Bitset;

static inline size_t ms_bitset_byte_count(const size_t bitCapacity) {
    return (bitCapacity + CHAR_BIT - 1) / CHAR_BIT;
}

static inline bool MS_Bitset_create(MS_Bitset *bs, size_t capacity) {
    if (capacity == 0) capacity = 1;
    const size_t bytes = ms_bitset_byte_count(capacity);
    bs->bits = (unsigned char *) calloc(bytes, 1);
    if (!bs->bits) {
        bs->capacity = 0;
        return false;
    }
    bs->capacity = capacity;
    return true;
}

static inline void MS_Bitset_destroy(MS_Bitset *bs) {
    if (!bs) return;
    free(bs->bits);
    bs->bits = NULL;
    bs->capacity = 0;
}

static inline bool MS_Bitset_resize(MS_Bitset *bs, size_t newCapacity) {
    if (newCapacity == 0) newCapacity = 1;
    const size_t oldBytes = ms_bitset_byte_count(bs->capacity);
    const size_t newBytes = ms_bitset_byte_count(newCapacity);
    unsigned char *newBits = (unsigned char *) realloc(bs->bits, newBytes);
    if (!newBits) return false;
    if (newBytes > oldBytes) {
        memset(newBits + oldBytes, 0, newBytes - oldBytes);
    }
    bs->bits = newBits;
    bs->capacity = newCapacity;
    return true;
}

static inline void MS_Bitset_set(const MS_Bitset *bs, const size_t index) {
    assert(index < bs->capacity && "bitset index out of range");
    bs->bits[index / CHAR_BIT] |= (unsigned char) (1u << (index % CHAR_BIT));
}

static inline bool MS_Bitset_contains(const MS_Bitset *bs, const size_t index) {
    assert(index < bs->capacity && "bitset index out of range");
    return (bs->bits[index / CHAR_BIT] >> (index % CHAR_BIT)) & 1u;
}

static inline void MS_Bitset_clear(const MS_Bitset *bs) {
    memset(bs->bits, 0, ms_bitset_byte_count(bs->capacity));
}

#ifdef __cplusplus
}
#endif

#endif