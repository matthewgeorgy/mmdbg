#ifndef MMDBG_H
#define MMDBG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

// Macro to strip just the filename out of the full path (WINDOWS).
#define __FILENAME__	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

static int malloc_cnt;
static int free_cnt;
static size_t total_alloc;
mmdbg_node_t *malloc_head = NULL;

void*
mmdbg_malloc(size_t size,
             char *file,
             int line)
{
    void *ptr = malloc(size);

    // if allocation succeeded
    if (ptr)
    {
        // print allocation info
        malloc_cnt++;
        total_alloc += size;
#ifdef MMDBG_DUMP_PRINT
        printf("-------------------------------------\n");
        printf("MALLOC:     %u bytes\n", size);
        printf("at address: %p\n", ptr);
        printf("in file:    %s\n", file);
        printf("on line:    %u\n", line);
        printf("count:      %d\n", malloc_cnt - free_cnt);
        printf("-------------------------------------\n");
#endif
        mmdbg_node_append(&malloc_head, ptr, file, line);
    }
    // if allocation failed
    else
    {
        // print allocation info
#ifdef MMDBG_DUMP_PRINT
        printf("-------------------------------------\n");
        printf("FAILED: %zu bytes\n", size);
        printf("in file: %s\n", file);
        printf("on line: %u\n", line);
        printf("-------------------------------------\n");
#endif 
    }
    
    // return ptr regardless
    return ptr;
}

void
mmdbg_free(void *buffer,
           char *file,
           int line)
{
    // print freeing info
    free_cnt++;
#ifdef MMDBG_DUMP_PRINT
    printf("-------------------------------------\n");
    printf("FREED:      %p\n", buffer);
    printf("at file:    %s\n", file);
    printf("on line:    %u\n", line);
    printf("count:      %d\n", malloc_cnt - free_cnt);
    printf("-------------------------------------\n");
#endif
    mmdbg_node_remove(&malloc_head, buffer);
    // free the buffer
    free(buffer);
}

void
mmdbg_print_c(FILE *stream)
{
    fprintf(stream, "=========================================================\n");
    fprintf(stream, "                    MMDBG OUTPUT\n");
    fprintf(stream, "=========================================================\n");
    fprintf(stream, "Total Mallocs: %d\n", malloc_cnt);
    fprintf(stream, "Total Frees:   %d\n", free_cnt);
    fprintf(stream, "Total Size:    %d bytes\n", total_alloc);
    mmdbg_node_t *temp = malloc_head;

    while (temp != NULL)
    {
        fprintf(stream, "\nWARNING: UNFREED MEMORY:   0x%p : (%s (%d))", temp->ptr,
                                                                         temp->file,
                                                                         temp->line);
        temp = temp->next;
    }
	fprintf(stream, "\n=========================================================\n");
	fprintf(stream, "                    END OF OUTPUT\n");
	fprintf(stream, "=========================================================\n");
}

// wrap malloc() and free()
#ifdef MMDBG_DEBUG_C
#define malloc(size)    mmdbg_malloc(size, __FILENAME__, __LINE__)
#define free(buffer)    mmdbg_free(buffer, __FILENAME__, __LINE__)
#endif // MMDBG_DEBUG_C

#endif // MMDBG_H
