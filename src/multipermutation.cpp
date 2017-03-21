#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multipermutation.h"

struct multipermutation* multipermutation_calloc(int k, int *degrees) {
    struct multipermutation *mp;
	mp = (struct multipermutation *)malloc(sizeof *mp);
    if (mp== NULL) {
        mg_error("Error allocating alignment. Not enough memory?");
        mg_quit(EXIT_FAILURE);
    }
    mp->k = k;
    mp->perms = (struct permutation**)
        malloc((k-1) * sizeof(struct permutation*));
    if (mp->perms == NULL) { mg_error("Error allocating alignment. Not enough memory?"); mg_quit(EXIT_FAILURE); }
    int i;
    for (i=0;i<k-1;i++) {
        mp->perms[i] = permutation_calloc(degrees[i+1]);
    }
    return mp;
}

void multipermutation_randomize(struct multipermutation* mp) {
    int i;
    for(i=0;i<mp->k-1;i++) {
        permutation_randomize(mp->perms[i]);
    }
}
void multipermutation_delete(struct multipermutation* mp) {
    int i;
    for(i=0;i<mp->k-1;i++) {
        permutation_delete(mp->perms[i]);
    }
    free(mp->perms);
    free(mp);   
}
void multipermutation_print(struct multipermutation* mp) {
    printf("mp_start\n");
    int i;    
    for(i=0;i<mp->k-1;i++) {
        permutation_print(mp->perms[i]);
    }
    printf("mp_end\n");
}

