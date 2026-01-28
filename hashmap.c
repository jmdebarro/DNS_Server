#include <stdio.h>
#include<string.h>
#include <ctype.h>
#include <stdlib.h>
#include "hashmap.h"


/* 
    Hashmap is a dynamic array where each index can be a linked list
    hashmap would be a pointer to a hashnode struct
    hashnode struct is key/val and next
*/



unsigned long hash_function(unsigned char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + tolower(c); /* hash * 33 + c */
    return hash % TABLE_SIZE;
}


hashmap init_hash_table() {
    hashmap table = calloc(TABLE_SIZE, sizeof(hashnode *));
    if (table == NULL) {
        perror("Failed Calloc: ");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    int len = 0;
    FILE *fptr = fopen("blocklist.txt", "r");

    while(getline(&line, &len, fptr) != -1) {
        line[strcspn(line, "\r\n")] = '\0';
        insert_into_table(table, line);
    }
    
    return table;
}


void insert_into_table(hashmap table, char *domain) {
    hashnode *node = malloc(sizeof(hashnode));
    char *domain_name = strdup(domain);
    node->domain = domain_name;
    node->next = NULL;

    unsigned long hash_val = hash_function((unsigned char *)domain);
    node->next = table[hash_val];
    table[hash_val] = node;
}


HASH_TABLE_SEARCH table_lookup(hashmap table, char *domain) {
    unsigned long hash_value = hash_function((unsigned char *)domain);
    hashnode *target_node = table[hash_value];
    if (target_node == NULL) {
        return FAILURE;
    } else {
        while (target_node != NULL) {
            if (strcmp(domain, target_node->domain) == 0){
                return SUCCESS;
            }
            target_node = target_node->next;
        }
    }

    return FAILURE;
}

void print_table(hashmap table) {
    int empty_cells = 0;
    for (int i=0;i<TABLE_SIZE;i++) {
        hashnode *cur_node = table[i];
        if (cur_node == NULL) {
            empty_cells++;
        } else {
            if (empty_cells > 0) {
            printf("[Empty Cell] x %d - %d to %d\n", empty_cells, i - empty_cells, i);
            }
            printf("Index: %d - ", i);
            while(cur_node->next != NULL) {
                printf("[%s]->", cur_node->domain);
                cur_node = cur_node->next;
            }
            printf("[%s]\n", cur_node->domain);
            empty_cells = 0;
        }
    }
}