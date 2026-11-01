#!/bin/bash
gcc -g -fsanitize=address -lasan hashmap.c && ./a.out
