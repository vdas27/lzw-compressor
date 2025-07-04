#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"

Kstack *create_stack(short C) {
    Kstack *k = malloc(sizeof(Kstack));
    if (!(k)) {
        fprintf(stderr, "ERROR: Failed to allocate memory for KStack. Exiting function.\n");
        exit(1);
    }
    k->C = C;
    k->below = NULL;
    return k;
}

Kstack *pop(Kstack *k) {
    Kstack *a;
    char K = (char) k->C;
    putchar(K);
    if (k->below){
        a = k->below;
    } else {
        a = NULL;
    }
    free(k);
    return a;
}

Kstack *push(Kstack *k, short C) {
    Kstack *a = malloc(sizeof(Kstack));
    if (!(a)) {
        fprintf(stderr, "ERROR: Failed to allocate memory for KStack. Exiting function. \n");
        exit(1);
    }
    a->C = C;
    a->below = k;
    return a;
}

array_entry *initialize_string_table_array(int max_bit_length) {
    signed int max_code = (signed int) (0x1 << max_bit_length);
    array_entry *string_table = (array_entry *)calloc(max_code, sizeof(array_entry));
    if (!(string_table)) {
        fprintf(stderr, "ERROR: Failed to allocate memory for string table. Exiting function. \n");
        exit(1);
    }
    for (int i = 0; i < 256; i++) {
        string_table[i].prefix = (signed int) -1;
        string_table[i].C = (short) i;
        string_table[i].active = (char) 1;
    }
    return string_table;
}

void insert_entry(array_entry *st, signed int code, signed int prefix, short C, char active) {
    st[code].active = active;
    st[code].prefix = prefix;
    st[code].C = C;
    return;
}

signed int getcode(reader *r) {
    signed int C = 0, D;
    if (r) {
        int m = r->local_max;

        if (r->marker) {
            m -= r->marker;
            C = r->leftover << m;
        }

        while (m > 8) {
            if ((D = getchar()) == EOF) return EOF;
            m -= 8;
            D = D << m;
            C = C ^ D;
        }

        if (m) {
            if ((D = getchar()) == EOF) return EOF;
            r->leftover = D & (0xFF >> m);
            r->marker = (8 - m);
            D = D >> r->marker;
            C = C ^ D;
        }

        if (C == EOF) {
            return EOF;
        } else return C;

    } else {
        
        if (fscanf(stdin, "%i", &C) == 1) {
            return C;
        } else return EOF;
    }
    
}

void print_st(array_entry *st, FILE *F, signed int max_code, char p) {

    if (p) {
        fprintf(F,"\nPre-prune table: \n");
    } else {
        fprintf(F,"\nFinal table: \n");
    }
    
    fprintf(F, "Code:    Prefix  C\n");
    for (int i = 256; i < max_code; i++) {
        if (st[i].active){
            fprintf(F, "  %i:    %i       %c\n",i, st[i].prefix, (char) st[i].C);
        } else i = max_code;
    }
}

array_entry *deprune(array_entry *st, int max_bits, signed int *new_code, signed int *newC) {
    signed int max_code, prefix;
    max_code = (signed int) (0x1 << max_bits);
    array_entry *new = initialize_string_table_array(max_bits);
    *new_code = 256;
    
    signed int *updated_codes = (signed int *) calloc(max_code - 256, sizeof(signed int));
    if (!updated_codes) {
        fprintf(stderr, "ERROR: Failed to allocate memory for code list. Exiting function. \n");
        exit(1);
    }

    for (int i = 256; i < max_code; i++) {
        if (st[i].active == 2) {
            if (st[i].prefix > 255) {
                prefix = updated_codes[st[i].prefix - 256];
            } else {
                prefix = st[i].prefix;
            }
            insert_entry(new, *new_code, prefix, st[i].C, (char) 1);
            
            updated_codes[i - 256] = *new_code;
            *new_code += 1;
        }
    }

    if (*newC > 255) {
        *newC = updated_codes[*newC - 256];
    }

    free(updated_codes);
    free(st);

    return new;

}

void decode() {
    
    int max_bits = 12, p = 0;
    const char *STAGE = "STAGE";
    const char *stage = getenv(STAGE);
    int nstage = 3;
    if (stage) nstage = atoi(stage);
    reader *r = NULL;


    if (nstage) {
        if (nstage == 2) {
            fscanf(stdin, "%i", &max_bits);
            fscanf(stdin, "%i", &p);
        } else if (nstage == 3) {
            max_bits = getchar();
            p = getchar();
            r = malloc(sizeof(reader));
            if (!r) {
                fprintf(stderr, "ERROR: Failed to allocate memory for reader. Exiting function. \n");
                exit(1);
            }
            r->local_max = 9;
            r->marker = 0;
            r->leftover = 0x00;
        }
    }

    const char *DBG = "DBG";
    const char *value = getenv(DBG);
    int dbgprint;
    FILE *F;
    if (value) dbgprint = atoi(value);
    if (dbgprint) {
        F = fopen("DBG.decode", "w");
    }


    signed int C, oldC, newC, new_code, max_code;
    char finalK;

    array_entry *st = initialize_string_table_array(max_bits);
    new_code = 256;
    max_code = (signed int) (0x1 << max_bits);
    oldC = -1;
    while ((newC = C = getcode(r)) != EOF) { //READS
        Kstack *ks = NULL;
        if (!(st[C].active)) {
            ks = create_stack((short) finalK);
            C = oldC;
        }

        while (st[C].prefix != -1) {
            if (!(ks)) {
                ks = create_stack(st[C].C);
            } else {
                ks = push(ks, st[C].C);
            }
            C = st[C].prefix;
        }

        finalK = (char) st[C].C; //LOOKUP
        putchar(finalK); //PRINT

        while (ks) {
            ks = pop(ks); //PRINT
        }

        if (oldC != -1) {
            
            if ((p) && (oldC > 255)) { //VALIDATE
                    st[oldC].active = 2;
            }

            if (new_code < max_code){ //ADD ENTRY
                insert_entry(st, new_code++, oldC, (short) finalK, (char) 1);
            }

            if (r) {//update bits printed
                if (r->local_max < max_bits) {
                    int lm  = (0x01 << r->local_max);
                    if (new_code == lm - 1) {
                        r->local_max += 1;
                    }
                }
            }

            if ((p) && (new_code == max_code)) { //PRUNE

                if (dbgprint) print_st(st, F, max_code, (char) 1);
                st = deprune(st, max_bits, &new_code, &newC);
                if (r){
                    int reset = new_code;
                    int reset_bits = 0;
                    while (reset) {
                        reset_bits++;
                        reset = reset >> 1;
                    }
                    r->local_max = reset_bits;
                }
            }     
        }
        oldC = newC;
    }
    if (dbgprint) {
        print_st(st, F, max_code, (char) 0);
        fclose(F);
    }
    if (r) free(r);
    free(st);
}
