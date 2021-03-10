#ifndef MMDBG_H
#define MMDBG_H

#include <stdio.h>
#include <stdlib.h>
#include "node.h"

// C
#ifdef MMDBG_C

static int malloc_cnt;
static size_t total_alloc;
mmdbg_node_t *malloc_head = NULL;

void*
mmdbg_malloc(size_t size,
             const char *file,
             int line)
{
    void *ptr = malloc(size);

    // if allocation succeeded
    if (ptr)
    {
        // print allocation info
        malloc_cnt++;
        total_alloc += size;
        printf("-------------------------------------\n");
        printf("MALLOC:     %u bytes\n", size);
        printf("at address: %p\n", ptr);
        printf("in file:    %s\n", file);
        printf("on line:    %u\n", line);
        printf("count:      %d\n", malloc_cnt);
        printf("-------------------------------------\n");
        mmdbg_node_append(&malloc_head, ptr);
    }
    // if allocation failed
    else
    {
        // print allocation info
        printf("-------------------------------------\n");
        printf("FAILED: %zu bytes\n", size);
        printf("in file: %s\n", file);
        printf("on line: %u\n", line);
        printf("-------------------------------------\n");
    }
    
    // return ptr regardless
    return ptr;
}

void
mmdbg_free(void *buffer,
           const char *file,
           int line)
{
    // print freeing info
    malloc_cnt--;
    printf("-------------------------------------\n");
    printf("FREED:      %p\n", buffer);
    printf("at file:    %s\n", file);
    printf("on line:    %u\n", line);
    printf("count:      %d\n", malloc_cnt);
    printf("-------------------------------------\n");

    mmdbg_node_remove(&malloc_head, buffer);
    // free the buffer
    free(buffer);
}

void
mmdbg_print_c(FILE *stream)
{
    mmdbg_node_t *temp = malloc_head;

    while (temp != NULL)
    {
        fprintf(stream, "WARNING: UNFREED MEMORY -> %p\n", temp->ptr);
        temp = temp->next;
    }
}

// wrap malloc() and free()
#define malloc(size)    mmdbg_malloc(size, __FILE__, __LINE__)
#define free(buffer)    mmdbg_free(buffer, __FILE__, __LINE__)

#endif // MMDBG_C

#endif // MMDBG_H
