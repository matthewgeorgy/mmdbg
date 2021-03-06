#ifndef MMDBG_H
#define MMDBG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: fix any remaining alignments
// TODO: add comments documenting the funcs

// Perfectly legal typedefs that shouldn't collide since they are correct.
// If they do cause collisions then just change them manually.
typedef unsigned char   byte;
typedef unsigned int    dword;

// Macro to strip just the filename out of the full path.
#if defined(_WIN32) || defined(_WIN64)
    #define __FILENAME__    (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
    #define __FILENAME__    (strrchr(__FILE__, '/ ') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

// Useful #define constants we use
#define MMDBG_TRUE              1
#define MMDBG_FALSE             0
#define MMDBG_FREE_BIT          0x01
#define MMDBG_OVER_BIT          0x02
#define MMDBG_UNDER_BIT         0x04
#define MMDBG_DOUBLE_FREE_BIT   0x08 
#define MMDBG_OVER_NUM          0x192BA3A2
#define MMDBG_UNDER_NUM         0x39D7A5DA

// For C vs. C++ compilation
#ifdef __cplusplus
    #define MMDBG_EXTERN    extern "C"
#else
    #define MMDBG_EXTERN    extern
#endif

// Defines NULL in C/C++ if not already defined (should be since <stdlib.h>)
#ifndef NULL
    #ifdef __cplusplus
        #define NULL    0
    #else
        #define NULL    ((void *)0)
    #endif // __cplusplus
#endif // NULL

// Record structure for storing memory information
typedef struct _TAG_mmdbg_rec
{
    void    *ptr;
    char    *file,
            *df_file;
    int     line,
            df_line,
            size;
    byte    flags;
    struct  _TAG_mmdbg_rec *next;
} mmdbg_rec_t;

MMDBG_EXTERN void   *mmdbg_malloc(size_t size, const char *file, int line);
MMDBG_EXTERN void   mmdbg_free(void *buffer, const char *file, int line);
MMDBG_EXTERN void   mmdbg_print(FILE *stream);
MMDBG_EXTERN void   mmdbg_rec_append(mmdbg_rec_t **head, void *ptr, const char *file, int line, int size);
MMDBG_EXTERN void   mmdbg_debug_memory();
#ifdef __cplusplus
    void            *operator new(size_t size, const char *file, int line);
    void            operator delete(void *buffer);
#endif // __cplusplus



//////////////////////////////////////////////////
// MMDBG IMPLEMENTATION
//////////////////////////////////////////////////

// #define this symbol in exactly 1 .c/.cpp file
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
static int              mmdbg_malloc_cnt;
static int              mmdbg_free_cnt;
static size_t           mmdbg_total_alloc;
static mmdbg_rec_t     *mmdbg_alloc_head = NULL;

////////////////////
// C
////////////////////

void *
mmdbg_malloc(size_t size,
             const char *file,
             int line)
{
    void    *ptr;
    dword   *buff_un,
            *buff_ov;
    
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
        mmdbg_rec_append(&mmdbg_alloc_head, ptr, file, line, size);
    }
    
    // return ptr regardless
    return ptr;
}

void
mmdbg_free(void *buffer,
           const char *file,
           int line)
{
    mmdbg_rec_t     *temp;
    dword           value;
    void            *p;

    // Set the 'freed' (or 'double freed') flag(s) if necessary
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
                temp->df_file = (char *)file;
                temp->df_line = line;
            }

            break;
        }

        temp = temp->next;
    }

    // After (potentially) setting the 'double freed' flag, proceed to
    // check for any buffer runs and then free the ptr.
    if (!(temp->flags & MMDBG_DOUBLE_FREE_BIT))
    {
        // Check for overrun
        p = (byte *)buffer + temp->size;
        value = *(dword *)p;
        if (value != MMDBG_OVER_NUM)
        {
            temp->flags |= MMDBG_OVER_BIT;
        }

        // Check for underrun
        p = (byte *)buffer - 4;
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
mmdbg_rec_append(mmdbg_rec_t **head,
                 void *ptr,
                 const char *file,
                 int line,
                 int size)
{
    mmdbg_rec_t     *new_node,
                    *temp;

    // Allocate and fill new node
    new_node = (mmdbg_rec_t *)malloc(sizeof(mmdbg_rec_t));
    new_node->ptr = ptr;
    new_node->file = (char *)file;
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

// Go through the list and find any buffer overruns/underruns,
// and then update each ptr's node accordingly
void
mmdbg_debug_memory()
{
    mmdbg_rec_t     *temp;
    dword           value;
    void            *p;

    temp = mmdbg_alloc_head;
    while (temp != NULL)
    {
        if (!(temp->flags && MMDBG_FREE_BIT))
        {
            // Find overruns
            p = (byte *)temp->ptr + temp->size;
            value = *(dword *)p;
            if (value != MMDBG_OVER_NUM)
            {
                temp->flags |= MMDBG_OVER_BIT;
            }

            // Find underruns
            p = (byte *)temp->ptr - 4;
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

void *
operator new(size_t size,
             const char *file,
             int line)
{
    void    *ptr;
    dword   *buff_un,
            *buff_ov;
    
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
        mmdbg_rec_append(&mmdbg_alloc_head, ptr, file, line, size);
    }

    // return ptr regardless
    return ptr;
}

void
operator delete(void *buffer)
{
    mmdbg_rec_t     *temp;
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
    p = (byte *)buffer + temp->size;
    value = *(dword *)p;
    if (value != MMDBG_OVER_NUM)
    {
        temp->flags |= MMDBG_OVER_BIT;
    }

    // Check for underrun
    p = (byte *)buffer - 4;
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
    mmdbg_rec_t     *temp;
    void            *p;

    mmdbg_debug_memory();

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
            p = (byte *)temp->ptr - 4;
            fprintf(stream, "BUFFER UNDERRUN:  0x%p (%s (%d))\n", p,
                                                                  temp->file,
                                                                  temp->line);
        }

        // Buffer Overruns
        if (temp->flags & MMDBG_OVER_BIT)
        {
            p = (byte *)temp->ptr + temp->size;
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

#define malloc(__size)      mmdbg_malloc(__size, __FILENAME__, __LINE__)
#define free(__buffer)      mmdbg_free(__buffer, __FILENAME__, __LINE__)

 #ifdef __cplusplus

  #define MMDBG_NEW new(__FILENAME__, __LINE__)
  #define new MMDBG_NEW

 #endif // __cplusplus

#endif // MMDBG_DEBUG

#endif // MMDBG_H
