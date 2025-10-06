#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct node {
    char *data;
    struct node* next;
};

int main(void)
{
    char line[BUFSIZ];
    struct node *head, *here, *p, *create(char *);

    head = malloc(sizeof(struct node));
    head->next = NULL;
    here = head;

    printf("Enter lines of text:\n");
    while (gets(line) != NULL) {
        if (line[0] == '.')
            break;
        here->next = create(line);
        here = here->next;
    }
    for (p = head->next; p != NULL; p = p->next)
        puts(p->data);
}

struct node *create(char *input)
{
    struct node *q;

    q = malloc(sizeof(struct node));
    q->data = malloc(strlen(input) + 1);
    strcpy(q->data, input);
    q->next = NULL;
    return(q);
}