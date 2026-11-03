#include "all.h"
#include <stdbool.h>

u8 buffer[256 MB];
static Item *item_head, *item_root;

#define ITEM_POOL ((ItemPool *) buffer)

void
items_init(void)
{
    ItemPool *new_pool = item_pool_manage(buffer, sizeof(buffer));
    assert((void *) new_pool == buffer);
    item_head = item_root = NULL;
}

static void
delete_children(Item *item)
{
    if (!item) return;

    assert(!item->first_child->prev_sibling);

    Item *i = item->first_child;
    while (i)
    {
        delete_children(i->first_child);
        Item *t = i;
        i = i->next_sibling;

        item_pool_free(ITEM_POOL, t);
    }
}

void
items_delete(void)
{
    delete_children(item_head->first_child);

    //
    // Before:
    //  before <----> item_head <----> after
    // After:
    //  before <----> after
    //
    Item *before, *after;
    before = item_head->prev_sibling;
    after = item_head->next_sibling;

    if (before)
        before->next_sibling = after;

    if (after)
        after->prev_sibling = before;

    assert(before->next_sibling->prev_sibling == before);
    assert(after->next_sibling->prev_sibling == after);
}

void
items_insert_new_sibling(ConstString new_item_data)
{
    Item *new_item = item_pool_alloc(ITEM_POOL);
    if (item_head == NULL)
    {
        // Initializing tree
        assert(!item_root);
        item_head = item_root = new_item;
        *new_item = (Item)
        {
            .next_sibling = NULL,
            .parent = NULL,
            .prev_sibling = NULL,
            .first_child = NULL,
            .data = new_item_data
        };
    }
    else
    {
        assert(item_head != NULL);

        // Initializing new as a child of some item
        //
        // Before:
        //  before <---> after
        // After:
        //  before <----> new <----> after
        //
        Item *before, *after;
        before = item_head;
        after = item_head->next_sibling;

        item_head->prev_sibling = before;
        before->next_sibling = item_head;
        assert(before->next_sibling->prev_sibling == before);
        assert(item_head->prev_sibling->next_sibling == item_head);

        if (after != NULL)
        {
            item_head->next_sibling = after;
            after->prev_sibling = item_head;
            assert(item_head->next_sibling->prev_sibling == item_head);
            assert(after->prev_sibling->next_sibling == after);
        }
        else
        {
            item_head->next_sibling = NULL;
        }
    }
}

bool
items_shift_item_upwards(void)
{
    if (item_head->prev_sibling)
    {
        ConstString swap_var = item_head->data;
        item_head->data = item_head->prev_sibling->data;
        item_head->prev_sibling->data = swap_var;
        return true;
    }
    return false;
}

bool
items_shift_item_downwards(void)
{
    if (item_head->next_sibling)
    {
        ConstString swap_var = item_head->data;
        item_head->data = item_head->next_sibling->data;
        item_head->next_sibling->data = swap_var;
        return true;
    }
    return false;
}

bool
items_traverse_to_first_child(void)
{
    if (item_head->first_child != NULL)
    {
        item_head = item_head->first_child;
        return true;
    }
    return false;
}

bool
items_traverse_to_prev_sibling(void)
{
    if (item_head->prev_sibling != NULL)
    {
        item_head = item_head->prev_sibling;
        return true;
    }
    return false;
}

bool
items_traverse_to_next_sibling(void)
{
    if (item_head->next_sibling != NULL)
    {
        item_head = item_head->next_sibling;
        return true;
    }
    return false;
}

bool
items_traverse_to_parent(void)
{
    if (item_head->parent != NULL)
    {
        item_head = item_head->parent;
        return true;
    }
    return false;
}
