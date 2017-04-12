#ifndef _MAGNA_GRAPH_
#define _MAGNA_GRAPH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "multipermutation.h"
#include "genetic.h"
#include <algorithm> // std::min

#ifndef _bms_
#define _bms_
extern float base_my_score;
#endif

// uncompressed sparse graph format (yale)
struct spmat {
    int nr;
    int nc; // num. rows, cols
    // int *A; //values
    int *IA; // ind first elt of each row. nnz = IA[nr]
    int *JA; // col inds
    int *INZ; // nnz of each row
};

struct spmat spmat_malloc(int nr, int nnz);
struct spmat edge_set_to_spmat(struct edge **edge_set, int nk, int nnz);

struct vertex {
	char* name;
	int number;
};

struct vertex* vertex_malloc(const char* name, int number);
struct vertex* vertex_realloc(char* name, int number);
int vertex_cmp(struct vertex* v1, struct vertex* v2);
void vertex_print(struct vertex* v);
void vertex_delete(struct vertex* v);

struct edge {
	int vertex1;
	int vertex2;
};

struct edge* edge_malloc(int vertex1, int vertex2);
struct edge* edge_realloc(int vertex1, int vertex2);
void graph_edge_delete(struct graph* g);
int edge_cmp(struct edge* e1, struct edge* e2);
void edge_print(struct edge* e);
void edge_delete(struct edge* e);

struct graph {
	struct vertex** vertex_set_by_name;
	struct vertex** vertex_set_by_number;
	int n_vertices;
	int n_edges;
    struct edge** edge_set; // temp array for edge reading
    struct spmat sp; // final edge set as sparse matrix yale format    
};

struct graph* graph_malloc(void);
void graph_vertex_malloc(struct graph* g, int n_vertices);
void graph_edge_malloc(struct graph* g, int n_edges);

struct graph** graph_list_read(char* file_name, int *n_gs);
struct graph* graph_read(char* file_name);
void graph_edge_list_read(struct graph* g, FILE*input_file, char* file_name);
void graph_read_vertices(struct graph* g, FILE* input_file, char* file_name);
void graph_read_edges(struct graph* g, FILE* input_file, char* file_name);
void graph_sif_read(struct graph *g, FILE *input_file, char *file_name);

void graph_print_vertices(struct graph* g);
void graph_print_edges(struct graph* g);
void graph_sort_vertices(struct graph* g);
void graph_sort_edges(struct graph* g);
int graph_find_vertex_number(struct graph* g, char* name);
int graph_find_vertex_number_unsorted(struct graph* g, char* name);
char* graph_find_vertex_name(struct graph* g, int number);
int graph_find_edge(struct graph* g, int v1, int v2);
int graph_is_edge(struct graph* g, int vertex1, int vertex2);
void graph_delete(struct graph* g);

struct alignment {
    struct graph** networks;
    int n_networks;
    struct multipermutation* mp;
    struct multipermutation* invmp;
    //	int n_edges_preserved;
    //	int n_edges_induced;
    //	int n_edges_unique;
    float score, edge_score, node_score;
    float s3_score;
    int is_computed;
    float my_score;
};

struct compute_aux_space {
//    SpMat cg; // composite graphs
//    std::vector<EigTriplet> cg_triplets;
    //    int *v_weights;  // vector of "cluster sizes"
    int *spA; // values
    int *spIA; // ind first elt of each row
    int *spJA; // col inds
    int *spJR; // col inds
    int *spJC; // row inds
    int *spINZ; // nnz of each row
    int *spWI; // for the accum. could be done w/ just spINZ
    int *v_weights;
    struct permutation *perm0, *permtmp;    
};

struct alignment* alignment_calloc(struct graph** gs, int n_gs, int use_alpha);
void alignment_randomize(struct alignment* a);
int alignment_read(struct alignment* a, char* file_name);
void alignment_tensor(struct alignment* a3, struct alignment* a1, struct alignment* a2, struct tensor_aux_space* taux);
int alignment_compare(struct alignment* a1, struct alignment* a2, struct carrier* rel);
int alignment_compare_ec(const void* a1, const void* a2);
int alignment_compare_ics(const void* a1, const void* a2);
int alignment_compare_3s(const void* a1, const void* a2);
//void alignment_update_inverse(struct alignment* a);
void alignment_write(struct population *pop, struct alignment* a, char* file_name);
void alignment_print(struct alignment* a);
void alignment_delete(struct alignment* a, int use_alpha);
float alignment_nodescore_compute(struct alignment *a, struct carrier *rel);
float alignment_node_correctness(struct alignment *a);
// float alignment_conserved_interaction_score(SpMat G, int *v_weights);
// void alignment_composite_graph(struct alignment *a, int **v_weights,
//                                compute_aux_space *caux);
void alignment_compute(struct alignment* a, struct carrier* rel,
                       compute_aux_space *caux);

struct compute_aux_space* compute_aux_space_malloc(graph **gs, int ng);
void compute_aux_space_delete(struct compute_aux_space* aux);

#endif
