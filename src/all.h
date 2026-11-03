#pragma once

#include <assert.h>

#include "../lib/typedefs/types.h"

#define MB * 1048576
#define KB * 1024
#define PAGES * 4096

typedef struct
{
    i32 len;
    const char *data;
} ConstString;

typedef struct Item
{
    struct Item *parent, *next_sibling, *prev_sibling, *first_child;
    ConstString data;
} Item;

#include "templates/item-pool.h"

void display_init(void);
void display_string(ConstString *str, i32 line_no);
void display_destroy(void);

void items_init(void);
