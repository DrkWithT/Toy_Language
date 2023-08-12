/**
 * @file hashing.c
 * @author Derek Tan
 * @brief Simple hashing function for "dictionaries".
 * @date 2023-07-31
 */

#include "utils/hashing.h"

size_t hash_key(const char *key)
{
    size_t key_len = strlen(key);
    size_t base = 1;
    size_t hash_num = 0;

    for (size_t i = 0; i < key_len; i++)
    {
        hash_num += base * key[i];
        base *= THE_HASHING_PRIME;
    }

    return hash_num;
}
