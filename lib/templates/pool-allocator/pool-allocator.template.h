#include "../template.h"

#ifndef BUILD_ENVIRONMENT
    #define ELEM_TYPE int
    #define FUNC_PREFIX
    #define STRUCT_PREFIX
    #include "preamble.h"
#endif


#ifndef ELEM_TYPE
    #error "Element type not defined."
#endif

#ifndef FUNC_PREFIX
    #error "Function prefix not defined (needed for name mangling)."
#endif

#ifndef STRUCT_PREFIX
    #error "Struct prefix not defined (needed for name mangling)."
#endif


typedef struct MANGLE_STRUCT(Pool) MANGLE_STRUCT(Pool);

MANGLE_STRUCT(Pool) * MANGLE_FUNCTION(manage)(void *buffer, i64 buffer_len);
ELEM_TYPE * MANGLE_FUNCTION(alloc)(MANGLE_STRUCT(Pool) *pool);
void MANGLE_FUNCTION(free)(MANGLE_STRUCT(Pool) *pool, ELEM_TYPE *ptr);
void MANGLE_FUNCTION(reset)(MANGLE_STRUCT(Pool) *pool);
