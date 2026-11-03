#include "all.h"

#include <unistd.h>

int
main(void)
{
    items_init();
    display_init();
    sleep(5);
    display_destroy();
}
