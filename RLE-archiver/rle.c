#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "rle.h"

unsigned char *rle_encode(const unsigned char *src, size_t isize, size_t *osize) {
    if (!src || !osize || !isize)
        return NULL;
    unsigned char *encoded = (unsigned char *)malloc(isize * 2);
    if (!encoded)
        return NULL;
    unsigned char count;
    size_t k = 0;
    for (size_t i = 0; i < isize; i++) {
        count = 1;
        while (i < isize - 1 && src[i] == src[i + 1] && count != 255) {
            ++count;
            ++i;
        }
        encoded[k++] = count;
        encoded[k++] = src[i];
    }
    *osize = k;
    unsigned char *temp = realloc(encoded, k);
    if (!temp)
        return encoded;
    encoded = temp;
    return encoded;
}

unsigned char *rle_decode(const unsigned char *src, size_t isize, size_t *osize) {
    if (!src || !osize || (isize % 2) != 0)
        return NULL;

    size_t total = 0;
    for (size_t i = 0; i < isize; i += 2) {
        unsigned char count = src[i];
        if (!count)
            return NULL;
        total += count;
    }

    unsigned char *decoded = (unsigned char *)malloc(total);
    if (!decoded)
        return NULL;
    
    size_t pos = 0;
    for (size_t i = 0; i < isize; i += 2) {
        unsigned char count = src[i];
        unsigned char byte = src[i + 1];
        while (count--)
            decoded[pos++] = byte;
    }

    *osize = pos;
    return decoded;
}
