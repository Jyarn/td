#pragma once

#include <assert.h>

#include "../lib/typedefs/types.h"

typedef struct
{
    i32 len;
    const char data[];
} ConstString;

void display_init(void);
void display_string(const char *str, i32 line_no);
void display_destroy(void);
