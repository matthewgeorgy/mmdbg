#ifndef MMDBG_H
#define MMDBG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned int		dword;
typedef unsigned long long	qword;

// Macro to strip just the filename out of the full path.
#define __FILENAME__	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define MMDBG_TRUE      1
#define MMDBG_FALSE     0
#define MMDBG_FREE_BIT  0x01
#define MMDBG_OVER_BIT  0x02
#define MMDBG_UNDER_BIT 0x04

////////////////////
// C
////////////////////

void   *mmdbg_malloc(size_t size, char *file, int line);
void    mmdbg_free(void *buffer, char *file, int line);
void    mmdbg_print(FILE *stream);

 #ifdef __cplusplus

////////////////////
// C++
////////////////////

void    *operator new(size_t size, char *file, int line);
void    operator delete(void *buffer);

 #endif // __cplusplus

////////////////////
// Node
////////////////////

typedef struct _TAG_mmdbg_node
{
    void    *ptr;
    char    *file;
    int     line;
    int     size;
    byte    flags;
    struct  _TAG_mmdbg_node *next;
} mmdbg_node_t;

void    mmdbg_node_append(mmdbg_node_t **head, void *ptr, char *file, int line, int size);
void    mmdbg_node_remove(mmdbg_node_t **head, void *ptr);
void    mmdbg_node_find_buffer_runs(mmdbg_node_t **head);



//////////////////////////////////////////////////
// MMDBG IMPLEMENTATION
//////////////////////////////////////////////////

 #ifdef MMDBG_IMPL

// Counters for malloc, free, total allocation,
// and head node for the list
static int      mmdbg_malloc_cnt;
static int      mmdbg_free_cnt;
static size_t   mmdbg_total_alloc;
mmdbg_node_t    *mmdbg_alloc_head = NULL;
mmdbg_node_t    *mmdbg_alloc_record = NULL;

////////////////////
// C
////////////////////

void*
mmdbg_malloc(size_t size,
             char *file,
             int line)
{
    void    *ptr;
    dword   *buff_un;
    dword   *buff_ov;
    
    buff_un = (dword *)malloc(size + 2 * sizeof(dword));
    ptr = buff_un + 1;
    buff_ov = (dword *)((byte *)ptr + size);
    *buff_ov = 0xFFFFEEEE;
    *buff_un = 0xBBBBAAAA;

    // if allocation succeeded
    if (ptr)
    {
        mmdbg_malloc_cnt++;
        mmdbg_total_alloc += size;
        mmdbg_node_append(&mmdbg_alloc_head, ptr, file, line, size);
    }
    
    // return ptr regardless
    return ptr;
}

void
mmdbg_free(void *buffer,
           char *file,
           int line)
{
    mmdbg_node_t *temp;
    dword value;
    void *p;

    // Set the 'freed' flag
    temp = mmdbg_alloc_head;
    while (temp != NULL)
    {
        if (temp->ptr == buffer)
        {
            temp->flags |= MMDBG_FREE_BIT;
            break;
        }

        temp = temp->next;
    }

    // Check for overrun
    p = (char *)buffer + temp->size;
    value = *(dword *)p;
    if (value != 0xFFFFEEEE)
    {
        temp->flags |= MMDBG_OVER_BIT;
    }

    // Check for underrun
    p = (char *)buffer - 4;
    value = *(dword *)p;
    if (value != 0xBBBBAAAA)
    {
        temp->flags |= MMDBG_UNDER_BIT;
    }

    mmdbg_free_cnt++;

    // free the buffer
    free((dword *)buffer - 1);
}


////////////////////
// Node
////////////////////

void
mmdbg_node_append(mmdbg_node_t **head,
                  void *ptr,
                  char *file,
                  int line,
                  int size)
{
    mmdbg_node_t    *new_node;
    mmdbg_node_t    *temp;

    // Allocate and fill new node
    new_node = (mmdbg_node_t *)malloc(sizeof(mmdbg_node_t));
    new_node->ptr = ptr;
    new_node->file = file;
    new_node->line = line;
    new_node->size = size;
    new_node->flags = 0x0;
    new_node->next = NULL;

    // If list is empty
    if (*head == NULL)
    {
        *head = new_node;
    }

    else
    {
        // Find last node
        temp = *head;
        while (temp->next != NULL)
            temp = temp->next;

        // Append
        temp->next = new_node;
    }
}

// Given the rearchitecting that we're currently doing, ergo we aren't deleting
// the list anymore, this function can probably be removed at some point. For now,
// though, we'll keep it; however, we probably won't be calling it anymore.
void
mmdbg_node_remove(mmdbg_node_t **head,
                  void *ptr)
{
    mmdbg_node_t    *temp;
    mmdbg_node_t    *prev;

    // First node contains the ptr.
    temp = *head;
    if (temp != NULL && temp->ptr == ptr)
    {
        *head = temp->next;
        free(temp);
        return;
    }

    // Otherwise, traverse list to find the node.
    else
    {
        // Search the list.
        while (temp != NULL && temp->ptr != ptr)
        {
            prev = temp;
            temp = temp->next;
        }

        // Reconnect list and remove the node.
        prev->next = temp->next;
        free(temp);
    }
}

void
mmdbg_node_find_buffer_runs(mmdbg_node_t **head)
{
    mmdbg_node_t *temp;
    dword value;
    void *p;

    temp = *head;
    while (temp != NULL)
    {
        if (!(temp->flags && MMDBG_FREE_BIT))
        {
            // Find overruns
            p = (char *)temp->ptr + temp->size;
            value = *(dword *)p;
            if (value != 0xFFFFEEEE)
            {
                temp->flags |= MMDBG_OVER_BIT;
            }

            // Find underruns
            p = (char *)temp->ptr - 4;
            value = *(dword *)p;
            if (value != 0xBBBBAAAA)
            {
                temp->flags |= MMDBG_UNDER_BIT;
            }
        }

        temp = temp->next;
    }
}

  #ifdef __cplusplus

////////////////////
// C++
////////////////////

// Counters for new and delete
static int      mmdbg_new_cnt;
static int      mmdbg_delete_cnt;

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
        mmdbg_new_cnt++;
        mmdbg_total_alloc += size;
        mmdbg_node_append(&mmdbg_alloc_head, ptr, file, line, size);
    }

    // return ptr regardless
    return ptr;
}

void
operator delete(void *buffer)
{
    mmdbg_delete_cnt++;

    mmdbg_node_remove(&mmdbg_alloc_head, buffer);

    // free the buffer
    free((dword *)buffer - 1);
}

  #endif // __cplusplus

// NOTE: option to specify stream for now,
// will probably remove this later and make it just
// go straight to stdout instead.
void
mmdbg_print(FILE *stream)
{
    mmdbg_node_t    *temp;

    mmdbg_node_find_buffer_runs(&mmdbg_alloc_head);

    fprintf(stream, "=========================================================\n");
    fprintf(stream, "                    MMDBG OUTPUT\n");
    fprintf(stream, "=========================================================\n");
    fprintf(stream, "Total Mallocs: %d\n", mmdbg_malloc_cnt);
    fprintf(stream, "Total Frees:   %d\n", mmdbg_free_cnt);
#ifdef __cplusplus
    fprintf(stream, "Total News:    %d\n", mmdbg_new_cnt);
    fprintf(stream, "Total Deletes: %d\n", mmdbg_delete_cnt);
#endif
    fprintf(stream, "Total Size:    %d bytes\n", mmdbg_total_alloc);

    // Print info about memory leaks
    temp = mmdbg_alloc_head;
    while (temp != NULL)
    {
        if (!(temp->flags & MMDBG_FREE_BIT))
        {
            fprintf(stream, "\nUNFREED MEMORY:   0x%p : (%s (%d))", temp->ptr,
                                                                    temp->file,
                                                                    temp->line);
        }

        temp = temp->next;
    }

    // TODO: This implementation works, but we can do a lot better. Instead of
    // using two lists, we can add a new flag to the mmdbg_node_t to keep track of
    // some state info about our buffers.
    // The new flag will contain (as of now) 2 bits:
    //      1) Freed
    //      2) Overrun
    //      3) Underrun *(for later)
    // Each time we call free, we first check the ends of the buffer to see if they're
    // intact; if not, we set the appropriate bits. Then we free the buffer and set
    // the 'freed' bit.
    // Then, when we enter this function, we simply check each bit of the node and
    // print the appropriate info.
    //
    // NOTE: At some point, we'll need to make a design decision, because keeping a
    // complete record of all the allocations is valuable info to have.
    // However, it is unneccessary to keep this data in memory, so we'll just
    // write it to a file instead. This will come much later though.

    // Print info about buffer overruns
    temp = mmdbg_alloc_head;
    while (temp != NULL)
    {
        void *p;

        p = (char *)temp->ptr + temp->size;
        if (temp->flags & MMDBG_OVER_BIT)
        {
            fprintf(stream, "\nBUFFER OVERRUN:   0x%p : (%s (%d))", p,
                                                                    temp->file,
                                                                    temp->line);
        }

        p = (char *)temp->ptr - 4;
        if (temp->flags & MMDBG_UNDER_BIT)
        {
            fprintf(stream, "\nBUFFER UNDERRUN:   0x%p : (%s (%d))",p,
                                                                    temp->file,
                                                                    temp->line);
        }

        temp = temp->next;
    }

    fprintf(stream, "\n=========================================================\n");
    fprintf(stream, "                    END OF OUTPUT\n");
    fprintf(stream, "=========================================================\n");
}

 #endif // MMDBG_IMPL

// wrap malloc(), free(), new, and delete
#ifdef MMDBG_DEBUG

#define malloc(size)    mmdbg_malloc(size, __FILENAME__, __LINE__)
#define free(buffer)    mmdbg_free(buffer, __FILENAME__, __LINE__)

 #ifdef __cplusplus

  #define MMDBG_NEW new(__FILENAME__, __LINE__)
  #define new MMDBG_NEW

 #endif // __cplusplus

#endif

#endif // MMDBG_H
