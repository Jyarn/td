#include "linked-list.template.h"

typedef MANGLE_STRUCT(List) List;
typedef MANGLE_STRUCT(Node) Node;
typedef ELEM_TYPE ElemType;

#ifndef BUILD_ENVIRONMENT
    #include "preamble.h"
#endif

List
MANGLE_FUNCTION(init)(void)
{
    return (List){ 0 };
}

static Node *
MANGLE_FUNCTION(get_node_at_index)(List *list, i64 index)
{
    Node *before_node = NULL;
    Node * (*next_elem_in_list)(List*, Node**) = MANGLE_FUNCTION(next);
    while (next_elem_in_list(list, &before_node))
    {
        if (index == 0)
        {
            return before_node;
        }
        index -= 1;
    }
    assert(false && "Index not found.");
}

void
MANGLE_FUNCTION(insert)(List *list, ElemType elem, i64 index)
{
    assert(list);
    assert(index >= 0);

    Node * (*get_node_at_index)(List*, i64) = MANGLE_FUNCTION(get_node_at_index);

    Node *before_node = get_node_at_index(list, index);
    assert(before_node);

    Node *after_node = malloc(sizeof(Node));
    *after_node = *before_node;

    before_node->next = after_node;
    before_node->elem = elem;

    if (after_node->next == NULL)
        list->tail = after_node;
    assert(false && "Index not found.");
}

void
MANGLE_FUNCTION(delete)(List *list, i64 index)
{
    assert(list);

    if (index == 0)
    {
        assert(list->head);
        list->head = list->head->next;
        if (list->head == NULL)
            list->tail = NULL;
    }
    else
    {
        Node * (*get_node_at_index)(List*, i64) = MANGLE_FUNCTION(get_node_at_index);
        Node *before_node = get_node_at_index(list, index - 1);
        assert(before_node);

        Node *delete_node = before_node->next;
        assert(delete_node && "Index out of range. Deleted node does not exist at index!");
        before_node = delete_node->next;

        if (before_node->next == NULL)
            list->tail = before_node;
    }
}

void
MANGLE_FUNCTION(append_end)(List *list, ElemType elem)
{
    assert(list);

    Node *new_node = malloc(sizeof(Node));
    *new_node = (Node)
    {
        .next = NULL,
        .elem = elem
    };

    if (list->tail)
    {
        list->tail->next = new_node;
    }
    else
    {
        assert(list->head == NULL);
        assert(list->tail == NULL);
        list->head = new_node;
    }
    list->tail = new_node;
}

Node *
MANGLE_FUNCTION(next)(List *list, Node **save_ptr)
{
    assert(list);
    assert(save_ptr);

    if (save_ptr == NULL)
        return list->head;

    else
        return (*save_ptr)->next;
}
