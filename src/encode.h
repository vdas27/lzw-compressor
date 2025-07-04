#ifndef AB1D5576_F986_4B13_AE08_ABCF50CDF907
#define AB1D5576_F986_4B13_AE08_ABCF50CDF907

typedef struct entry {
    signed int prefix;
    signed int code;
    struct entry *next;
    short C;
} entry;

typedef struct hash_table {
    u_int64_t (*hash)(short, signed int);
    entry **buckets;
    u_int32_t n_buckets;
} hash_table;

typedef struct bits_info {
    int local_max;
    int marker;
    char buffer;
} bits_info;

//print codes in binary format
void bit_print(signed int K, bits_info *b);

//construct heap allocated entry struct
entry *e_cons(signed int pre, signed int code, short C, entry *n);

//intialize new heap allocated hash_table struct
hash_table *ht_new(u_int32_t n, u_int64_t (*h)(short, signed int));

//hashing function for encode
u_int64_t C_pref_hash(short C, signed int pref);

//insert new entry into string table
void entry_insert(hash_table *ht, entry *e);


//lookup a code value in string table using prefix and character
//returns -1 if entry does not exist
signed int code_lookup(hash_table *ht, signed int prefix, short C);

//free entry struct from heap
void free_entry(entry *e);

//free hash_table struct from heap
void free_ht(hash_table *ht);

//initialize string table with 255 ascii entries
hash_table *intialize_string_table(signed int max_code);

//debugging function to check balance of hash_table buckets
//prints to stderr
void ht_balance(hash_table *ht);

//print string table to DBG.encode
//p argument to signal if printing a table before it is pruned
void print_ht(hash_table *ht, FILE *F, signed int max, char p);

//prune hash table
hash_table *enprune(hash_table *ht, signed int *prune_valid, signed int max, signed int *new_code, signed int *prefix);

void encode(int p, int max_bit_length);

#endif /* AB1D5576_F986_4B13_AE08_ABCF50CDF907 */