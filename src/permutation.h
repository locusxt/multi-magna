#ifndef _PERMUTATION_
#define _PERMUTATION_

#define SHORT

#include "carrier.h"

struct permutation {
	int* sequence;
	int degree;
#ifdef MAGNA_MUTATION
    int *ec_map; // edge conservation map
#endif
};

struct cycle_representation {
	int* cycle_data;
	int degree;
};

struct tensor_aux_space {
	struct permutation** paths;
	struct cycle_representation** crs;
	struct permutation** half_paths;
    int k;
};

struct permutation* permutation_calloc(int degree);
void permutation_randomize(struct permutation* p);
void permutation_delete(struct permutation* p);

void cycle_representation_delete(struct cycle_representation* cr);
int cycle_decomposition(struct cycle_representation* cr, struct permutation* p);
void cycle_print(struct cycle_representation* cr);

void permutation_print(struct permutation* p);
int random_range(int bound);
void swap(int* x, int* y);
void knuth_shuffle(struct permutation* p);
int mod(int a, int b);

int product(struct permutation* r, struct permutation* p, struct permutation* q);
int right_division(struct permutation* r, struct permutation* p, struct permutation* q);
int inverse(struct permutation* q, struct permutation* p);
int half_cycle(struct permutation* p, struct cycle_representation* cr, int cross_point);

int intersection_count(struct permutation* p, struct permutation* q);
void tensor(struct permutation** rs, struct permutation** ps, struct permutation** qs, struct tensor_aux_space* aux);
int evaluate(struct permutation* p, int x);

struct tensor_aux_space* tensor_aux_space_malloc(int* degrees, int k);
struct cycle_representation* cycle_representation_calloc(int degree);
void tensor_aux_space_delete(struct tensor_aux_space* aux);

#endif
