#ifndef MMDBG_H
#define MMDBG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Perfectly legal typedefs that shouldn't collide since they are correct.
// If this produces an issue... not my problem!
// Learn how the types in the computer are named :P
typedef unsigned char       byte;
typedef unsigned short      word;
typedef unsigned int        dword;
typedef unsigned long long  qword;

// Macro to strip just the filename out of the full path.
#define __FILENAME__    (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// Useful #define constants we use
#define MMDBG_TRUE              1
#define MMDBG_FALSE             0
#define MMDBG_FREE_BIT          0x01
#define MMDBG_OVER_BIT          0x02
#define MMDBG_UNDER_BIT         0x04
#define MMDBG_DOUBLE_FREE_BIT   0x08 
#define MMDBG_OVER_NUM          0xFFFFEEEE
#define MMDBG_UNDER_NUM         0xBBBBAAAA

////////////////////
// C
////////////////////

void    *mmdbg_malloc(size_t size, char *file, int line);
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
    char    *df_file;
    int     line;
    int     df_line;
    int     size;
    byte    flags;
    struct  _TAG_mmdbg_node *next;
} mmdbg_node_t;

void    mmdbg_node_append(mmdbg_node_t **head, void *ptr, char *file, int line, int size);
void    mmdbg_node_remove(mmdbg_node_t **head, void *ptr);
void    mmdbg_node_find_buffer_runs(mmdbg_node_t *head);



//////////////////////////////////////////////////
// MMDBG IMPLEMENTATION
//////////////////////////////////////////////////

// #define this symbol in exactly 1 AND ONLY 1 .c/.cpp file
// before including the utility!
// For example:
//          #include <...>
//          #include <...>
//          #include <...>
//
//          #define MMDBG_IMPL
//          #include <mmdbg.h>
 #ifdef MMDBG_IMPL

// Counters for malloc, free, total allocation,
// and head node for the list
static int      mmdbg_malloc_cnt;
static int      mmdbg_free_cnt;
static size_t   mmdbg_total_alloc;
mmdbg_node_t    *mmdbg_alloc_head = NULL;

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

    // if allocation succeeded
    if (buff_un)
    {
        ptr = buff_un + 1;
        buff_ov = (dword *)((byte *)ptr + size);
        *buff_ov = MMDBG_OVER_NUM;
        *buff_un = MMDBG_UNDER_NUM;

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
    mmdbg_node_t    *temp;
    dword           value;
    void            *p;

    // Set the 'freed' (or 'double freed') flag(s)
    temp = mmdbg_alloc_head;
    while (temp != NULL)
    {
        if (temp->ptr == buffer)
        {
            if (!(temp->flags & MMDBG_FREE_BIT))
            {
                temp->flags |= MMDBG_FREE_BIT;
            }
            else if (temp->flags & MMDBG_FREE_BIT)
            {
                temp->flags |= MMDBG_DOUBLE_FREE_BIT;
                temp->df_file = file;
                temp->df_line = line;
            }

            break;
        }

        temp = temp->next;
    }

    if (!(temp->flags & MMDBG_DOUBLE_FREE_BIT))
    {
        // Check for overrun
        p = (char *)buffer + temp->size;
        value = *(dword *)p;
        if (value != MMDBG_OVER_NUM)
        {
            temp->flags |= MMDBG_OVER_BIT;
        }

        // Check for underrun
        p = (char *)buffer - 4;
        value = *(dword *)p;
        if (value != MMDBG_UNDER_NUM)
        {
            temp->flags |= MMDBG_UNDER_BIT;
        }

        mmdbg_free_cnt++;

        // free the buffer
        free((dword *)buffer - 1);
    }
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
    new_node->df_file = NULL;
    new_node->df_line = 0;
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

// Given the rearchitecting that we're currently doing, (ergo we aren't deleting
// the list anymore), this function can probably be removed at some point. For now,
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

// Go through the list and find any buffer overruns/underruns,
// and then update each ptr's node accordingly
void
mmdbg_node_find_buffer_runs(mmdbg_node_t *head)
{
    mmdbg_node_t    *temp;
    dword           value;
    void            *p;

    temp = head;
    while (temp != NULL)
    {
        if (!(temp->flags && MMDBG_FREE_BIT))
        {
            // Find overruns
            p = (char *)temp->ptr + temp->size;
            value = *(dword *)p;
            if (value != MMDBG_OVER_NUM)
            {
                temp->flags |= MMDBG_OVER_BIT;
            }

            // Find underruns
            p = (char *)temp->ptr - 4;
            value = *(dword *)p;
            if (value != MMDBG_UNDER_NUM)
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
    dword   *buff_un;
    dword   *buff_ov;
    
    buff_un = (dword *)malloc(size + 2 * sizeof(dword));

    // if allocation succeeded
    if (buff_un)
    {
        ptr = buff_un + 1;
        buff_ov = (dword *)((byte *)ptr + size);
        *buff_ov = MMDBG_OVER_NUM;
        *buff_un = MMDBG_UNDER_NUM;

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
    mmdbg_node_t    *temp;
    dword           value;
    void            *p;

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
    if (value != MMDBG_OVER_NUM)
    {
        temp->flags |= MMDBG_OVER_BIT;
    }

    // Check for underrun
    p = (char *)buffer - 4;
    value = *(dword *)p;
    if (value != MMDBG_UNDER_NUM)
    {
        temp->flags |= MMDBG_UNDER_BIT;
    }

    mmdbg_delete_cnt++;

    // free the buffer
    free((dword *)buffer - 1);
}

  #endif // __cplusplus

void
mmdbg_print(FILE *stream)
{
    mmdbg_node_t    *temp;

    mmdbg_node_find_buffer_runs(mmdbg_alloc_head);

    fprintf(stream, "\n=========================================================\n");
    fprintf(stream, "                    MMDBG OUTPUT\n");
    fprintf(stream, "=========================================================\n");
    fprintf(stream, "Total Mallocs: %d\n", mmdbg_malloc_cnt);
    fprintf(stream, "Total Frees:   %d\n", mmdbg_free_cnt);
#ifdef __cplusplus
    fprintf(stream, "Total News:    %d\n", mmdbg_new_cnt);
    fprintf(stream, "Total Deletes: %d\n", mmdbg_delete_cnt);
#endif
    fprintf(stream, "Total Size:    %d bytes\n\n", mmdbg_total_alloc);

    // Print out all debug-related info
    temp = mmdbg_alloc_head;
    while (temp != NULL)
    {
        void *p;

        // Memory Leaks
        if (!(temp->flags & MMDBG_FREE_BIT))
        {
            fprintf(stream, "UNFREED MEMORY:   0x%p (%s (%d))\n", temp->ptr,
                                                                  temp->file,
                                                                  temp->line);
        }

        // Double Frees
        if (temp->flags & MMDBG_DOUBLE_FREE_BIT)
        {
            fprintf(stream, "DOUBLE FREE:      0x%p (%s (%d))\n", temp->ptr,
                                                                  temp->df_file,
                                                                  temp->df_line);
        }

        // Buffer Underruns
        if (temp->flags & MMDBG_UNDER_BIT)
        {
            p = (char *)temp->ptr - 4;
            fprintf(stream, "BUFFER UNDERRUN:  0x%p (%s (%d))\n", p,
                                                                  temp->file,
                                                                  temp->line);
        }

        // Buffer Overruns
        if (temp->flags & MMDBG_OVER_BIT)
        {
            p = (char *)temp->ptr + temp->size;
            fprintf(stream, "BUFFER OVERRUN:   0x%p (%s (%d))\n", p,
                                                                  temp->file,
                                                                  temp->line);
        }

        temp = temp->next;
    }

    fprintf(stream, "=========================================================\n");
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
