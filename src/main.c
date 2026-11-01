#include "all.h"

#include <unistd.h>

int
main(void)
{
    display_init();
    display_string("hewowowo", 5);
    display_string("HEWOWO", 5);
    display_string("PHWINCESSS!!!", 6);
    sleep(5);
    display_destroy();
}
