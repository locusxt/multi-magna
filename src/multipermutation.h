#ifndef _MULTIPERMUTATION_
#define _MULTIPERMUTATION_

#define SHORT

#include "carrier.h"
#include "permutation.h"

struct multipermutation {
    struct permutation **perms; // there are k-1 permutations
    int k; // the number of networks
};

struct multipermutation* multipermutation_calloc(int k, int *degrees);
void multipermutation_randomize(struct multipermutation* mp);
void multipermutation_delete(struct multipermutation* mp);
void multipermutation_print(struct multipermutation* mp);

#endif
