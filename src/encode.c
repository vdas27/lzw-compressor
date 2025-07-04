#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"

void bit_print(signed int K, bits_info *b) {
    int kshift; char j;
    int m = b->local_max;
    int slots = 8 - b->marker;
    
    m -= slots;
    kshift = K >> m;
    j  = (char) kshift | b->buffer;

    putchar(j);
    kshift = kshift << m;
    kshift = kshift ^ K;
    
    while (m > 8) {
        m -= 8;
        kshift = K >> m;
        j  = (char) kshift;
        putchar(j);
        kshift = kshift << m;
        kshift = kshift ^ K;
    }

    b->buffer = (char) K;
    b->buffer = b->buffer << (8 - m);
    b->marker = m;
}

entry *e_cons(signed int pre, signed int code, short C, entry *n) {
    entry *a = calloc(1, sizeof(entry));
    if (!(a)) {
        fprintf(stderr, "ERROR: Failed to allocate memory for entry. Exiting function.\n");
        exit(1);
    }
    a->prefix = pre;
    a->code = code;
    a->C = C;
    a->next = n;
    return a;
}

hash_table *ht_new(u_int32_t n, u_int64_t (*h)(short, signed int)) {
    hash_table *t = calloc(1, sizeof(hash_table));
    if (!(t)) {
        fprintf(stderr, "ERROR: Failed to allocate memory for hash table. Exiting function.\n ");
        exit(1);
    }
    
    t->n_buckets = n;
    t->hash = h;
    t->buckets = calloc(n, sizeof(entry));
    if (!(t->buckets)) {
        fprintf(stderr, "ERROR: Failed to allocate memory for buckets. Exiting function.\n");
        exit(1);
    }
    for (u_int32_t i = 0; i < n; i++) {
        t->buckets[i] = NULL;
    }
    return t;
}

u_int64_t C_pref_hash(short C, signed int pref) {
    int CHAR_BIT = sizeof(char) * 8;
    u_int64_t hash_code = (((long unsigned)(pref) << CHAR_BIT) | (unsigned)(C));
    return hash_code;
}

void entry_insert(hash_table *ht, entry *e) {
    u_int32_t bucket_index = (ht->hash)(e->C, e->prefix) % ht->n_buckets;
    e->next = NULL;
    entry *bucket = ht->buckets[bucket_index];
    entry *new_entry = e;
    if (!(bucket)) {
        ht->buckets[bucket_index] = new_entry;
    } else {
        new_entry->next = bucket;
        ht->buckets[bucket_index] = new_entry;
    }
}

signed int code_lookup(hash_table *ht, signed int prefix, short C){
    u_int32_t bucket_index = (ht->hash)(C, prefix) % ht->n_buckets;
    entry *bucket = ht->buckets[bucket_index];
    while (bucket != NULL) {
        

        if ((prefix == bucket->prefix) && (C == bucket->C)) {
            
            return (bucket->code);
        }
        bucket = bucket->next;
    }
    return -1;
}

void free_entry(entry *e) {
    if (e->next){
        free_entry(e->next);
    }
    free(e);
}

void free_ht(hash_table *ht) {
    for (u_int32_t i = 0; i < ht->n_buckets; i++){
        if (ht->buckets[i]) free_entry(ht->buckets[i]);
    }
    if (ht->buckets) free(ht->buckets);
    free(ht);
}

hash_table *intialize_string_table(signed int max_code) {
    hash_table *string_table = ht_new(5000, C_pref_hash);
    for (signed int i = 0; i < 256; i++) {
        entry *e = e_cons((signed int) -1, (signed int) i, (short) i, NULL);
        entry_insert(string_table, e);
    }
    return string_table;
}

void ht_balance(hash_table *ht) {
    for (int i = 0; i < ht->n_buckets; i++) {
        int a = 0;
        entry *bucket = ht->buckets[i];
        while (bucket) {
            a++;
            bucket = bucket->next;
        }
        fprintf(stderr, "Bucket %i has %i entries\n", i, a);
    }
}

entry *code_search(hash_table *ht, signed int code) {
    u_int32_t n = ht->n_buckets;
    for (u_int32_t i = 0; i < n; i++) {
        entry *bucket = ht->buckets[i];
        while (bucket) {
            if (bucket->code == code) {
                return bucket;
            }
            bucket = bucket->next;
        }
    }
    return NULL;
}

void print_ht(hash_table *ht, FILE *F, signed int max, char p) {
    if (p) {
        fprintf(F,"\nPre-prune table: \n");
    } else {
        fprintf(F,"\nFinal table: \n");
    }
    fprintf(F, "Code:    Prefix  C\n");
    
    for (signed int i = 256; i < max; i++) {
        entry *a = code_search(ht, i);
        if (a) {
            fprintf(F, "  %i:    %i       %c\n", a->code, a->prefix, (char) a->C);
        }
    }
}


    
hash_table *enprune(hash_table *ht, signed int *prune_valid, signed int max, signed int *new_code, signed int *prefix) {
    
    *new_code = 256;
    for (signed int i = 0; i <  max - 256; i++) {
        if (prune_valid[i]) {
            prune_valid[i] = *new_code;
            *new_code += 1;   
        }
    }

    if (*prefix > 255) {
        *prefix = prune_valid[*prefix - 256];
    }
    
    hash_table *new = intialize_string_table(max);
    u_int32_t n = ht->n_buckets;
    signed int code, pre;
    for (u_int32_t i = 0; i < n; i++) {
        
        entry *a = ht->buckets[i];
        while (a) {
            
            code = a->code;
            
            if (code > 255) {
                
                if (prune_valid[code - 256]) {
                    if (a->prefix > 255) {
                        pre = prune_valid[a->prefix - 256];
                    } else {
                        pre = a->prefix;
                    }
                    entry *b = e_cons(pre, prune_valid[code - 256], a->C, NULL);
                    entry_insert(new, b);
                }
            }
            
            a = a->next;
        }
    }
    
    free_ht(ht);
    
    return new;
}


void encode(int p, int max_bit_length) {
    
    const char *STAGE = "STAGE";
    const char *stage = getenv(STAGE);
    int nstage = 3;
    if (stage) nstage = atoi(stage);
    
    bits_info *b = NULL;

    if (nstage == 2) {
        fprintf(stdout, "%i\n", max_bit_length);
        fprintf(stdout, "%i\n", p ? 1:0);
    } else if (nstage == 3) {
        b = malloc(sizeof(bits_info));
        if (!b) {
            fprintf(stderr, "ERROR: Failed to allocate memory for bits_info. Exiting function.\n ");
            exit(1);
        }
        b->local_max = 9;
        b->marker = 0;
        b->buffer = 0x00;
        putchar((char) max_bit_length);
        putchar((char) p);
    }

    
    const char *DBG = "DBG";
    const char *value = getenv(DBG);
    int dbgprint;
    FILE *F;
    if (value) dbgprint = atoi(value);
    if (dbgprint) {
        F = fopen("DBG.encode", "w");
    }

    int C = -1, K;
    signed int prefix, new_code, max_code;
    signed int *prune_valid;
    prefix = (signed int) -1;
    new_code = (signed int) 256;
    max_code = (signed int) (0x1 << max_bit_length);
    hash_table *string_table = intialize_string_table(max_code);
    
    if (p) {
        prune_valid = (signed int *) calloc(max_code - 256, sizeof(signed int));
        if (!(prune_valid)) {
            fprintf(stderr, "ERROR: Failed to allocate memory for prune list. Exiting function.\n ");
            exit(1);
        }
    }

    while ((K = getchar()) != EOF) {

        signed int check = code_lookup(string_table, prefix, (short) K);//LOOKUP
        
        if (check != -1) {
            prefix = check;
        } else {

            if (nstage != 3) {
                fprintf(stdout, "%i\n", prefix); //PRINT
            } else {
                bit_print(prefix, b);
            }
            
            
             
            if ((p) && (new_code == max_code)) { //PRUNE
                if (dbgprint) print_ht(string_table, F, max_code, (char) 1);
                string_table = enprune(string_table, prune_valid, max_code, &new_code, &prefix);
                if (prune_valid) free(prune_valid);
                prune_valid = (signed int *) calloc(max_code - 256, sizeof(signed int));
                if (!(prune_valid)) {
                    fprintf(stderr, "ERROR: Failed to allocate memory for prune list. Exiting function.\n");
                    exit(1);
                }
                if (b){
                    int reset = new_code;
                    int reset_bits = 0;
                    while (reset) {
                        reset_bits++;
                        reset = reset >> 1;
                    }
                    b->local_max = reset_bits;
                }
            }

            if ((p) && (prefix > 255)) { //VALIDATE
                if (!prune_valid[prefix - 256]) {
                    prune_valid[prefix - 256] = (signed int) 1;
                }
		    }

            if (new_code < max_code){ //ADD ENTRY
                entry *e = e_cons(prefix, new_code++, (short) K, NULL);
                entry_insert(string_table, e);   
            }

            if (b) {//update bits printed
                if (b->local_max < max_bit_length) {
                    int lm  = (0x01 << b->local_max);
                    if (new_code == lm) {
                       b->local_max += 1;
                    }
                }
            }

            prefix = code_lookup(string_table, (signed int) -1, (short) K);
            
        }
        C = K;
    }

    if (C != -1) {
        if (nstage != 3){
            fprintf(stdout,"%i\n", prefix);
        } else {
            bit_print(prefix, b);
            if (b->marker) putchar(b->buffer);
        }
    }

    if (dbgprint) {
        print_ht(string_table, F, new_code, (char) 0);
        fclose(F);
    }
    
    if (b) free(b);
    
    free_ht(string_table);
}