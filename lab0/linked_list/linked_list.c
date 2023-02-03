#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* the struct for list items*/
struct list_item {
    int value;
    struct list_item *next;
};


/* prints all elements in the list*/
void print(struct list_item *first){
    struct list_item* iterItem = first->next;
    int index = 0;

    while (iterItem != NULL)
    {
        printf("Inxed: %d \n Value: %d \n", index, iterItem->value);
        index++;
        iterItem = iterItem->next;
    }
}

/* Puts x at the end of the list */
void append(struct list_item *first, int x){
    struct list_item* iterItem = first;

    while (true)
    {
        if (iterItem->next == NULL)
        {
            struct list_item* newItem = (struct list_item*) malloc(sizeof(struct list_item));
            newItem->value = x;
            newItem->next = NULL;
            iterItem->next = newItem;
            return;
        }
        iterItem = iterItem->next;
    }
}

/* Puts x at the beginning of the list*/
void prepend(struct list_item *first, int x){
    struct list_item* newItem = (struct list_item*) malloc(sizeof(struct list_item));
    newItem->value = x;

    struct list_item* temp = first->next;
    first->next = newItem;
    newItem->next = temp;
}

/* find the first element in the list larger than x and input x right before that element*/
void input_sorted(struct list_item *first, int x){
    struct list_item* iterItem = first;
    
    while (iterItem->next != NULL && iterItem->next->value <= x)
    {
        iterItem = iterItem->next;
    }
    struct list_item* newItem = (struct list_item*) malloc(sizeof(struct list_item));
    newItem->value = x;
    newItem->next = iterItem->next;
    iterItem->next = newItem;
}

 /* free everything dynamically allocated*/
void clear(struct list_item *first){
    while(first->next != NULL)
    {
        struct list_item *toDelete = first->next;
        first->next = toDelete->next;
        free(toDelete);
    }
}



int main(int argc, char **argv){
    struct list_item root;
    root.value = -1; /*this value is always ignored*/
    root.next = NULL;
    print(&root);
    

    prepend(&root, 10);
    append(&root, 1337);
    prepend(&root, 3);
    input_sorted(&root, 11);
    input_sorted(&root, 1400);
    input_sorted(&root, -10);
    prepend(&root, 50);
    print(&root);

    printf("Nu clearar vi listan och sedan printar\n");
    clear(&root);
    print(&root);
}