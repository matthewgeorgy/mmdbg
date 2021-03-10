#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct _TAG_node
{
    void *ptr;
    struct _TAG_node *next;
} node_t;

void
node_append(node_t **head, void *ptr)
{
    
	// Allocate and fill new node
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->ptr = ptr;
	new_node->next = NULL;

    // If list is empty
    if (*head == NULL)
    {
        *head = new_node; printf("HERE\n");
    }
    else
    {
        // Find last node
        node_t *temp = *head;
        while (temp->next != NULL)
            temp = temp->next;

        // Append
        temp->next = new_node;
    }
}

void
node_remove(node_t **head, void *ptr)
{
    // List empty
    if (*head == NULL)
        return;

    node_t *temp = *head;
    node_t *prev;

    // One node in list
    if (temp != NULL && temp->next == NULL)
    {
        free(temp);
    }

	// Head node contains the ptr
    else if (temp != NULL && temp->ptr == ptr)
    {
        *head = (*head)->next;
        free(temp);
    }

	// Search the list
    else
    {
        prev = temp;
        temp = temp->next;

        // Traverse list to find the node with ptr
        while (temp->next != NULL)
        {
            // Remove node
            if (temp->ptr == ptr)
            {
                prev->next = temp->next;
                free(temp);
            }
            // Keep walking through
            else
            {
                prev = temp; 
                temp = temp->next;
            }

        }

    }
}

#endif // NODE_H
