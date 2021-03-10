#ifndef MMDBG_NODE_H
#define MMDBG_NODE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct _TAG_mmdbg_node
{
    void *ptr;
    char *file;
    int line;
    struct _TAG_mmdbg_node *next;
} mmdbg_node_t;

void
mmdbg_node_append(mmdbg_node_t **head,
                  void *ptr,
                  char *file,
                  int line)
{
	// Allocate and fill new node
    mmdbg_node_t *new_node = (mmdbg_node_t *)malloc(sizeof(mmdbg_node_t));
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
        mmdbg_node_t *temp = *head;
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
    mmdbg_node_t *temp = *head;
    mmdbg_node_t *prev;

    // First node contains the ptr.
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

#endif // MMDBG_NODE_H
