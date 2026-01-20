#include <stdio.h>
#include<string.h>
#include <ctype.h>
#include <stdlib.h>


#define TABLE_SIZE 200003

/* 
    Hashmap is a dynamic array where each index can be a linked list
    hashmap would be a pointer to a hashnode struct
    hashnode struct is key/val and next
*/
struct hashnode {
    char *domain;
    struct hashnode *next;
} typedef hashnode;


unsigned long hash_function(unsigned char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + tolower(c); /* hash * 33 + c */
    return hash % TABLE_SIZE;
}

hashnode ** init_hash_table() {
    hashnode **table = calloc(TABLE_SIZE, sizeof(hashnode *));
    if (table == NULL) {
        perror("Failed Calloc: ");
        exit(EXIT_FAILURE);
    }

    return table;
}

void insert_into_table(hashnode **table, char *domain) {
    hashnode *node = malloc(sizeof(hashnode));
    char *domain_name = strdup(domain);
    node->domain = domain_name;
    node->next = NULL;

    unsigned long hash_val = hash_function(domain);
    node->next = table[hash_val];
    table[hash_val] = node;
}

int table_lookup(hashnode **table, char *domain) {
    unsigned long hash_value = hash_function(domain);
    hashnode *target_node = table[hash_val];
    if (target_node == NULL) {
        return 0;
    } else {
        while (target_node != NULL) {
            if (strcmp(domain, target_node->domain) == 0){
                return 1;
            }
            target_node = target_node->next;
        }
    }

    return 0;
}