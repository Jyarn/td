#include "types.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/*
 * Open Addressing Dictionary. Gurantees that each unique key gets mapped to a single bucket.
 * Because of this deletes are technically supported, in the sense that buckets are marked as
 * deleted and can never be used again (ie. deleting a bucket means that no key will be mapped
 * to that bucket).
 *
 * hashmap_set sets the appropriate bucket if the key already exists, otherwise it inserts into
 * the hashmap
 *
 * TODO:
 *  benchmark robinhood hashing
 *  benchmark optimization without mod when looping through keys
 *  optimize hashmap_expand
 */

typedef struct
{
    i64 len;
    const char *data;
} String;

typedef struct
{
    String key;
    String value;
    u64 unmodded_hash_value;
    enum
    {
        HasNeverBeenSet = 0,
        WasSetButNotNow,
        IsSet,
    } status;
} KeyValuePair;

typedef u64 (*HashFunc)(String key);
typedef bool (*KeyEqFunc)(String key1, String key2);

typedef struct
{
    i64 max_buckets, total_filled_buckets;
    HashFunc hash;
    KeyEqFunc key_equals;
    KeyValuePair buckets[];
} HashMap;



HashMap *
hashmap_init(void *backing_buffer, i64 buffer_len, HashFunc hash, KeyEqFunc key_equals)
{
    assert(backing_buffer);
    assert(sizeof(HashMap) < buffer_len);

    HashMap *new_hashmap = backing_buffer;
    new_hashmap->max_buckets =
        (buffer_len - sizeof(HashMap)) / (sizeof(KeyValuePair));
    new_hashmap->total_filled_buckets = 0;

    new_hashmap->hash = hash;
    new_hashmap->key_equals = key_equals;

    memset(
        new_hashmap->buckets,
        0,
        new_hashmap->max_buckets * sizeof(KeyValuePair)
    );

    return new_hashmap;
}

bool
hashmap_set(HashMap *hashmap, String key, String value)
{
#ifndef DONT_ENFORCE_LOAD_FACTOR
    if (3 * hashmap->total_filled_buckets >= 4 * hashmap->max_buckets)
        return false;
#else
    if (hashmap->max_buckets == hashmap->total_filled_buckets)
        return false;
#endif

    u64 hash_value = hashmap->hash(key);

    for (i64 i = 0; i < hashmap->max_buckets; i++)
    {
        u64 bucket_index = (hash_value + i) % hashmap->max_buckets;
        if (i == 0 && hashmap->buckets[bucket_index].status != HasNeverBeenSet)
            printf("collision!\n");

        // if (bucket is not set) or (bucket.key == arg.key)
        if ((hashmap->buckets[bucket_index].status == HasNeverBeenSet) ||
                ((hashmap->buckets[bucket_index].unmodded_hash_value == hash_value) &&
                 (hashmap->key_equals(hashmap->buckets[bucket_index].key, key))))
        {
            hashmap->buckets[bucket_index] = (KeyValuePair)
            {
                .value = value,
                .key = key,
                .unmodded_hash_value = hash_value,
                .status = IsSet,
            };
            hashmap->total_filled_buckets += 1;
            return true;
        }
    }

    assert(false && "hashmap_try_set(): all buckets full despite having space?");
}

bool
hashmap_try_get(HashMap *hashmap, String key, String **value_out)
{
    u64 hash_value = hashmap->hash(key);
    for (i64 i = 0; i < hashmap->max_buckets; i++)
    {
        u64 bucket_index = (hash_value + i) % hashmap->max_buckets;
        if (hashmap->buckets[bucket_index].status == HasNeverBeenSet)
            return false;

        if ((hashmap->buckets[bucket_index].unmodded_hash_value == hash_value) &&
            (hashmap->key_equals(hashmap->buckets[bucket_index].key, key)))
        {
            *value_out = &hashmap->buckets[bucket_index].value;
            return hashmap->buckets[bucket_index].status == IsSet;
        }
    }

    return false;
}

bool
hashmap_contains_key(HashMap *hashmap, String key)
{
    u64 hash_value = hashmap->hash(key);
    for (i64 i = 0; i < hashmap->max_buckets; i++)
    {
        u64 bucket_index = (hash_value + i) % hashmap->max_buckets;
        if (hashmap->buckets[bucket_index].status == HasNeverBeenSet)
            return false;

        if ((hashmap->buckets[bucket_index].unmodded_hash_value == hash_value) &&
            (hashmap->key_equals(hashmap->buckets[bucket_index].key, key)))
        {
            // avoid a branch (yes this is code duplication but why would you want
            // to waste cycles just because you want to both have one function
            // check if the hashmap contains a key and gets a key)
            return hashmap->buckets[bucket_index].status == IsSet;
        }
    }

    return false;
}

void
hashmap_delete(HashMap *hashmap, String key)
{
    u64 hash_value = hashmap->hash(key);
    for (i64 i = 0; i < hashmap->max_buckets; i++)
    {
        u64 bucket_index = (hash_value + i) % hashmap->max_buckets;
        if (hashmap->buckets[bucket_index].status == HasNeverBeenSet)
            return;

        if ((hashmap->buckets[bucket_index].unmodded_hash_value == hash_value) &&
            (hashmap->key_equals(hashmap->buckets[bucket_index].key, key)))
        {
            hashmap->buckets[bucket_index].status = WasSetButNotNow;
        }
    }
}

HashMap *
hashmap_expand(HashMap *old_hashmap, void *new_backing_buffer, i64 new_buffer_len)
{
    HashMap *new_hashmap = hashmap_init(new_backing_buffer, new_buffer_len, old_hashmap->hash, old_hashmap->key_equals);
    assert(new_hashmap->max_buckets > old_hashmap->max_buckets);

    for (i64 i = 0; i < old_hashmap->max_buckets; i++)
    {
        if (old_hashmap->buckets[i].status == IsSet)
        {
            hashmap_set(
                new_hashmap,
                old_hashmap->buckets[i].key,
                old_hashmap->buckets[i].value
            );
        }
    }

    return new_hashmap;
}
