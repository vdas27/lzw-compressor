#ifndef B6024EEC_47B1_43D8_A5E2_7E0CE27C1E65
#define B6024EEC_47B1_43D8_A5E2_7E0CE27C1E65

typedef struct array_entry {
    signed int prefix;
    short C;
    char active;
} array_entry;

typedef struct Kstack {
    short C;
    struct Kstack *below;
} Kstack;

typedef struct reader {
    int local_max;
    int marker;
    char leftover;
} reader;

//create heap allocated Kstack
Kstack *create_stack(short C);

//pop top of stack and output char located at the top
//returns pointer to top of stack
Kstack *pop(Kstack *k);

//push value to top of stack
//returns pointer to top of stack
Kstack *push(Kstack *k, short C);

//initialize heap allocated string table array with 255 ascii entries
array_entry *initialize_string_table_array(int max_bit_length);

//insert new entry into string table
void insert_entry(array_entry *st, signed int code, signed int prefix, short C, char active);

//reads code from input in ascii or bits
signed int getcode(reader *r);

//print string table in DBG.decode
//p argument to signal if printing a table before it is pruned
void print_st(array_entry *st, FILE *F, signed int max_code, char p);

//prune string table
array_entry *deprune(array_entry *st, int max_bits, signed int *new_code, signed int *newC);

void decode();

#endif /* B6024EEC_47B1_43D8_A5E2_7E0CE27C1E65 */