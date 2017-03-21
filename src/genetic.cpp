#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <libgen.h>
#include <string.h>
#include <vector>
#include "genetic.h"

#define DEBUG_CROSS 0
#define SORT 1

struct population* population_malloc(struct carrier* rel, int pop_size, int pop_elite, struct graph** gs, int n_gs, int n_threads) {
    struct population* pop = (struct population*)malloc(sizeof *pop);
    int pop_temp = pop_size - pop_elite;
    struct alignment** alignment_set = (struct alignment**)malloc((pop_size + pop_temp) * (sizeof *alignment_set));
    struct alignment** alignment_temp = (struct alignment**)malloc( pop_temp * (sizeof *alignment_temp));
    
    int i,ret;
    struct tensor_aux_space **tauxs;
    tauxs = (struct tensor_aux_space **)malloc(n_threads*(sizeof *tauxs));	
    struct compute_aux_space **cauxs;
    cauxs = (struct compute_aux_space **)malloc(n_threads*(sizeof *cauxs));	
    
    if (!pop || !alignment_set || !alignment_temp || !tauxs) {
        mg_error("Error allocating alignment population. Not enough memory?");
        mg_quit(EXIT_FAILURE);
    }
    
    /* Allocate memory for alignments */
    for (i = 0; i < pop_size + pop_temp; i++)
        alignment_set[i] = alignment_calloc(gs, n_gs, rel->use_alpha);
	
    /* Allocate tensor auxiliary space */
    int *degrees = (int *)malloc(n_gs*sizeof(int));
    if(degrees==NULL){mg_error("Allocation error.");mg_quit(EXIT_FAILURE);}        
    for(i=0;i<n_gs;i++) { degrees[i] = gs[i]->n_vertices; }        
    for (i = 0; i < n_threads; i++) tauxs[i] = tensor_aux_space_malloc(degrees,n_gs);
    for (i = 0; i < n_threads; i++) cauxs[i] = compute_aux_space_malloc(gs,n_gs);
    
    pop->alignment_set = alignment_set;
    pop->alignment_temp = alignment_temp;
    pop->tauxs = tauxs;
    pop->cauxs = cauxs;
    pop->pop_size = pop_size;
    pop->pop_elite = pop_elite;
    pop->pop_temp = pop_temp;
    pop->n_threads = n_threads;
    pop->runtime = 0;
    if (n_threads>1) {
        struct parallel_compute_task *ctasks;
        struct parallel_tensor_task *ttasks;
        ctasks = (struct parallel_compute_task*)malloc(n_threads*sizeof(struct parallel_compute_task));
        ttasks = (struct parallel_tensor_task*)malloc(n_threads*sizeof(struct parallel_tensor_task));     
        if (!ctasks || !ttasks) {
            mg_error("Error allocating alignment population. Not enough memory?");
            mg_quit(EXIT_FAILURE);
        }        
        for (i = 0; i < n_threads; i++) {
            ctasks[i].pop = pop;
            ctasks[i].rel = rel;
            ctasks[i].index = i;        
            //ctasks[i].cond = PTHREAD_COND_INITIALIZER;
            //ctasks[i].mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_init(&ctasks[i].cond,NULL);
            pthread_mutex_init(&ctasks[i].mutex,NULL);
            ctasks[i].state = 0;
        }
        for (i = 0; i < n_threads; i++) {
            ttasks[i].pop = pop;
            ttasks[i].index = i;
            //ttasks[i].cond = PTHREAD_COND_INITIALIZER;
            //ttasks[i].mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_init(&ttasks[i].cond,NULL);
            pthread_mutex_init(&ttasks[i].mutex,NULL);
            ttasks[i].state = 0;
        }        
        pop->ctasks = ctasks;
        pop->ttasks = ttasks;
        for (i = 0; i < n_threads; i++) {
            ret = pthread_create(&ctasks[i].thread, NULL, slice_alignment_compute, (void*)(ctasks+i));
            if (ret) { mg_error("Error creating main thread. Too many threads?"); mg_quit(EXIT_FAILURE); }
            pthread_detach(ctasks[i].thread);
        }
        for (i = 0; i < n_threads; i++) {
            ret = pthread_create(&ttasks[i].thread, NULL, slice_alignment_tensor, (void*)(ttasks+i));
            if (ret) { mg_error("Error creating main thread. Too many threads?"); mg_quit(EXIT_FAILURE); }
            pthread_detach(ttasks[i].thread);
        }
    }	
    free(degrees);
	
    return pop;
}


void population_delete(struct population* pop, struct carrier* rel) {
    int pop_size = pop->pop_size;
    int pop_temp = pop->pop_temp;
    int n_threads = pop->n_threads;
	
    struct alignment** alignment_set = pop->alignment_set;
    int i;
    for (i = 0; i < pop_size + pop_temp; i++)
        if (alignment_set[i] != NULL)
            alignment_delete(alignment_set[i], rel->use_alpha);
    struct tensor_aux_space** tauxs = pop->tauxs;
    struct compute_aux_space **cauxs = pop->cauxs;
    for (i = 0; i < n_threads; i++)
        if (tauxs[i] != NULL)
            tensor_aux_space_delete(tauxs[i]);
    for (i = 0; i < n_threads; i++)
        if (cauxs[i] != NULL)
            compute_aux_space_delete(cauxs[i]);
    free(pop->alignment_set);
    free(pop->alignment_temp);
    free(pop->gs2orig);
    free(pop->orig2gs);
    free(tauxs);
    free(cauxs);
    if(n_threads>1) {
        for(i=0;i<n_threads;i++) {
            pthread_cond_destroy(&pop->ctasks[i].cond);
            pthread_cond_destroy(&pop->ttasks[i].cond);
            pthread_mutex_destroy(&pop->ctasks[i].mutex);
            pthread_mutex_destroy(&pop->ttasks[i].mutex);
        }
        free(pop->ctasks);
        free(pop->ttasks);
    }        
    free(pop);
}

struct population* population_read(struct carrier* rel, int pop_size, int pop_elite, struct graph** gs, int n_gs, char* file_name, int n_threads) {
	FILE* input_file = fopen(file_name, "r");
	if (input_file == NULL) {
		mg_error("Error reading population file: %s", file_name);
		mg_quit(EXIT_FAILURE);
	}
	
	printf("Reading alignments from: %s\n", file_name);
	
	struct population* pop = population_malloc(rel, pop_size, pop_elite, gs, n_gs, n_threads);
	struct alignment** alignment_set = pop->alignment_set;

    char *strtmp1;
    strtmp1 = strdup(file_name);
    char *input_file_dir = dirname(strtmp1);
	
	// Read in alignments
	int i;
	char aln_file_name[512];
    char aln_full_file_name[1024];
	struct alignment* a;
	for (i = 0; i < pop_size; i++) {
		if (fgets(aln_file_name, sizeof aln_file_name, input_file) == NULL)
			break;
		// get rid of newline character
//		int len = strlen(aln_file_name) - 1;
//		if (aln_file_name[len] == '\n')	aln_file_name[len] = '\0';
//        len = strlen(aln_file_name) - 1;
//		if (aln_file_name[len] == '\r')	aln_file_name[len] = '\0';        

        sprintf(aln_full_file_name, "%s/%s", input_file_dir, trimwhitespace(aln_file_name));
		
		a = alignment_set[i];
		alignment_read(a, aln_full_file_name);
	}

    free(strtmp1);
	
	// Randomize remaining
	int j;
	for (j = i; j < pop_size; j++)
		alignment_randomize(alignment_set[j]);
	
	pop->alignment_set = alignment_set;
	pop->pop_size = pop_size;
	pop->pop_elite = pop_elite;
		
	population_sort(pop, rel);
	
	fclose(input_file);
		
	return pop;
}

struct population* population_random(struct carrier* rel, int pop_size, int pop_elite, struct graph** gs, int n_gs, int n_threads) {
	struct population* pop = population_malloc(rel, pop_size, pop_elite, gs, n_gs, n_threads);
	struct alignment** alignment_set = pop->alignment_set;
	
	int i;
	for (i = 0; i < pop_size; i++)
		alignment_randomize(alignment_set[i]);
		
	population_sort(pop, rel);
	
	return pop;
}

// For every alignment,
// for every permutation in the alignment,
// with prob. mutation_prob,
// swap two random indices
void population_mutate(struct population* pop, struct carrier* rel) {
    float mutation_prob = 0.01;
    struct alignment** alignment_set = pop->alignment_set;
    int pop_size = pop->pop_size;
    int i,j,r,s,k,l,x,y,tmp,degree,*sequence;
    float u;
    for (i = 0; i < pop_size; i++) {
        struct alignment* a = alignment_set[i];
        struct permutation** perms = a->mp->perms;
        k = a->mp->k;
        u = ((float)rand()/(float)RAND_MAX);
        if (u<mutation_prob) {
            a->is_computed = 0;
            for(l=0;l<k-1;l++) {
                degree = perms[l]->degree;
                sequence = perms[l]->sequence;
                u = ((float)rand()/(float)RAND_MAX);
                if(u<1/(k-1)) {
                    x = rand() % degree;
                    y = rand() % degree;
                    tmp = sequence[x];
                    sequence[x] = sequence[y];
                    sequence[y] = tmp;
                }
                /*
                r = int(sqrt(degree));
                for(j=0;j<=r;j++) {
                    u = ((float)rand()/(float)RAND_MAX);
                    if(u<1/r) {
                        x = rand() % degree;
                        y = rand() % degree;
                        tmp = sequence[x];
                        sequence[x] = sequence[y];
                        sequence[y] = tmp;
                    }
                }
                */
            }
        }
    }
}

void population_sort(struct population* pop, struct carrier* rel) {
    int pop_size = pop->pop_size;
    int n_threads = pop->n_threads;
    struct alignment** alignment_set = pop->alignment_set;

    int ret;
    int i;
    if (n_threads == 1) {
        for (i = 0; i < pop_size; i++) {
            struct alignment* a = alignment_set[i];
            if (a->is_computed == 0)
                alignment_compute(a, rel, pop->cauxs[0]);
        }
    } else if (n_threads >= 1) {
        //pthread_t threads[n_threads];
        //struct parallel_compute_task *tasks;
        //tasks = (struct parallel_compute_task*)
        //    malloc(n_threads*sizeof(struct parallel_compute_task)); 
        //if (tasks == NULL) { mg_error("Allocation error."); mg_quit(EXIT_FAILURE); }               
        struct parallel_compute_task *tasks = pop->ctasks;
		
        for (i = 0; i < n_threads; i++) {
            //tasks[i].pop = pop;
            //tasks[i].rel = rel;
            //tasks[i].index = i;
            //ret = pthread_create(threads+i, NULL, slice_alignment_compute, (void*)(tasks+i));
            //if (ret != 0) {
            //    mg_error( "Error creating thread %d. Too many threads?", i);
            //    mg_quit(EXIT_FAILURE);
            //}
            pthread_mutex_lock(&tasks[i].mutex);
            tasks[i].state = 1;
            pthread_cond_signal(&tasks[i].cond);
            pthread_mutex_unlock(&tasks[i].mutex);            
        }
        
        for (i = 0; i < n_threads; i++) {
            pthread_mutex_lock(&tasks[i].mutex);
            while (tasks[i].state!=0)
                pthread_cond_wait(&tasks[i].cond, &tasks[i].mutex);
            pthread_mutex_unlock(&tasks[i].mutex);
        }
        //for (i = 0; i < n_threads; i++) pthread_join(threads[i], NULL);
        //free(tasks);
    }

    /*
      if (SORT == 0) {
      if (rel->rel == 0)
      qsort(alignment_set, pop_size, sizeof(struct alignment*), alignment_compare_ec);
      else if (rel->rel == 1)
      qsort(alignment_set, pop_size, sizeof(struct alignment*), alignment_compare_ics);
      else if (rel->rel == 2)
      qsort(alignment_set, pop_size, sizeof(struct alignment*), alignment_compare_3s);
      }
      else if (SORT == 1) */
    quick_locally_insertion(alignment_set, rel, pop_size);
}

void population_print(struct population* pop) {
    struct alignment** alignment_set = pop->alignment_set;
    int pop_size = pop->pop_size;
    int pop_temp = pop->pop_temp;
    int i;
    for (i = 0; i < pop_size + pop_temp; i++)
        printf("EC %p %d: %f\n", alignment_set[i], i, alignment_set[i]->edge_score);
}

void population_step_roulette(struct population* pop, struct carrier* rel, int cur_gen) {
    int pop_size = pop->pop_size;
    int pop_elite = pop->pop_elite;
    int pop_temp = pop->pop_temp;
    int n_threads = pop->n_threads;
    struct alignment** alignment_set = pop->alignment_set;
		
    /* Compute the weight of the population */
    int i;
    float pop_weight = 0;
    for (i = 0; i < pop_size; i++)
        pop_weight += alignment_set[i]->score;
	
    /* The roulette */
    if (n_threads == 1) {
        int crosses = 0;
        struct tensor_aux_space** tauxs = pop->tauxs;
        while (crosses < pop_temp) {
            int indiv1 = roulette(alignment_set, pop_size, pop_weight);
            int indiv2 = indiv1;
            while (indiv2 == indiv1)
                indiv2 = roulette(alignment_set, pop_size, pop_weight);
					
            struct alignment* a1 = alignment_set[indiv1];
            struct alignment* a2 = alignment_set[indiv2];
            struct alignment* a3 = alignment_set[pop_size + crosses];
		
            alignment_tensor(a3, a1, a2, tauxs[0]);
            a3->is_computed = 0;
			
            crosses += 1;
        }
    } else if (n_threads > 1) {
        //pthread_t threads[n_threads];
        //struct parallel_tensor_task *tasks;
        //tasks = (struct parallel_tensor_task*)
        //    malloc(n_threads*sizeof(struct parallel_tensor_task)); 
        //if (tasks == NULL) { mg_error("Allocation error."); mg_quit(EXIT_FAILURE); }       
        struct parallel_tensor_task *tasks = pop->ttasks;

        for (i = 0; i < n_threads; i++) {
            //tasks[i].pop = pop;
            //tasks[i].index = i;
            tasks[i].pop_weight = pop_weight;
            //tasks[i].cur_gen = cur_gen;
            pthread_mutex_lock(&tasks[i].mutex);
            tasks[i].state = 1;
            pthread_cond_signal(&tasks[i].cond);
            pthread_mutex_unlock(&tasks[i].mutex);            
            //pthread_create(threads+i, NULL, slice_alignment_tensor, (void*)(tasks+i));
        }
		
        for (i = 0; i < n_threads; i++) {
            pthread_mutex_lock(&tasks[i].mutex);
            while (tasks[i].state!=0)
                pthread_cond_wait(&tasks[i].cond, &tasks[i].mutex);
            pthread_mutex_unlock(&tasks[i].mutex);
        }
        //for (i = 0; i < n_threads; i++)	pthread_join(threads[i], NULL);
        //free(tasks);
    }

    /* Swap temp with new members */
    struct alignment** alignment_temp = pop->alignment_temp;
    memcpy(alignment_temp, alignment_set + pop_size, pop_temp*(sizeof *alignment_set));
    memcpy(alignment_set + pop_size, alignment_set + pop_elite, pop_temp*(sizeof *alignment_set));
    memcpy(alignment_set + pop_elite, alignment_temp, pop_temp*(sizeof *alignment_set));
	
    population_mutate(pop, rel);
    population_sort(pop, rel);
	
    if (DEBUG_CROSS == 1)
        population_print(pop);
}

void* slice_alignment_tensor(void *p) {
    struct parallel_tensor_task* task = (struct parallel_tensor_task*)p;    
	struct population* pop = task->pop;
	int index = task->index;
	struct alignment** alignment_set = pop->alignment_set;
	struct tensor_aux_space** tauxs = pop->tauxs;
	int n_threads = pop->n_threads;
	int pop_size = pop->pop_size;
	int pop_temp = pop->pop_temp;
    srand(((int)time(0)) ^ index);	
	int i;

    while(1) {
        pthread_mutex_lock(&task->mutex);
        while (task->state!=1)
            pthread_cond_wait(&task->cond, &task->mutex);
        pthread_mutex_unlock(&task->mutex);        

        float pop_weight = task->pop_weight;
    
        for (i = pop_size + index; i < pop_size + pop_temp; i += n_threads) {
            int indiv1 = roulette(alignment_set, pop_size, pop_weight);
            int indiv2 = indiv1;
            while (indiv2 == indiv1)
                indiv2 = roulette(alignment_set, pop_size, pop_weight);
		
            struct alignment* a1 = alignment_set[indiv1];
            struct alignment* a2 = alignment_set[indiv2];
            struct alignment* a3 = alignment_set[i];
		
            alignment_tensor(a3, a1, a2, tauxs[index]);
            a3->is_computed = 0;
        }

        pthread_mutex_lock(&task->mutex);
        task->state = 0;
        pthread_cond_signal(&task->cond);
        pthread_mutex_unlock(&task->mutex);
    }    
	
	return 0;
}
    
void* slice_alignment_compute(void*p) {
    struct parallel_compute_task* task = (struct parallel_compute_task*)p;
	struct population* pop = task->pop;
	struct carrier* rel = task->rel;
	int index = task->index;
	struct alignment** alignment_set = pop->alignment_set;
	int n_threads = pop->n_threads;
	int pop_size = pop->pop_size;
    int pop_temp = pop->pop_temp;    
	int i;

    while(1) {
        pthread_mutex_lock(&task->mutex);
        while (task->state!=1)
            pthread_cond_wait(&task->cond, &task->mutex);
        pthread_mutex_unlock(&task->mutex);
        

        for (i = index; i < pop_size + pop_temp; i += n_threads) {
            if (i >= pop_size) break;
            struct alignment* a = alignment_set[i];
            if (a->is_computed == 0)
                alignment_compute(a, rel, pop->cauxs[index]);
        }

        pthread_mutex_lock(&task->mutex);
        task->state = 0;
        pthread_cond_signal(&task->cond);
        pthread_mutex_unlock(&task->mutex);
    }    
	return 0;
}

typedef std::pair<int,int> intpair;
bool intpaircomp ( const intpair& l, const intpair& r) { return l.second < r.second; }

void run_simulation(char* graphs_file, char* init_pop_file, char* output_file_name, int rel_rel, int pop_size, int pop_elite, int n_gen, int freq, float alpha, char *node_sim_list_file, int n_threads) {

    struct graph **gsorig,**gs;
    int ngs;
    struct carrier *rel=NULL;
    struct population* pop=NULL;    
    
    long starttime = (long)time(0);
    srand(starttime);

    gsorig = graph_list_read(graphs_file, &ngs);
    if (ngs < 2) mg_error("Need at least 2 networks for alignment.");
    gs = (struct graph**)malloc(ngs*sizeof(struct graph*));
    if(!gs) mg_error("Allocation error.");

    // sort the networks by the number of degrees
    int i;
    std::vector<intpair> degrees(ngs);
    for (i=0; i<ngs; i++) {
        degrees[i].first = i;
        degrees[i].second = gsorig[i]->n_vertices;
    }
    std::sort (degrees.begin(), degrees.end(), intpaircomp);

    int *gs2orig = (int*)malloc(ngs*sizeof(int));
    if (gs2orig==NULL) mg_error("Allocation error");    
    int *orig2gs = (int*)malloc(ngs*sizeof(int));
    if (orig2gs==NULL) mg_error("Allocation error");    
    for (i=0; i<ngs; i++) orig2gs[i] = degrees[i].first; // map from original index to current index
    for (i=0; i<ngs; i++) gs2orig[orig2gs[i]] = i; // map from current index to original index
    for (i=0; i<ngs; i++) gs[i] = gsorig[orig2gs[i]];

    rel = carrier_create(gsorig, gs2orig, gs, ngs, rel_rel, alpha, node_sim_list_file);

    // Random or initialize?
    if (init_pop_file == NULL) pop = population_random(rel, pop_size, pop_elite, gs, ngs, n_threads);
    else pop = population_read(rel, pop_size, pop_elite, gs, ngs, init_pop_file, n_threads);

    pop->gs2orig = gs2orig;
    pop->orig2gs = orig2gs;
	
    // Optimizing what?
    char opt_str[10];
    if (rel->rel == 0)
        sprintf(opt_str, "EC");
    else if (rel->rel == 1)
        sprintf(opt_str, "CIQ");
    else if (rel->rel == 2)
        sprintf(opt_str, "S3");
	
    int period = n_gen / freq;
	
    // save 0th generation
    char alignment_file[256];
    sprintf(alignment_file, "%s_%s_%d_%d_%d.txt", output_file_name, opt_str, pop_size, n_gen, 0);
    population_save_best(pop, alignment_file);
    //    population_save_best(pop, NULL);    
    sprintf(alignment_file, "%s_%s_%d_%d_stats.txt", output_file_name, opt_str, pop_size, n_gen);
    population_save_best_stats(0, pop, alignment_file);   
	
    // run, save stats with each generation
    for (i = 0; i < n_gen; i++) {
        if (DEBUG_CROSS == 1) printf("GENERATION %d\n", i);
        population_step_roulette(pop, rel,i+1);
        pop->runtime = time(0) - starttime;
        if ((i+1) % period == 0) {
            sprintf(alignment_file, "%s_%s_%d_%d_%d.txt", output_file_name, opt_str, pop_size, n_gen, i+1);
            population_save_best(pop, alignment_file);
            //        population_save_best(pop, NULL);
            sprintf(alignment_file, "%s_%s_%d_%d_stats.txt", output_file_name, opt_str, pop_size, n_gen);
            population_save_best_stats(i+1, pop, alignment_file);
        }
    }

    population_delete(pop,rel);
    carrier_delete(rel,ngs);
    for (i=0; i<ngs; i++) graph_delete(gsorig[i]);
    free(gs);
    free(gsorig);
}

// Print the edge and node scores of the best alignment to the stats file
// on every some generation
void population_save_best_stats(int geni, struct population* pop, char* stats_file) {
	struct alignment* a = pop->alignment_set[0];
    printf("generation %d runtime %ld score %f edge_score %f node_score %f\n",
           geni,
           pop->runtime,
           a->score,
           a->edge_score,
           a->node_score);
    
    FILE* output_fd = NULL;    
    if (geni==0) output_fd = fopen(stats_file,"w");
    else output_fd = fopen(stats_file, "a");
    if (output_fd == NULL) {
        printf("population_save_best_stats: Couldn't open file: %s\n", stats_file);
        mg_quit(EXIT_FAILURE);
    }    

    fprintf(output_fd, "generation %d runtime %ld score %f edge_score %f node_score %f\n",
            geni,
            pop->runtime,
            a->score,
            a->edge_score,
            a->node_score);
    fclose(output_fd);
}    

void population_save_best(struct population* pop, char* alignment_file) {	
	struct alignment* best_alignment = pop->alignment_set[0];
	alignment_write(pop, best_alignment, alignment_file);
}

inline void swap_align(struct alignment** a1, struct alignment** a2) {
	struct alignment* temp = *a1;
	*a1 = *a2;
	*a2 = temp;
}

void insertion_sort(struct alignment** alignment_set, struct carrier* rel, int first, int last) {
	struct alignment** as = alignment_set;

	int i;
	// Move through interval
	for(i = first + 1; i < last; i++) {
		int j;
		// Move the next element in
		for(j = i; j > first; j--) {
			if (alignment_compare(as[j], as[j-1], rel) <= 0)
				swap_align(alignment_set + j, alignment_set + (j-1));
			// If no swaps made, element is in its place
			else break;
		}
	}
}

int partition(struct alignment** alignment_set, struct carrier* rel, int first, int last) {
	struct alignment** as = alignment_set;
	
	// Layout of interval: p xxxxx... xxx e
	// p - pivot, x - element, e - end
	int left = first + 1;
	int right = last - 1;
	
	// Keep going till left and right cross paths
	while (left < right) {
		
		// Move right index
		while ((right > first) && (alignment_compare(as[first], as[right], rel) <= 0))
			right--;
		// Pathological case: right reaches the beginning
		// In this case, no swaps needed
		if (right == first)
			return first;
		
		// Move left index
		// No segfaults here, by the magic of lazy evaluation
		while ((left < last) && (alignment_compare(as[left], as[first], rel) <= 0))
			left++;
		// Pathological case: left reaches the end
		// Move pivot to the end
		if (left == last) {
			swap_align(alignment_set + first, alignment_set + (last-1));
			return last - 1;
		}
		
		// Swap appropriate elements
		if (left < right)
			swap_align(alignment_set + left, alignment_set + right);
		
	}
	// Move pivot
	swap_align(alignment_set + first, alignment_set + right);
	return right;
}

void sub_quick_locally_insertion(struct alignment** alignment_set, struct carrier* rel, int first, int last) {
	struct alignment** as = alignment_set;
	
	// Trivially sorted
	if (last - first <= 1)
		return;
	// Swap two element if necessary
	else if (last - first <= 2) {
		if (alignment_compare(as[last-1], as[first], rel) <= 0)
			swap_align(alignment_set + first, alignment_set + (last-1));
	}
	// Call Insertion Sort on small enough interval
	else if (last - first <= 10)
		insertion_sort(alignment_set, rel, first, last);
	// Otherwise, partition the interval
	else {
		// Randomize the selected pivot,
		// no deliberately triggering worst-case performance!!!
		int randIndex = first + rand() % (last - first);
		swap_align(alignment_set + first, alignment_set + randIndex);
		
		// Create partitions
		int mid = partition(alignment_set, rel, first, last);
		
		// Recurse on the partitions
		sub_quick_locally_insertion(alignment_set, rel, first, mid);
		sub_quick_locally_insertion(alignment_set, rel, mid + 1, last);
	}
}

void quick_locally_insertion(struct alignment** alignment_set, struct carrier* rel, int n) {
	sub_quick_locally_insertion(alignment_set, rel, 0, n);
}

int roulette(struct alignment** alignment_set, int n, float total_weight) {
	float w = ((float)rand() / RAND_MAX) * (total_weight + n);
	float sum = 0;
	int i;
	for (i = 0; i < n; i++) {
		sum += alignment_set[i]->score + 1;
		if (sum > w)
			return i;
	}
	return n - 1;
}
