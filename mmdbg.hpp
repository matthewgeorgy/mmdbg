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

void*
operator new(size_t size, char,
             char *file,
             int line)
{
    void *ptr = malloc(size);
    new_cnt++;
    total_alloc += size;
    mmdbg_node_append(&new_head, ptr, file, line);

    return ptr;
}

void
operator delete(void *buffer)
{
    delete_cnt++;
    mmdbg_node_remove(&new_head, buffer);

    free(buffer);
}

void
mmdbg_print_cpp(FILE *stream)
{
    fprintf(stream, "=========================================================\n");
    fprintf(stream, "                    MMDBG OUTPUT\n");
    fprintf(stream, "=========================================================\n");
    fprintf(stream, "Total News:    %d\n", new_cnt);
    fprintf(stream, "Total Deletes: %d\n", delete_cnt);
    fprintf(stream, "Total Size:    %d bytes\n", total_alloc);
    mmdbg_node_t *temp = new_head;

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

#define NEW new(0, __FILENAME__, __LINE__)
#define new NEW

#endif // MMDBG_HPP
