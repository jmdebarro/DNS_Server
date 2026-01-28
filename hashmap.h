#define TABLE_SIZE 200003

typedef struct hashnode {
    char *domain;
    struct hashnode *next;
} hashnode;

typedef hashnode ** hashmap;

typedef enum {
    SUCCESS,
    FAILURE
} HASH_TABLE_SEARCH;

unsigned long hash_function(unsigned char *str);
hashmap init_hash_table();
void insert_into_table(hashmap table, char *domain);
HASH_TABLE_SEARCH table_lookup(hashmap table, char *domain);
void print_table(hashmap table);