#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct node {
    char *text;
    struct node *next;
};

struct node* make_node(char *input) {
    struct node *n = malloc(sizeof(struct node));
    n->text = malloc(strlen(input) + 1);
    strcpy(n->text, input);
    n->next = NULL;
    return n;
}

int main() {
    char input[BUFSIZ];
    struct node *head = malloc(sizeof(struct node));
    head->next = NULL;
    struct node *current = head;

    while (fgets(input, BUFSIZ, stdin)) {
        if (input[0] == '.') break;
        current->next = make_node(input);
        current = current->next;
    }

    for (struct node *p = head->next; p; p = p->next)
        printf("%s", p->text);
    
    return 0;
}