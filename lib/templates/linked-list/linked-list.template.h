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


typedef struct MANGLE_STRUCT(Node)
{
    ELEM_TYPE elem;
    struct MANGLE_STRUCT(Node) *next;
} MANGLE_STRUCT(Node);

typedef struct
{
    MANGLE_STRUCT(Node) *head, *tail;
} MANGLE_STRUCT(List);

MANGLE_STRUCT(List) MANGLE_FUNCTION(init)(void);
void MANGLE_FUNCTION(insert)(MANGLE_STRUCT(List) *list, ELEM_TYPE elem, i64 index);
void MANGLE_FUNCTION(delete)(MANGLE_STRUCT(List) *list, i64 index);
void MANGLE_FUNCTION(append_end)(MANGLE_STRUCT(List) *list, ELEM_TYPE elem);
void MANGLE_FUNCTION(destroy)(MANGLE_STRUCT(List) *list);
MANGLE_STRUCT(Node) * MANGLE_FUNCTION(next)(MANGLE_STRUCT(List) *list, MANGLE_STRUCT(Node) **save_ptr);
