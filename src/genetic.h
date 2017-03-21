#ifndef _GENETIC_
#define _GENETIC_

#include "graph.h"
#include <pthread.h>

struct population {
    struct alignment** alignment_set;
    int pop_size;
    int pop_elite;
    int pop_temp;
    int *gs2orig;
    int *orig2gs;
    int n_threads;
    struct tensor_aux_space** tauxs;
    struct compute_aux_space **cauxs;
    struct parallel_compute_task *ctasks;
    struct parallel_tensor_task *ttasks;
    struct alignment **alignment_temp;
    long runtime;
};

struct population* population_malloc(struct carrier* rel, int pop_size, int pop_elite, struct graph** gs, int n_gs, int n_threads);
struct population* population_read(struct carrier* rel, int pop_size, int pop_elite, struct graph** gs, int n_gs, char* file_name, int n_threads);
struct population* population_random(struct carrier* rel, int pop_size, int pop_elite, struct graph** gs, int n_gs, int n_threads);
void population_sort(struct population* pop, struct carrier* rel);
void population_step_roulette(struct population* pop, struct carrier* rel, int cur_gen);
void run_simulation(char* graphs_file, char* init_pop_file, char* output_file_name, int rel_rel, int pop_size, int pop_elite, int n_gen, int freq, float alpha, char *node_sim_list_file, int n_threads);
void population_save_best_stats(int geni, struct population* pop, char* stats_file);
void population_save_best(struct population* pop, char* alignment_file);
void population_delete(struct population* pop, struct carrier *rel);

void swap_align(struct alignment** a1, struct alignment** a2);
void insertion_sort(struct alignment** alignment_set, struct carrier* rel, int first, int last);
int partition(struct alignment** alignment_set, struct carrier* rel, int first, int last);
void sub_quick_locally_insertion(struct alignment** alignment_set, struct carrier* rel, int first, int last);
void quick_locally_insertion(struct alignment** alignment_set, struct carrier* rel, int n);
int roulette(struct alignment** alignment_set, int n, float total_weight);

/* Parallel interface */
struct parallel_compute_task {
	struct population* pop;
	struct carrier* rel;
	int index;
	//long total_time;
    pthread_t thread;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int state; // for cond
};

struct parallel_tensor_task {
	struct population* pop;
	float pop_weight;
	int index;
    //int cur_gen;
    pthread_t thread;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int state; // for cond
};

void* slice_alignment_compute(void* task);
void* slice_alignment_tensor(void* task);

#endif
