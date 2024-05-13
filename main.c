#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define UNUSED __attribute__((unused))

void showtasks(void);

typedef struct {
    int colour;
    char print;
} TagProps;

typedef enum {
    true = 1,
    fals = 0,
} bool;

TagProps tagtable[] = {
    { 0x6082b6, 1 },
    { 0x60b435, 1 },
};


int curtime;
int threshold = 359;

static inline void
print(const char* str)
{
    int l = 0;
    while(*str) {
        l++;
        str++;
    }

    write(STDOUT_FILENO, str-l, l);
}

void
quit(int status UNUSED)
{
    print("\x1b[?25h");
    exit(0);
}

unsigned int
intostr(char* buff, unsigned int in, int level, int* offset)
{
    if (in / level == 0) {
        *offset = 0;
        buff[0] = '0';
        return 0;
    }

    unsigned int r = intostr(buff, in, level*10, offset);
    unsigned int t = (in - r*level*10) / level;
    buff[*offset] = t + '0';
    *offset += 1;
    return r*10 + t;
}

void
printesc(int tag)
{
    char buff[20];
    int off = 0;
    buff[off++] = '\x1b';
    buff[off++] = '[';
    buff[off++] = '3';
    buff[off++] = '8';
    buff[off++] = ';';
    buff[off++] = '2';
    buff[off++] = ';';

    int t = 0;
    intostr(&buff[off], (tagtable[tag].colour >> 16) & 0xff, 1, &t);
    off += t;
    buff[off++] = ';';

    t = 0;
    intostr(&buff[off], (tagtable[tag].colour >> 8) & 0xff, 1, &t);
    off += t;
    buff[off++] = ';';

    t = 0;
    intostr(&buff[off], tagtable[tag].colour & 0xff, 1, &t);
    off += t;
    buff[off++] = 'm';

    assert(off <= 20);
    buff[off] = '\0';
    print(buff);
}

int
timetostr(int tim, char** r)
{
    *r = malloc(256);
    int off = 0;
    int t = 0;

    int tt = tim / 31557600;
    if (tt != 0) {
        intostr(&(*r)[off], tt, 1, &t);
        off += t;
        (*r)[off++] = 'y';
        (*r)[off++] = ';';
        tim -= tt * 31557600;
    }

    tt = tim / 604800;
    if (tt != 0) {
        intostr(&(*r)[off], tt, 1, &t);
        off += t;
        (*r)[off++] = 'w';
        (*r)[off++] = ';';
        tim -= tt * 604800;
    }

    tt = tim / 86400;
    if (tt != 0) {
        intostr(&(*r)[off], tt, 1, &t);
        off += t;
        (*r)[off++] = 'd';
        (*r)[off++] = ';';
        tim -= tt * 86400;
    }

    tt = tim / 3600;
    if (tt != 0) {
        intostr(&(*r)[off], tt, 1, &t);
        off += t;
        (*r)[off++] = 'h';
        (*r)[off++] = ';';
        tim -= tt * 3600;
    }

    tt = tim / 60;
    if (tt != 0) {
        intostr(&(*r)[off], tt, 1, &t);
        off += t;
        (*r)[off++] = 'm';
        (*r)[off++] = ';';
        tim -= tt * 60;
    }

    if (tim != 0) {
        intostr(&(*r)[off], tim, 1, &t);
        off += t;
    } else
        (*r)[off++] = '0';

    (*r)[off++] = 's';
    (*r)[off] = ';';

    assert(off <= 256);
    return off;
}

void
bind(int offset, int tag, const char* str, int tim, int start, bool repeat, bool finished)
{
    if (!tagtable[tag].print || start > curtime)
        return;

    usleep(100000);

    print("\x1b[3m");
    printesc(tag);
    if (!finished)
        print("\x1b[1m");

    char* offstr = malloc(offset);
    for (int i = 0; i < offset; i++) offstr[i] = ' ';

    write(STDOUT_FILENO, offstr, offset);
    free(offstr);

    write(STDOUT_FILENO, " - ", 3);
    if (finished)
        print("\x1b[9m");

    print(str);
    print("\x1b[0m");
    printesc(tag);

    int temp_curtime = curtime - repeat*start;
    if (tim > temp_curtime) {
        write(STDOUT_FILENO, " @ ", 3);
        char* ret;
        int s = timetostr(tim - temp_curtime, &ret);
        write(STDOUT_FILENO, ret, s);
        free(ret);

    } else if (repeat) {
        write(STDOUT_FILENO, " @ ", 3);
        char* ret;
        int ceil = temp_curtime % tim == 0 ? temp_curtime / tim : (temp_curtime/ tim) + 1;
        int s = timetostr(tim*ceil - temp_curtime, &ret);
        write(STDOUT_FILENO, ret, s);
        free(ret);
    }

    write(STDOUT_FILENO, "\x1b[0;m\n", 6);
}

unsigned int
strtoint(const char* str, int* off)
{
    int r = 0;
    while ('0' <= str[*off] && str[*off] <= '9') {
        r = r*10 + *str - '0';
        *off += 1;
    }

    return r;
}

/*
 * str(ing) sub(set)
 */
bool
strsub(const char* c1, const char* c2)
{
    while (*c1 && *c2)
        if (*c2 == ' ')
            c2++;

        else if (*(c1++) != *(c2++))
            return fals;

    return !(*c1);
}

int
findline(unsigned int offset, unsigned int line, const char* buff, int size)
{
    for (int i = 0; i < size; i++) {
        if (buff[i] == '\n') { // find start of a line
            if (strsub("bind", &buff[i+1])) { // check if the line
                                              // starts with "bind"
                int r;

                for (int j = i; j < size; j++) {
                    if (buff[j] == 'b') // store start of line for return
                        r = j;

                    else if ('0' <= buff[j] && buff[j] <= '9') { // find first num
                        int t = 0;
                        if (strtoint(&buff[j], &t) == offset && !line--)
                            return r;
                        else
                            break;
                    }
                }

                continue;
            }
        }
    }

    return -1;
}

inline static int
strlength(const char* s)
{
    int r = 0;
    while (s[r])
        r++;
    return r;
}

inline static int
bufwrite(const char* s, char* b, int sz, int off)
{
    int i;
    for (i = off; i < sz; i++)
        b[i] = s[i];

    return i;
}

int
jumptoline(int argc, char** argv, unsigned int last, const char* buff, unsigned int size)
{
    unsigned int i = last;
    for (int k = 2; k < argc; k++) {
        int t = 0;
        int j = findline(k-2, strtoint(argv[k], &t), &buff[i], size-last);

        if (j == -1)
            return -1;
        else
            i += j;
    }

    return i;
}

void
toggle(int at, int fd, const char* buff, int size)
{
    for (at += 24; at < size; at++) {
        if (buff[at] == ')') {
            lseek(fd, at-4, SEEK_SET);
            if (strsub("fals", &buff[at-4]))
                assert(write(fd, "true", 4) == 4);
            else
                write(fd, "fals", 4);

            break;
        }
    }

}

void
prompt(char* buff, int size, const char* msg)
{
    int readsize = 0;
    bool flush = fals;

    print(msg);

    for (;;) {
        buff[size-1] = '\0';
        if ((readsize = read(STDIN_FILENO, buff, size)) < size || buff[size-1] == '\n') {
            assert(0 <= readsize);

            if (flush) {
                print(msg);
                flush = fals;
                continue;
            }

            buff[readsize-1] = '\0';
            return;
        }

        if (!flush)
            print("error: input too big [friendly tip: make it smaller (: ]\n");

        flush = true;
    }
}

unsigned int
promptnum(char buff[5], const char* msg, unsigned int bound)
{
    for (;;) {
        prompt(buff, 5, msg);

        int t = 0;
        int r = strtoint(buff, &t);
        assert(t <= 5);

        if (!buff[t]) {
            if (r < bound)
                return r;
            else
                print("too big, have you tried making it smaller?\n");

        } else
            print("not a number\n");
    }
}

void
addtask(int at, int fd, const char* buff, int size)
{
    char name[33];
    char offset[2];
    char tag[5];
    int start;
    int tim;
    char repeat;

    for (;;) {
        prompt(offset, 2, "offset? ");
        if (offset[0] == '\0' || ('0' <= offset[0] && offset[0] <= '9'))
            break;
        else
            print("not a number trying entering a number next time\n");
    }

    promptnum(tag, "tag? ", sizeof(tagtable)/sizeof(TagProps));
    prompt(name, 33, "name? ");
}

void
parseargs(int argc, char** argv)
{
    if (argc <= 1)
        return;

    int fd = open("./main.c", O_RDWR);
    struct stat st;
    fstat(fd, &st);

    char* buff = malloc(st.st_size+1);
    read(fd, buff, st.st_size);
    buff[st.st_size] = '\0';

    const unsigned int size = st.st_size;

    int last; // start of line after config start
    for (int i = size-1; i >= 0; i--) { // find start of config ("curtime")
        if (buff[i] == '\n') {
            if (strsub("curtime", &buff[i+1]))
                break;
            last = i;
        }
    }

    switch (argv[1][0]) {
        case 'x':
            if (argv[1][1])
                break;

            for (int i = 2; i < argc; i++) {
                int o = 0;
                int v = strtoint(argv[i], &o);
                if (0 <= v && v < (int)sizeof(tagtable)/(int)sizeof(TagProps))
                    tagtable[v].print = fals;
            }

            showtasks();
            break;

        case 'i':
            if (argv[1][1])
                break;

            for (int i = 0; i < (int)sizeof(tagtable)/(int)sizeof(TagProps); i++)
                tagtable[i].print = fals;

            for (int i = 2; i < argc; i++) {
                int o = 0;
                int v = strtoint(argv[i], &o);
                if (0 <= v && v < (int)sizeof(tagtable)/(int)sizeof(TagProps))
                    tagtable[v].print = true;
            }

            showtasks();
            break;


        case 't': {
            if (argv[1][1] && argc < 2)
                break;

            int j = 0;
            if ((j = jumptoline(argc, argv, last, buff, size)) != -1)
                toggle(j, fd, buff, size);

            break;
        }

        case 'a': {
            if (argv[1][1])
                break;

            int j = 0;

            if ((j = jumptoline(argc, argv, last, buff, size)) != -1)
                addtask(j, fd, buff, size);

            break;
        }

        case 'e':
            if (argv[1][1])
                break;

            break;

        case 'd':
            if (argv[1][1])
                break;

            break;

        case 'r':
            if (argv[1][1])
                break;

            break;
    }

    free(buff);
    close(fd);
    quit(0);
}



static inline void
rebuild(char** argv)
{
    struct stat exe, imp;
    stat("./main.c", &imp);
    stat("./td", &exe);

    if (exe.st_mtim.tv_sec < imp.st_mtim.tv_sec) {
        print("info: rebuilding...\n");
        int status;
        if ((status = system("gcc -ggdb -Wall -Wextra -Wpedantic main.c -o td"))) {
            print("error: build failed\n");
            exit(status);
        }

        if (execvp("./td", argv) == -1) {
            print("error: exec failed\n");
            exit(1);
        }
    }
}

int
main(int argc, char** argv)
{
    rebuild(argv);
    parseargs(argc, argv);
    print("\x1b[?25l");
    signal(SIGINT, quit);

    int counter = 0;

    for (;;) {
        if (!counter)
            showtasks();

        counter = (counter + 1) % threshold;

        sleep(1);
        rebuild(argv);
    }
}

void
showtasks(void)
{
    print("\x1b[2J\x1b[H");
    curtime = (int)time(NULL);
    bind(0, 0, "What is a \"Catchy\"!?", 180, 1715034634, true, fals);
    bind(0, 1, "Resume", 199, 0, true, fals);
    bind(1, 1, "STARR", 0, 0, fals, fals);
    bind(1, 1, "Writeup", 0, 0, fals, fals);
    bind(0, 1, "MGEA06 lecture", 0, 0, fals, fals);
    bind(0, 1, "CSCC43 textbook", 0, 0, fals, fals);
    bind(0, 1, "CSCC69", 0, 0, fals, fals);
    bind(1, 1, "Form groups", 0, 0, fals, fals);
}
