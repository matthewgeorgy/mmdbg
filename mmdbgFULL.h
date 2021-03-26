#ifndef MMDBG_H
#define MMDBG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Macro to strip just the filename out of the full path.
#define __FILENAME__	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

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
    struct  _TAG_mmdbg_node *next;
} mmdbg_node_t;

void    mmdbg_node_append(mmdbg_node_t **head, void *ptr, char *file, int line);
void    mmdbg_node_remove(mmdbg_node_t **head, void *ptr);



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

////////////////////
// C
////////////////////

void*
mmdbg_malloc(size_t size,
             char *file,
             int line)
{
    void    *ptr;
    
    ptr = malloc(size);

    // if allocation succeeded
    if (ptr)
    {
        mmdbg_malloc_cnt++;
        mmdbg_total_alloc += size;
        mmdbg_node_append(&mmdbg_alloc_head, ptr, file, line);
    }
    
    // return ptr regardless
    return ptr;
}

void
mmdbg_free(void *buffer,
           char *file,
           int line)
{
    mmdbg_free_cnt++;

    mmdbg_node_remove(&mmdbg_alloc_head, buffer);

    // free the buffer
    free(buffer);
}


////////////////////
// Node
////////////////////

void
mmdbg_node_append(mmdbg_node_t **head,
                  void *ptr,
                  char *file,
                  int line)
{
    mmdbg_node_t    *new_node;
    mmdbg_node_t    *temp;

    // Allocate and fill new node
    new_node = (mmdbg_node_t *)malloc(sizeof(mmdbg_node_t));
    new_node->ptr = ptr;
    new_node->file = file;
    new_node->line = line;
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
        mmdbg_node_append(&mmdbg_alloc_head, ptr, file, line);
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
    free(buffer);
}

  #endif // __cplusplus

// NOTE: option to specify stream for now,
// will probably remove this later and make it just
// go straight to stdout instead.
void
mmdbg_print(FILE *stream)
{
    mmdbg_node_t    *temp;

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
        fprintf(stream, "\nUNFREED MEMORY:   0x%p : (%s (%d))", temp->ptr,
                                                                temp->file,
                                                                temp->line);
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
