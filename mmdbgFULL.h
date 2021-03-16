#ifndef MMDBG_H
#define MMDBG_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

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
void    operator delete(void *buffer, char *file, int line);

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
static int      malloc_cnt;
static int      free_cnt;
static size_t   total_alloc;
mmdbg_node_t    *alloc_head = NULL;

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
        malloc_cnt++;
        total_alloc += size;
        // print allocation info
#ifdef MMDBG_DUMP_PRINT
        printf("-------------------------------------\n");
        printf("MALLOC:     %u bytes\n", size);
        printf("at address: %p\n", ptr);
        printf("in file:    %s\n", file);
        printf("on line:    %u\n", line);
        printf("count:      %d\n", malloc_cnt - free_cnt);
        printf("-------------------------------------\n");
#endif
        mmdbg_node_append(&alloc_head, ptr, file, line);
    }
    // if allocation failed
    else
    {
        // print allocation info
#ifdef MMDBG_DUMP_PRINT
        printf("-------------------------------------\n");
        printf("MALLOC FAILED:  %zu bytes\n", size);
        printf("in file:        %s\n", file);
        printf("on line:        %u\n", line);
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
    free_cnt++;
    // print freeing info
#ifdef MMDBG_DUMP_PRINT
    printf("-------------------------------------\n");
    printf("FREE:       %p\n", buffer);
    printf("at file:    %s\n", file);
    printf("on line:    %u\n", line);
    printf("count:      %d\n", malloc_cnt - free_cnt);
    printf("-------------------------------------\n");
#endif

    mmdbg_node_remove(&alloc_head, buffer);

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
static int      new_cnt;
static int      delete_cnt;

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
        mmdbg_node_append(&alloc_head, ptr, file, line);
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
operator delete(void *buffer, char *file, int line)
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

    mmdbg_node_remove(&alloc_head, buffer);

    // free the buffer
    free(buffer);
}

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
    fprintf(stream, "Total Mallocs: %d\n", malloc_cnt);
    fprintf(stream, "Total Frees:   %d\n", free_cnt);
#ifdef __cplusplus
    fprintf(stream, "Total News:    %d\n", new_cnt);
    fprintf(stream, "Total Deletes: %d\n", delete_cnt);
#endif
    fprintf(stream, "Total Size:    %d bytes\n", total_alloc);

    // Print info about memory leaks
    temp = alloc_head;
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

  #endif // __cplusplus

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
