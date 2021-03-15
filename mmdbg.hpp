#ifndef MMDBG_HPP
#define MMDBG_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

// Macro to strip just the filename out of the full path (WINDOWS).
#define __FILENAME__	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

static int new_cnt;
static int delete_cnt;
static size_t total_alloc;
mmdbg_node_t *new_head = NULL;

void *operator new(size_t size, char, char *file, int line)
{
    void *p = malloc(size);
    new_cnt++;
    total_alloc += size;
    printf("%s %d\n", file, line);

    return p;
}

void operator delete(void *p)
{
    delete_cnt++;
    printf("%s %d\n", __FILENAME__, __LINE__);

    free(p);
}

#define NEW new(0, __FILENAME__, __LINE__)
#define new NEW

#endif // MMDBG_HPP
