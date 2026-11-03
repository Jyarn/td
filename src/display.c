#include "all.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <string.h>

static i64 window_width;
static i64 window_height;

struct termios stdin_config, stdout_config;


void
display_init(void)
{
    tcgetattr(STDOUT_FILENO, &stdout_config);
    tcgetattr(STDIN_FILENO, &stdin_config);

    printf("\x1b[0m");      // Reset screen
    printf("\x1b[?1049h");  // Switch to alt buffer
    printf("\x1b[H");       // Move cursor to home position (0, 0)
    printf("\x1b[?25H");    // Show cursor
    fflush(stdout);

    // Configure alt buffer to be in raw mode
    struct termios new_term_config;
    cfmakeraw(&new_term_config);
    tcsetattr(STDOUT_FILENO, TCSANOW, &new_term_config);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term_config);

    // Get window size and height
    struct winsize ioctl_winsize;
    assert(!ioctl(STDOUT_FILENO, TIOCGWINSZ, &ioctl_winsize));
    window_width = ioctl_winsize.ws_col;
    window_height = ioctl_winsize.ws_row;
}

void
display_string(ConstString *str, i32 line_no)
{
    assert(line_no >= 0);
    if (line_no >= window_height)
        return;

    printf("\x1b[%d;1H", line_no);
    printf("\x1b[0K");
    printf("%.*s", str->len, str->data);
    printf("\x1b[H");
    fflush(stdout);
}

void
display_destroy(void)
{
    printf("\x1b[?1049l");
    tcsetattr(STDOUT_FILENO, TCSADRAIN, &stdout_config);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &stdin_config);
}
