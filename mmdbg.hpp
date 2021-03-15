#ifndef MMDBG_HPP
#define MMDBG_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

// Macro to strip just the filename out of the full path.
#define __FILENAME__	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// Counters for new, delete, total allocation
// and head node for the list
static int      new_cnt;
static int      delete_cnt;
static size_t   total_alloc;
mmdbg_node_t    *new_head = NULL;

void*
operator new(size_t size,
             char *file,
             int line)
{
    void    *ptr;

    ptr = malloc(size);

    // if allocation succeeded
    if (ptr)
    {
        new_cnt++;
        total_alloc += size;
        // print allocation info
#ifdef MMDBG_DUMP_PRINT
        printf("-------------------------------------\n");
        printf("NEW:        %u bytes\n", size);
        printf("at address: %p\n", ptr);
        printf("in file:    %s\n", file);
        printf("on line:    %u\n", line);
        printf("count:      %d\n", new_cnt - delete_cnt);
        printf("-------------------------------------\n");
#endif
        mmdbg_node_append(&new_head, ptr, file, line);
    }

    // if allocation failed
    else
    {
        // print allocation info
#ifdef MMDBG_DUMP_PRINT
        printf("-------------------------------------\n");
        printf("NEW FAILED: %zu bytes\n", size);
        printf("in file:    %s\n", file);
        printf("on line:    %u\n", line);
        printf("-------------------------------------\n");
#endif
    }

    // return ptr regardless
    return ptr;
}

void
operator delete(void *buffer)
{
    delete_cnt++;

    // print freeing info
#ifdef MMDBG_DUMP_PRINT
    printf("-------------------------------------\n");
    printf("DELETE:     %p\n", buffer);
    printf("at file:    %s\n", file);
    printf("on line:    %u\n", line);
    printf("count:      %d\n", new_cnt - delete_cnt);
    printf("-------------------------------------\n");
#endif

    mmdbg_node_remove(&new_head, buffer);

    // free the buffer
    free(buffer);
}

void
mmdbg_print_cpp(FILE *stream)
{
    mmdbg_node_t    *temp;

    fprintf(stream, "=========================================================\n");
    fprintf(stream, "                    MMDBG OUTPUT\n");
    fprintf(stream, "=========================================================\n");
    fprintf(stream, "Total News:    %d\n", new_cnt);
    fprintf(stream, "Total Deletes: %d\n", delete_cnt);
    fprintf(stream, "Total Size:    %d bytes\n", total_alloc);

    // Print info about memory leaks
    temp = new_head;
    while (temp != NULL)
    {
        fprintf(stream, "\nUNFREED MEMORY:   0x%p : (%s (%d))", temp->ptr,
                                                                temp->file,
                                                                temp->line);
        temp = temp->next;
    }

    fprintf(stream, "\n=========================================================\n");
    fprintf(stream, "                    END OF OUTPUT\n");
    fprintf(stream, "=========================================================\n");
}

// wrap new and delete
#ifdef MMDBG_DEBUG_CPP
#define MMDBG_NEW new(__FILENAME__, __LINE__)
#define new MMDBG_NEW
#endif // MMDBG_DEBUG_CPP

#endif // MMDBG_HPP
