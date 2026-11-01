#include "pool-allocator.template.h"

#ifndef BUILD_ENVIRONMENT
    #include "preamble.h"
#endif

typedef ELEM_TYPE ElemType;

typedef union PoolChunk
{
    ElemType as_data;
    union PoolChunk *as_next;
} PoolChunk;

typedef struct MANGLE_STRUCT(Pool)
{
    i64 total_chunks, num_chunks_initialized;
    PoolChunk *free_list_head;
    PoolChunk chunks[];
} MANGLE_STRUCT(Pool);

typedef MANGLE_STRUCT(Pool) Pool;

Pool *
MANGLE_STRUCT(manage)(void *buffer, i64 buffer_len)
{
    assert(buffer);
    assert(buffer_len > sizeof(Pool) && "buffer size not big enough to fit Pool metadata.");

    Pool *new_pool = (Pool *) buffer;
    new_pool->total_chunks = (buffer_len - sizeof(Pool)) / sizeof(i64);
    new_pool->num_chunks_initialized = 0;
    new_pool->free_list_head = NULL;

    return new_pool;
}

ElemType *
MANGLE_FUNCTION(alloc)(Pool *pool)
{
    if (pool->free_list_head)
    {
        PoolChunk *allocd_chunk = pool->free_list_head;
        pool->free_list_head = pool->free_list_head->as_next;
        return &allocd_chunk->as_data;
    }
    else if (pool->num_chunks_initialized < pool->total_chunks)
    {
        PoolChunk *allocd_chunk = &pool->chunks[pool->num_chunks_initialized];
        pool->num_chunks_initialized += 1;
        return &allocd_chunk->as_data;
    }
    else
    {
        assert(false && "Pool ran out of chunks.");
    }
}

void
MANGLE_FUNCTION(free)(Pool *pool, ElemType *ptr)
{
    PoolChunk *chunk_ptr = (PoolChunk *) ptr;

    assert(pool);
    assert(chunk_ptr);

    // check that we actually manage this chunk
    assert(pool->chunks <= chunk_ptr);
    assert((((u8 *) chunk_ptr) - ((u8 *) pool->chunks)) % sizeof(i64) == 0 && "Unaligned pointer passed to pool_free"); // TODO: test if this makes sense
    assert(chunk_ptr - pool->chunks < pool->num_chunks_initialized);

    chunk_ptr->as_next = pool->free_list_head;
    pool->free_list_head = chunk_ptr;
}

void
MANGLE_FUNCTION(reset)(Pool *pool)
{
    assert(pool);
    pool->num_chunks_initialized = 0;
    pool->free_list_head = NULL;
}
