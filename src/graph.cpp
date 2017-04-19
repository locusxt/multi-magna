// Written by Vikram Saraph and Vipin Vijayan
#include <libgen.h>
#include <iostream>
#include <map>
#include "graph.h"

#include <string>
#include <cstring>
#include <cmath>

float base_my_score = 0.0;

// Graph
// Sort vertices by node name, dictionary order on names
// Sort edges by node number, dictionary order on pairs of integers

struct spmat spmat_malloc(int nr, int nnz) {
    struct spmat sp;
    sp.nr = nr;
    //sp.nc = nc;
    //sp.A = (int*)malloc(nnz*sizeof(int)); // final values
    sp.IA = (int*)malloc((nr+1)*sizeof(int)); // ind first elt of each row
    sp.JA = (int*)malloc(nnz*sizeof(int)); // final col inds
    sp.INZ = (int*)malloc(nr*sizeof(int)); // nnz of each row
    return sp;
}

void spmat_delete(struct spmat sp) {
    free(sp.IA);
    free(sp.JA);
    free(sp.INZ);
}

struct vertex* vertex_malloc(const char* name, int number) {
	struct vertex* v;
	v = (struct vertex*)malloc(sizeof *v);
    if(v==NULL){mg_error("Error allocating vertex. Not enough memory?");mg_quit(EXIT_FAILURE);}
	
	int len = strlen(name) + 1;
	v->name = (char*)malloc(len * sizeof(char));
    if(v->name==NULL){mg_error("Error allocating vertex name. Not enough memory?");mg_quit(EXIT_FAILURE);}
	strcpy(v->name, name);
	v->number = number;
	return v;
}

int vertex_cmp(struct vertex* v1, struct vertex* v2) {
	char* name1 = v1->name;
	char* name2 = v2->name;
	
	return strcmp(name1, name2);
}

void vertex_print(struct vertex* v) {
	printf("%s, %d\n", v->name, v->number);
}

void vertex_delete(struct vertex* v) {
	free(v->name);
	free(v);
}

struct edge* edge_malloc(int vertex1, int vertex2) {
	struct edge* e;
	e = (struct edge*)malloc(sizeof *e);
    if(e==NULL){mg_error("Error allocating edge. Not enough memory?");mg_quit(EXIT_FAILURE);}    
	
	e->vertex1 = vertex1;
	e->vertex2 = vertex2;
	
	return e;
}

void graph_edge_delete(struct graph* g) {
    int i;
    for (i = 0; i < g->n_edges; i++)
		edge_delete(g->edge_set[i]);
	free(g->edge_set);
    g->edge_set = NULL;
}

int edge_cmp(struct edge* e1, struct edge* e2) {
	if (e1->vertex1 > e2->vertex1)
		return 1;
	else if (e1->vertex1 == e2->vertex1) {
		if (e1->vertex2 == e2->vertex2)
			return 0;
		else if (e1->vertex2 > e2->vertex2)
			return 1;
	}
	return -1;	
}

void edge_print(struct edge* e) {
	printf("(%d, %d)\n", e->vertex1, e->vertex2);
}

void edge_delete(struct edge* e) {
	free(e);
}

struct graph* graph_malloc(void) {
	struct graph* g;
	g = (struct graph*)malloc(sizeof *g);
    if(g==NULL){mg_error("Error allocating graph. Not enough memory?");mg_quit(EXIT_FAILURE);}    
	
	g->vertex_set_by_name = NULL;
	g->n_vertices = 0;
	g->edge_set = NULL;
	g->n_edges = 0;
	
	return g;
}

void graph_vertex_malloc(struct graph* g, int n_vertices) {	
	struct vertex** vertex_set_by_name;
	struct vertex** vertex_set_by_number;
	vertex_set_by_name = (struct vertex**)malloc(n_vertices * (sizeof *vertex_set_by_name));
	vertex_set_by_number = (struct vertex**)malloc(n_vertices * (sizeof *vertex_set_by_name));

    if(vertex_set_by_name==NULL || vertex_set_by_number==NULL){
        mg_error("Error allocating vertex set by name/number. Not enough memory?"); mg_quit(EXIT_FAILURE);}    
		
	g->vertex_set_by_name = vertex_set_by_name;
	g->vertex_set_by_number = vertex_set_by_number;
	g->n_vertices = n_vertices;
}

void graph_vertex_realloc(struct graph* g, int n_vertices) {	
	struct vertex** vertex_set_by_name;
	struct vertex** vertex_set_by_number;
	vertex_set_by_name = (struct vertex**)realloc(g->vertex_set_by_name, n_vertices * (sizeof *vertex_set_by_name));
	vertex_set_by_number = (struct vertex**)realloc(g->vertex_set_by_number, n_vertices * (sizeof *vertex_set_by_name));

    if(vertex_set_by_name==NULL || vertex_set_by_number==NULL){
        mg_error("Error allocating vertex set by name/number. Not enough memory?"); mg_quit(EXIT_FAILURE);}    
		
	g->vertex_set_by_name = vertex_set_by_name;
	g->vertex_set_by_number = vertex_set_by_number;
	g->n_vertices = n_vertices;
}

void graph_edge_malloc(struct graph* g, int n_edges) {
	struct edge** edge_set;
	edge_set = (struct edge**) malloc(n_edges * (sizeof *edge_set));
    if(edge_set==NULL){mg_error("Error allocating edge set. Not enough memory?");mg_quit(EXIT_FAILURE);}    
	
	g->edge_set = edge_set;
	g->n_edges = n_edges;
}

void graph_edge_realloc(struct graph* g, int n_edges) {
	struct edge** edge_set;
	edge_set = (struct edge**)realloc(g->edge_set, n_edges * (sizeof *edge_set));
    if(edge_set==NULL){mg_error("Error allocating edge set. Not enough memory?");mg_quit(EXIT_FAILURE);}	
	g->edge_set = edge_set;
	g->n_edges = n_edges;
}

struct graph** graph_list_read(char* file_name, int *n_gs) {
    struct graph **gs;
    FILE *input_file = fopen(file_name, "r");
    if (input_file == NULL) { mg_error( "Couldn't open graph list file: %s", file_name); mg_quit(EXIT_FAILURE); }
    int k = 2;
    gs = (struct graph**)malloc(k * (sizeof (struct graph*)));
    if (gs == NULL) { mg_error( "Allocation error." ); mg_quit(EXIT_FAILURE); }
    int i = 0;
    char input_buffer[1024];
    char *tmpstr1, *tmpstr2=NULL, *line,*file_name_dir;
    tmpstr1 = strdup(file_name);
    file_name_dir = dirname(tmpstr1);
    while (fgets(input_buffer, sizeof input_buffer, input_file) != NULL) {
        tmpstr2 = strdup(input_buffer);
        line = trimwhitespace(tmpstr2);
        
        if (line==NULL || line[0]=='\0') continue;
        i++;
        if(i>k) {
            k *= 2;
            gs = (struct graph**)realloc(gs, k * (sizeof (struct graph*)));
            if (gs == NULL) { mg_error( "Allocation error." ); mg_quit(EXIT_FAILURE); }
        }
        sprintf(input_buffer, "%s/%s", file_name_dir, line);
        gs[i-1] = graph_read(input_buffer);
        free(tmpstr2);
    }
    fclose(input_file);
    free(tmpstr1);
    k = i;
    gs = (struct graph**)realloc(gs,k * (sizeof (struct graph*)));
    if (gs == NULL) { mg_error( "Allocation error." ); mg_quit(EXIT_FAILURE); }
    *n_gs = k;
    return gs;
}

struct graph* graph_read(char* file_name) {
	FILE* input_file = fopen(file_name, "r");
	if (input_file == NULL) {
		mg_error("Couldn't open network file: %s", file_name);
		mg_quit(EXIT_FAILURE);
	}
    
//	char input_buffer[16];
//	fgets(input_buffer, sizeof input_buffer, input_file);
//    rewind(input_file);

	struct graph* g = graph_malloc();
    
    //    if(strncmp(input_buffer,"LEDA.GRAPH",10)==0) {
    if(strcmp(".gw",file_name+strlen(file_name)-3) == 0) {
        graph_read_vertices(g, input_file, file_name);
        graph_read_edges(g, input_file, file_name);
    }
    else if(strcmp(".sif",file_name+strlen(file_name)-4) == 0) {
        graph_sif_read(g,input_file,file_name);
    }
    else { // assume it's a edge list file if it's not a LEDA graph
        graph_edge_list_read(g, input_file, file_name);
    }

	fclose(input_file);
	printf("Graph %s read. %d vertices, %d edges.\n", file_name, g->n_vertices, g->n_edges);
    //    graph_print_edges(g);
	return g;
}

void graph_edge_list_read(struct graph* g, FILE *input_file, char* file_name) {
    struct vertex* new_vertex;
    int n_alloc_edges = 1024;
    graph_edge_malloc(g,n_alloc_edges); g->n_edges = 0;

    std::map<std::string,int> verts;
    //std::vector<int> edges1,edges2;
    std::map<std::string,int>::iterator it;

    int ret;
    char input_buffer[1024];
    char str1[512], str2[512];
    int num1,num2;
    int i = 0;
    while(fgets(input_buffer, sizeof input_buffer, input_file)) {
        //        if (input_buffer[0]=='#' || input_buffer[0]=='\0') continue;
        ret = sscanf(input_buffer, " %s ", str1);
        if (ret!=1 || str1[0]=='#') continue;

        ret = sscanf(input_buffer," %s %s ", str1, str2);
        if(ret!=2) {mg_error("Error reading network file: %s", file_name); mg_quit(EXIT_FAILURE);}

        it = verts.find(std::string(str1));
        if(it!=verts.end()) num1 = it->second;
        else {
            num1 = g->n_vertices;
            g->n_vertices++;
            verts[std::string(str1)] = num1;
        }
        it = verts.find(std::string(str2));
        if(it!=verts.end()) num2 = it->second;
        else {
            num2 = g->n_vertices;
            g->n_vertices++;
            verts[std::string(str2)] = num2;
        }        
        if (num1 != num2) g->n_edges++;
        if (n_alloc_edges < g->n_edges) {
            graph_edge_realloc(g,2*n_alloc_edges);
            g->n_edges = n_alloc_edges+1;
            n_alloc_edges *= 2;
        }
        if (num1 < num2) { g->edge_set[i] = edge_malloc(num1,num2); i++; }
        else if (num1 > num2) { g->edge_set[i] = edge_malloc(num2,num1); i++; }
    }
    if (ferror(input_file)) {
        mg_error("Error reading network file: %s", file_name); mg_quit(EXIT_FAILURE);
    }

    graph_vertex_malloc(g,g->n_vertices);
    for(it=verts.begin();it!=verts.end();++it) {
        new_vertex = vertex_malloc((it->first).c_str(), it->second);
        g->vertex_set_by_number[it->second] = new_vertex;
        g->vertex_set_by_name[it->second] = new_vertex;
    }
    graph_sort_vertices(g);

    g->sp = edge_set_to_spmat(g->edge_set, g->n_vertices, g->n_edges);
    graph_edge_realloc(g,g->n_edges);
    graph_edge_delete(g);
    g->n_edges = g->sp.IA[g->sp.nr];
    //graph_sort_edges(g);    
}


void graph_sif_read(struct graph *g, FILE *input_file, char *file_name) {
    struct vertex* new_vertex;
    int n_alloc_edges = 1024;
    graph_edge_malloc(g,n_alloc_edges); g->n_edges = 0;

    std::map<std::string,int> verts;
    //std::vector<int> edges1,edges2;
    std::map<std::string,int>::iterator it;    

    char *input_buffer=NULL;
    char *str1, *str2, *strint;
    int num1,num2;
    int i = 0;
    while((input_buffer = read_line(input_file))) {
        //        if (input_buffer[0]=='#' || input_buffer[0]=='\0') continue;
        //ret = sscanf(input_buffer, " %s ", str1);
        str1 = strtok(input_buffer, " \r\t\n");
        if (str1==NULL || str1[0]=='#') continue;

        //        ret = sscanf(input_buffer," %s %s ", str1, str2);
        strint = strtok(NULL," \r\t\n");
        str2 = strtok(NULL," \r\t\n");
        if (strint==NULL || str2==NULL) {mg_error("Error reading network file: %s", file_name); mg_quit(EXIT_FAILURE);}

        it = verts.find(std::string(str1));
        if(it!=verts.end()) num1 = it->second;
        else {
            num1 = g->n_vertices;
            g->n_vertices++;
            verts[std::string(str1)] = num1;
        }
        do {
            it = verts.find(std::string(str2));
            if(it!=verts.end()) num2 = it->second;
            else {
                num2 = g->n_vertices;
                g->n_vertices++;
                verts[std::string(str2)] = num2;
            }        
            if (num1 != num2) g->n_edges++;
            if (n_alloc_edges < g->n_edges) {
                graph_edge_realloc(g,2*n_alloc_edges);
                g->n_edges = n_alloc_edges+1;
                n_alloc_edges *= 2;
            }
            if (num1 < num2) { g->edge_set[i] = edge_malloc(num1,num2); i++; }
            else if (num1 > num2) { g->edge_set[i] = edge_malloc(num2,num1); i++; }
        } while((str2 = strtok(NULL," \r\t\n")));
        free(input_buffer);
    }
    if (ferror(input_file)) {
        mg_error("Error reading network file: %s", file_name); mg_quit(EXIT_FAILURE);
    }
    
    graph_vertex_malloc(g,g->n_vertices);
    for(it=verts.begin();it!=verts.end();++it) {
        //printf("v: %s\n",(it->first).c_str());
        new_vertex = vertex_malloc((it->first).c_str(), it->second);
        g->vertex_set_by_number[it->second] = new_vertex;
        g->vertex_set_by_name[it->second] = new_vertex;
    }
    graph_sort_vertices(g);

    g->sp = edge_set_to_spmat(g->edge_set, g->n_vertices, g->n_edges);
    graph_edge_realloc(g,g->n_edges);
    graph_edge_delete(g);
    g->n_edges = g->sp.IA[g->sp.nr];
    //graph_sort_edges(g);
}

void graph_read_edges(struct graph* g, FILE* input_file, char* file_name) {
	char input_buffer[1024];
    int ret;

	int n_edges;
	ret = fscanf(input_file, "%d\n", &n_edges);
    if (ret!=1) {
        mg_error("File format error in network file: %s", file_name);
        mg_quit(EXIT_FAILURE);
    }
	
	graph_edge_malloc(g, n_edges);
	struct edge** edge_set = g->edge_set;
	
	int i;
	char* vertex1;
	char* vertex2;
	for (i = 0; i < n_edges; i++) {
		if (fgets(input_buffer, sizeof input_buffer, input_file) == NULL) {
            mg_error("File format error in network file: %s", file_name);
            mg_quit(EXIT_FAILURE);
        }
		vertex1 = strtok(input_buffer, " ");
		vertex2 = strtok(NULL, " ");
		// Add things here to retrieve other information
		edge_set[i] = edge_malloc(atoi(vertex1) - 1, atoi(vertex2) - 1);
	}
    g->sp = edge_set_to_spmat(edge_set, g->n_vertices, n_edges);
    graph_edge_delete(g);
	//graph_sort_edges(g);
    g->n_edges = g->sp.IA[g->sp.nr];
}

void graph_read_vertices(struct graph* g, FILE* input_file, char* file_name) {
	char input_buffer[1024];
    int ret;
	
	fgets(input_buffer, sizeof input_buffer, input_file);
	fgets(input_buffer, sizeof input_buffer, input_file);
	fgets(input_buffer, sizeof input_buffer, input_file);
	fgets(input_buffer, sizeof input_buffer, input_file);
	
	int n_vertices;
	ret = fscanf(input_file, "%d\n", &n_vertices);
    if (ret!=1) {
		mg_error("File format error in network file: %s", file_name);
        mg_quit(EXIT_FAILURE);
    }
	
	graph_vertex_malloc(g, n_vertices);
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	struct vertex** vertex_set_by_number = g->vertex_set_by_number;

	int number;
	char* name;
	struct vertex* new_vertex;
	for (number = 0; number < n_vertices; number++) {
		if (fgets(input_buffer, sizeof input_buffer, input_file)==NULL) {
            mg_error("File format error in network file: %s", file_name);
            mg_quit(EXIT_FAILURE);
        }
		name = strtok(input_buffer, "|{}");
		new_vertex = vertex_malloc(name, number);
		vertex_set_by_name[number] = new_vertex;
		vertex_set_by_number[number] = new_vertex;
	}
	
	graph_sort_vertices(g);
}

void graph_print_vertices(struct graph* g) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	int n_vertices = g->n_vertices;
	
	int i;
	for (i = 0; i < n_vertices; i++)
		vertex_print(vertex_set_by_name[i]);
}

void graph_print_edges(struct graph* g) {
	struct edge** edge_set = g->edge_set;
	int n_edges = g->n_edges;
	
	int i;
	for (i = 0; i < n_edges; i++)
		edge_print(edge_set[i]);
}

void graph_sort_vertices(struct graph* g) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	
	int i;
	// Move through interval
	for (i = 1; i < g->n_vertices; i++) {
		int j;
		// Move the next element in
		for (j = i; j > 0; j--) {
			int cmp = vertex_cmp(vertex_set_by_name[j], vertex_set_by_name[j-1]);
			struct vertex* temp;
			if (cmp > 0) {
				temp = vertex_set_by_name[j-1];
				vertex_set_by_name[j-1] = vertex_set_by_name[j];
				vertex_set_by_name[j] = temp;
			}
			// If no swaps made, element is in its place
			else break;
		}
	}
}

void graph_sort_edges(struct graph* g) {
	struct edge** edge_set = g->edge_set;
	
	int i;
	// Move through interval
	for (i = 1; i < g->n_edges; i++) {
		int j;
		// Move the next element in
		for (j = i; j > 0; j--) {
			int cmp = edge_cmp(edge_set[j], edge_set[j-1]);
			struct edge* temp;
			if (cmp == 1) {
				temp = edge_set[j-1];
				edge_set[j-1] = edge_set[j];
				edge_set[j] = temp;
			}
			// If no swaps made, element is in its place
			else break;
		}
	}
}

int graph_find_vertex_number(struct graph* g, char* name) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	int n_vertices = g->n_vertices;
	
	struct vertex* v = vertex_malloc(name, -1);
	
	int left = 0;
	int right = n_vertices - 1;
	int mid = 0;
	while (right >= left) {
		mid = (left + right) / 2;
		int cmp = vertex_cmp(vertex_set_by_name[mid], v);
		if (cmp > 0)
			left = mid + 1;
		else if (cmp < 0)
			right = mid - 1;
		else
			break;
	}
	vertex_delete(v);

	if (right >= left)
		return vertex_set_by_name[mid]->number;
	else
		return -1;
}

char* graph_find_vertex_name(struct graph* g, int number) {
	struct vertex** vertex_set_by_number = g->vertex_set_by_number;
	
	return vertex_set_by_number[number]->name;
}

int graph_find_edge(struct graph* g, int v1, int v2) {
    struct spmat sp = g->sp;
	int left = sp.IA[v1];
	int right = sp.IA[v1] + sp.INZ[v1] - 1;
	while (right >= left) {
		int mid = (left + right) / 2;
        int del = sp.JA[mid] - v2;
        int cmp = ((del>0)-(del<0)); //edge_cmp(edge_set[mid], e);
		if (cmp == 1)
			left = mid + 1;
		else if (cmp == -1)
			right = mid - 1;
		else
			return mid;
	}
	return -1;
}

int graph_is_edge(struct graph* g, int vertex1, int vertex2) {
	int index = graph_find_edge(g, vertex1, vertex2);
	if (index >= 0)
		return 1;
	
	index = graph_find_edge(g, vertex2, vertex1);
	if (index >= 0)
		return 1;
	
	return 0;
}


void graph_delete(struct graph* g) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	struct vertex** vertex_set_by_number = g->vertex_set_by_number;
	int n_vertices = g->n_vertices;
	struct edge** edge_set = g->edge_set;
	int n_edges = g->n_edges;
	
	int i;
	for (i = 0; i < n_vertices; i++)
		vertex_delete(vertex_set_by_name[i]);
	free(vertex_set_by_name);
	free(vertex_set_by_number);

    if (edge_set != NULL) {
        for (i = 0; i < n_edges; i++)
            edge_delete(edge_set[i]);
        free(edge_set);
	}
    spmat_delete(g->sp);
	free(g);
}

struct alignment* alignment_calloc(struct graph** gs, int ngs, int use_alpha) {
    // check network sizes
    int i;
    for(i=1;i<ngs;i++) {
        int m = gs[i-1]->n_vertices;
        int n = gs[i]->n_vertices;
        if (m > n) {
            printf("alignment_malloc: Size of domain is larger than size of range\n");
            mg_quit(EXIT_FAILURE);
        }
    }
	
    struct alignment* a;
    a = (struct alignment*)malloc(sizeof *a);
    if(a==NULL){mg_error("Error allocating alignment. Not enough memory?");mg_quit(EXIT_FAILURE);}    
	
    struct multipermutation* mp;
    int *degrees = (int *)malloc(ngs*sizeof(int));
    if(degrees==NULL){mg_error("Allocation error.");mg_quit(EXIT_FAILURE);}        
    for(i=0;i<ngs;i++) { degrees[i] = gs[i]->n_vertices; }        
    mp = multipermutation_calloc(ngs,degrees);
	
    a->networks = gs;
    a->n_networks = ngs;
    a->mp = mp;
    if (use_alpha) a->invmp = multipermutation_calloc(ngs,degrees);
	
    //	a->n_edges_preserved = -1;
    //	a->n_edges_induced = -1;
    a->score = -1.0;
    a->is_computed = 0;

    free(degrees);
	
    return a;
}

void alignment_randomize(struct alignment* a) {
	struct multipermutation* mp = a->mp;
	multipermutation_randomize(mp);
}

int alignment_read(struct alignment* a, char* file_name) {

    struct graph** gs = a->networks;
	
    FILE* input_file = fopen(file_name, "r");
    if (input_file == NULL) {
        mg_error( "Couldn't open alignment file: %s", file_name);
    }

    int i,j,k,l,nk;
    k = a->n_networks;
    nk = gs[k-1]->n_vertices;

    char *contline,*line = (char*)malloc(2048*sizeof(char));
    if(line==NULL){mg_error("Error allocating vertex map. Not enough memory?");mg_quit(EXIT_FAILURE);}    
    char *name1,*name2;
    int vertex1,vertex2;
    struct permutation* p;
    int* sequence;

    struct permutation **perms = a->mp->perms;

    for(l=0;l<k-1;l++)
        for(i=0;i<perms[l]->degree;i++)
            perms[l]->sequence[i] = -1;

    while(fgets(line,2048,input_file) != NULL) {
        //        ret = fscanf(input_file, "%[^ \t\n\r\v\f,]", name1);
        contline = line;
        name1 = contline;
        for(;*contline!=',';contline++); *contline = '\0'; contline++;
        name1 = trimwhitespace(name1);
        if(name1) {
            vertex1 = graph_find_vertex_number(gs[0],name1);
            if (vertex1<0) {
                mg_error( "Error in alignment file: %s. Unknown vertex name: %s, in graph %d.", file_name, name1,1);
            }		
        }            
        else vertex1 = -1;
        for (l=0;l<k-1;l++) {
            p = perms[l];
            sequence = p->sequence;             
            //name2 = strtok(NULL,",");
            name2 = contline;
            for(;*contline!=',';contline++); *contline = '\0'; contline++;       
            name2 = trimwhitespace(name2);
            if(name2) {
                vertex2 = graph_find_vertex_number(gs[l+1],name2);
                if (vertex2<0) {
                    mg_error( "Error in alignment file: %s. Unknown vertex name: %s, in graph %d.", file_name, name2, l+2); mg_quit(EXIT_FAILURE);
                }		
            }                
            else vertex2 = -1;
            
            if (vertex1>-1) sequence[vertex1] = vertex2;
            
            name1 = name2;
            vertex1 = vertex2;
        }
    }
    free(line);

    // Randomize the rest
    int* mapped_to = (int*)malloc(nk*sizeof(int));
    if(mapped_to==NULL){mg_error("Error allocating vertex map. Not enough memory?");mg_quit(EXIT_FAILURE);}

    int nl,nlx;    
    for(l=0;l<k-1;l++) {
        nl = gs[l]->n_vertices;
        nlx = gs[l+1]->n_vertices;
        p = a->mp->perms[l];
        sequence = p->sequence;                     

        for(i=0;i<nlx;i++) mapped_to[i] = 0;        
    
        for (i = 0; i < nlx; i++)
            if (sequence[i] >= 0 && sequence[i] < nlx)
                mapped_to[sequence[i]] = 1;
        j = 0;
        for (i = 0; i < nlx; i++)
            if (mapped_to[i] == 0) {
                mapped_to[j] = i;
                j++;
            }
        struct permutation* shuffle_perm = permutation_calloc(nlx - nl);
        knuth_shuffle(shuffle_perm);
        for (i = 0; i < nlx - nl; i++)
            sequence[nl+i] = mapped_to[evaluate(shuffle_perm, i)];	
        permutation_delete(shuffle_perm);
    }
    free(mapped_to);

    //    for(l=0;l<k-1;l++) permutation_print(a->mp->perms[l]);
	
    printf("Alignment %s read.\n", file_name);
	
    return 0;
}

void alignment_tensor(struct alignment* a3, struct alignment* a1, struct alignment* a2, struct tensor_aux_space* taux) {
    //    int i;
//    for(i=0;i<a1->n_networks-1;i++) {
//        tensor(a3->mp->perms[i], a1->mp->perms[i], a2->mp->perms[i], maux[i]);
//    }
    tensor(a3->mp->perms, a1->mp->perms, a2->mp->perms, taux);
}


int alignment_compare(struct alignment* a1, struct alignment* a2, struct carrier* rel) {
    float del = a2->score - a1->score;
    return ((del>0)-(del<0));    
}

// void alignment_update_inverse(struct alignment* a) {
//     int i;
//     for(i=0;i<a->n_networks-1;i++) {
//         struct permutation* p = a->mp->perms[i];
//         struct permutation* q = a->invmp->perms[i];
//         inverse(q, p);
//     }
// }

// converts set of edges to sparse matrix
// unweighted edges, so values are not used
struct spmat edge_set_to_spmat(struct edge **edge_set, int nk, int nnz) {
    struct spmat sp = spmat_malloc(nk,nnz);
    int *spWI = (int*)malloc(nk*sizeof(int)); // for the accum. could be done w/ spINZ
    int i,j,k;
    // create uncompressed spmat
    sp.IA[0] = 0;
    for(i=0;i<nk;i++) sp.INZ[i] = 0;
    for(i=0;i<nnz;i++) sp.INZ[edge_set[i]->vertex1]++;
    for(i=0;i<nk;i++) sp.IA[i+1] = sp.IA[i]+sp.INZ[i]; // spIA = [0;cumsum(spINZ)]
    for(i=0;i<nk;i++) sp.INZ[i] = 0;
    for(i=0;i<nnz;i++) {
        j = sp.IA[edge_set[i]->vertex1] + sp.INZ[edge_set[i]->vertex1]++;
        sp.JA[j] = edge_set[i]->vertex2;
        // btw, if sp.A is different for each i, then we have to reorder it here        
    }
    // accumulate duplicates
    for(i=0;i<nk;i++) spWI[i] = -1;
    int start,oldend,count = 0;
    for(j=0;j<nk;j++) {
        start = count;
        oldend = sp.IA[j] + sp.INZ[j];
        for(k=sp.IA[j]; k<oldend; k++) {
            i = sp.JA[k];
            if (spWI[i] >= start) {
                // sp.A[spWI[i]] += sp.A[k];
                sp.INZ[j]--;
            } else {
                // sp.A[count] = sp.A[k];
                sp.JA[count] = sp.JA[k];
                spWI[i] = count;
                count++;
            }
        }
        sp.IA[j] = start;
    }
    sp.IA[nk] = count;
    free(spWI);
    return sp;
}

void alignment_write(struct population *pop, struct alignment* a, char* file_name) {
    FILE* output_file = NULL;
    if (file_name != NULL) {
        output_file = fopen(file_name, "w");
        if (output_file == NULL) mg_error("alignment_write: Couldn't open file: %s\n", file_name);
    }

    struct graph** gs = a->networks;
    int k = a->n_networks;
    int nk = gs[k-1]->n_vertices;
    int l, nl;
    int i;
		
    char *name;
    //    std::vector<int> vlist(k);
    int *vlist = (int*)malloc(k*sizeof(int));
    if(vlist==NULL) { mg_error("Allocation error"); mg_quit(EXIT_FAILURE); }
    for(i=0;i<nk;i++) {
        int v_weight = 1;
        vlist[k-1] = i;
        struct permutation *perminv;
        //        alignment_update_inverse(a);
        for (l=k-2; l>=0; l--) {
            //  perminv = a->invmp->perms[l];
            perminv = permutation_calloc(a->mp->perms[l]->degree);            
            inverse(perminv,a->mp->perms[l]);
            nl = gs[l]->n_vertices;                        
            vlist[l] = evaluate(perminv,vlist[l+1]);
            permutation_delete(perminv);
            if (vlist[l] >= nl) {
                while(l>=0) {
                    vlist[l] = -1;
                    l--;
                }
                break;
            }
            else v_weight++;
        }
        int s;
        if (v_weight>1) {
            for(l=0;l<k;l++) {
                if (l!=0) fprintf(output_file,"\t");
                s = pop->gs2orig[l];
                if(vlist[s]>-1) {
                    name = graph_find_vertex_name(gs[s], vlist[s]);
                    if (file_name) fprintf(output_file, "%s", name);
                    else printf("%s", name);
                }
                else {
                    if(file_name) fprintf(output_file, "-");
                    else printf("-");
                }                
            }
            if (file_name) fprintf(output_file, "\n");
            else printf("\n");
        }
    }
    free(vlist);
    fclose(output_file);
}

void alignment_print(struct population *pop, struct alignment* a) {
    alignment_write(pop, a, NULL);
}

void alignment_delete(struct alignment* a, int use_alpha) {
	multipermutation_delete(a->mp);
        if (use_alpha) multipermutation_delete(a->invmp);
	free(a);
}

// This returns the fraction of maps that are mapped correctly
// Correct as in wrt vertex name
float alignment_node_correctness(struct alignment *a) {
    //    int *sequence = a->perm->sequence;
    //    int n_dom = a->networks[0]->n_vertices;
    //    int n_ran = a->networks[1]->n_vertices;
    int sum = 0;

    //    printf("alignment correctness\n");

	int i;
    //	char* name1;
    //	char* name2;
    int n_dom = 1;
	for (i = 0; i < n_dom; i++) {
//		name1 = graph_find_vertex_name(a->networks[0], i);
//		j = evaluate(a->perm, i);
//		name2 = graph_find_vertex_name(a->networks[1], j);
//        sum += (strcmp(name1,name2)==0); // this looks at the whole name
        //sum += (strncmp(name1,name2,1)==0); // this looks at only the first character
        //        printf("dom(%d) %s rge(%d) %s\n", i, name1, j, name2);
    }
    return ((float)sum/n_dom);
}

//#define CG_DEBUG

void alignment_composite_graph(struct alignment *a, struct compute_aux_space *caux) {
    struct graph **gs = a->networks;
    int k = a->n_networks;
    int nk = gs[k-1]->n_vertices;
    int *spA = caux->spA; // values
    int *spIA = caux->spIA; // ind first elt of each row
    int *spJA = caux->spJA; // col inds
    int *spJR = caux->spJR; // col inds
    int *spJC = caux->spJC; // row inds
    int *spINZ = caux->spINZ; // nnz of each row
    int *spWI = caux->spWI; // for the accum. could be done w/ just spINZ
    int *v_weights = caux->v_weights;
    int i,j,l,nl;
    int nnz=0;
    for(l=0;l<k;l++) nnz += gs[l]->n_edges;
    for(i=0;i<nk;i++) v_weights[i] = 1; //Init cluster sizes
    int v1,v2,u = 0;
    struct spmat csp;
    csp = gs[k-1]->sp;
    // Goes through the last network and adds all edges.
    for(j=0;j<csp.nr;j++) {
        for(i=csp.IA[j]; i<csp.IA[j] + csp.INZ[j]; i++) {
            v1 = j;
            v2 = csp.JA[i];
            if (v1<v2) { spJR[u] = v1; spJC[u] = v2; spA[u] = 1; }
            else { spJR[u] = v2; spJC[u] = v1; spA[u] = 1; }
            u++;
        }
    }
    struct permutation *perm0, *permtmp;
    struct permutation **perms = a->mp->perms;
    perm0 = caux->perm0;
    for(i=0;i<nk;i++) perm0->sequence[i] = i;
    permtmp = caux->permtmp;
    // This is the idea: for each node in the last network, there is a
    // corresponding "cluster" or 1-1 alignment. Conceptually, we are
    // fusing each cluster into a node while retaining the intra-network
    // edges. Algorithmically, we go through each edge in each network
    // (going from last to first), and add the edge (i.e. node pairs) to
    // the composite graph.
    // The rest after this for loop is just converting from (i,j,x)
    // to an uncompressed sparse graph format (yale), while adding the x's.
    for (l=k-2; l>=0; l--) {
        nl = gs[l]->n_vertices;
        csp = gs[l]->sp;
        // Composition: permtmp(x) = perm0(perms[l](x)); perm0 = permtmp
        for(i=0;i<nl;i++) permtmp->sequence[i] = evaluate(perm0,evaluate(perms[l],i));
        for(i=0;i<nl;i++) perm0->sequence[i] = permtmp->sequence[i];
        for(i=0;i<nl;i++) v_weights[evaluate(perm0,i)]++;
        for(j=0;j<csp.nr;j++) {
            for(i=csp.IA[j]; i<csp.IA[j] + csp.INZ[j]; i++) {
                v1 = evaluate(perm0,j);
                v2 = evaluate(perm0,csp.JA[i]);
                if (v1<v2) { spJR[u] = v1; spJC[u] = v2; spA[u] = 1; }
                else { spJR[u] = v2; spJC[u] = v1; spA[u] = 1; }
                u++;
            }
        }
    }
    // Create uncompressed spmat
    spIA[0] = 0;
    for(i=0;i<nk;i++) spINZ[i] = 0;
    for(i=0;i<nnz;i++) spINZ[spJC[i]]++;
    for(i=0;i<nk;i++) spIA[i+1] = spIA[i]+spINZ[i]; // NB: spIA = [0;cumsum(spINZ)]
    for(i=0;i<nk;i++) spINZ[i] = 0;
    for(i=0;i<nnz;i++) {
        j = spIA[spJC[i]] + spINZ[spJC[i]]++;
        spJA[j] = spJR[i];
        // BTW, if spA is different for each i, then we have to reorder it here.       
    }
#ifdef CG_DEBUG
    printf("A: "); for(i=0;i<nnz;i++) printf("%d ", spA[i]); printf("\n");
    printf("JA: "); for(i=0;i<nnz;i++) printf("%d ", spJA[i]); printf("\n");
    printf("JR: "); for(i=0;i<nnz;i++) printf("%d ", spJR[i]); printf("\n");
    printf("JC: "); for(i=0;i<nnz;i++) printf("%d ", spJC[i]); printf("\n");
    printf("IA: "); for(i=0;i<nk+1;i++) printf("%d ", spIA[i]); printf("\n");
    printf("INZ: "); for(i=0;i<nk;i++) printf("%d ", spINZ[i]); printf("\n");
    printf("v_weights: "); for(i=0;i<nk;i++) printf("%d ", v_weights[i]); printf("\n");
#endif
    // accumulate duplicates
    for(i=0;i<nk;i++) spWI[i] = -1;
    int start,oldend,count = 0;
    for(j=0;j<nk;j++) {
        start = count;
        oldend = spIA[j] + spINZ[j];
        for(k=spIA[j]; k<oldend; k++) {
            i = spJA[k];
            if (spWI[i] >= start) {
                spA[spWI[i]] += spA[k];
                spINZ[j]--;
            }
            else {
                spA[count] = spA[k];
                spJA[count] = spJA[k];
                spWI[i] = count;
                count++;
            }
        }
        spIA[j] = start;
    }
    spIA[nk] = count;

#ifdef CG_DEBUG
    printf("A: "); for(i=0;i<nnz;i++) printf("%d ", spA[i]); printf("\n");
    printf("JA: "); for(i=0;i<nnz;i++) printf("%d ", spJA[i]); printf("\n");
    printf("JR: "); for(i=0;i<nnz;i++) printf("%d ", spJR[i]); printf("\n");
    printf("JC: "); for(i=0;i<nnz;i++) printf("%d ", spJC[i]); printf("\n");
    printf("IA: "); for(i=0;i<nk+1;i++) printf("%d ", spIA[i]); printf("\n");
    printf("INZ: "); for(i=0;i<nk;i++) printf("%d ", spINZ[i]); printf("\n");
    printf("v_weights: "); for(i=0;i<nk;i++) printf("%d ", v_weights[i]); printf("\n");
#endif
}

float alignment_partial_substructure_score(struct alignment *a, struct compute_aux_space *caux) {
    struct graph **gs = a->networks;
    int k = a->n_networks;
    int nk = gs[k-1]->n_vertices;
    int *spA = caux->spA; // values
    int *spIA = caux->spIA; // ind first elt of each row
    int *spJA = caux->spJA; // col inds
    int *spINZ = caux->spINZ; // nnz of each row
    int *v_weights = caux->v_weights;
    int i,j;
    
    // calculate preserved and unique edges
    float cis = 0;
    int unique_edges = 0;
    int r,w;
    // btw this is the transpose of the composite graph. hence j is col ind, spJA[i] is row ind
    for (j=0;j<nk;j++) {
        for(i=spIA[j]; i<spIA[j] + spINZ[j]; i++) {
            w = std::min(v_weights[j],v_weights[spJA[i]]);
            r = spA[i];
            #ifdef INSPECT_COMPOSITE_GRAPH
                printf("row %d col %d vrow %d vcol %d w %d r %d\n",
                       j,spJA[i],v_weights[j],v_weights[spJA[i]],w,r);
            #endif
            if (w > 1) {
                if (r>1) unique_edges++;
                if (r>1) cis += (float)r / (float)w;
            }
        }
    }
    return (cis/(float)unique_edges);    
}    

float alignment_cluster_interaction_quality(struct alignment *a, struct compute_aux_space *caux) {
    struct graph **gs = a->networks;
    int k = a->n_networks;
    int nk = gs[k-1]->n_vertices;
    int *spA = caux->spA; // values
    int *spIA = caux->spIA; // ind first elt of each row
    int *spJA = caux->spJA; // col inds
    int *spINZ = caux->spINZ; // nnz of each row
    int *v_weights = caux->v_weights;
    int i,j;
    
    // calculate preserved and unique edges
    float cis = 0;
    int unique_edges = 0;
    int total_edges = 0;
    int r,w;
    // btw this is the transpose of the composite graph. hence j is col ind, spJA[i] is row ind
    for (j=0;j<nk;j++) {
        for(i=spIA[j]; i<spIA[j] + spINZ[j]; i++) {
            w = std::min(v_weights[j],v_weights[spJA[i]]);
            r = spA[i];
            #ifdef INSPECT_COMPOSITE_GRAPH
                printf("row %d col %d vrow %d vcol %d w %d r %d\n",
                       j,spJA[i],v_weights[j],v_weights[spJA[i]],w,r);
            #endif
            if (w > 1) {
                //if (r>0) unique_edges++; // r>0 is required b/c (0,0) gets added regardless; idk why
                if (r>0) total_edges += r;
                if (r>1) cis += (float)r * (float)r / (float)w;
            }
        }
    }
    return (cis/(float)total_edges);    
}    

//mine
void delete_fmp(struct alignment* a, int** fmp){
    struct graph ** networks = a->networks;
    int gn = a->n_networks;
    int vn = networks[gn - 1]->n_vertices;
    int i, j, k;

    for (i = 0; i < vn; ++i){
        free(fmp[i]);
    }
    free(fmp);
}

//my function alignment->fusion node
int** alignment_fusion_node(struct alignment* a){
    struct graph ** networks = a->networks;
    int gn = a->n_networks;
    int vn = networks[gn - 1]->n_vertices;

    //fmp[i][j]: fusion node i in graph j
    int** fmp = (int**) malloc(vn * sizeof(int*));
    int i, j, k;
    for (i = 0; i < vn; ++i){
        fmp[i] = (int*) malloc(gn * sizeof(int));
    }
    //init, TODO: use memset
    for (i = 0; i < vn; ++i){
        for (j = 0; j < gn; ++j){
            fmp[i][j] = -1;
        }
    }
    // printf("test point a\n");

    struct multipermutation* mp = a->mp;
    for (i = 0; i < vn; ++i)
        fmp[i][gn - 1] = i;
    
    // printf("test point b\n");
    for (i = gn - 1; i > 0; --i){
        int pi = i - 1;//prev graph
        struct permutation * p = mp->perms[i - 1]; //graph 0 has no perm
        int* seq = p->sequence;
        int lvn = networks[pi]->n_vertices;//local vertex num
        for (j = 0; j < lvn; ++j){//j th element in seq
            for (k = 0; k < vn; ++k){//visit all fusion nodes
                if (fmp[k][i] == seq[j]){
                    fmp[k][pi] = j;
                    break;
                }
            }
        }
    }
    
    // printf("test point c\n");

    return fmp;
}

void delete_emp(struct alignment* a, int*** emp){
    struct graph ** networks = a->networks;
    int gn = a->n_networks;
    int vn = networks[gn - 1]->n_vertices;
    int i, j, k;

    for (i = 0; i < vn; ++i){
        for(j = 0; j < vn; ++j){
            free(emp[i][j]);
        }
        free(emp[i]);
    }
    free(emp);
}

int*** alignment_fusion_edges(struct alignment* a, int** fmp){
    struct graph ** networks = a->networks;
    int gn = a->n_networks;//graph num
    int vn = networks[gn - 1]->n_vertices;//max vertex num_get

    //fmp[i][j]: fusion node i in graph j
    int*** emp = (int***) malloc(vn * sizeof(int**));
    int i, j, k;
    for (i = 0; i < vn; ++i){
        emp[i] = (int**) malloc(vn * sizeof(int*));
        for (j = 0; j < vn; ++j){
            emp[i][j] = (int*) malloc(gn * sizeof(int));
            for (k = 0; k < gn; ++k)
                emp[i][j][k] = -1;
        }
    }

    for (i = 0; i < vn; ++i){
        for (j = 0; j < vn; ++j){
            for (k = 0; k < gn; ++k){
                int ni = fmp[i][k];
                int nj = fmp[j][k];
                if (ni == -1 || nj == -1) {
                    emp[i][j][k] = 0;
                    continue;
                }
                struct graph* g = networks[k];
                if (graph_is_edge(g, ni, nj)) emp[i][j][k] = 1;
                else emp[i][j][k] = 0;
            }
        }
    }
    return emp;
} 


//a, b bitset
float cal_similarity(int a, int b, int vn){
    if (a == 0 && b == 0) return 1.0; // self
    int share, total, i;
    int na, nb;
    for (i = 0; i < vn; ++i){
        na = a & (1 << i);
        nb = b & (1 << i);
        if (na && nb) ++share;
        if (na || nb) ++total;
    }
    float ans =  ((float)share) / ((float)total);
    // printf("%d %d %f\n", a, b, ans);
    return ans;
}

float cal_my_score(struct alignment* a, int ** fmp, int*** emp){
    struct graph ** networks = a->networks;
    int gn = a->n_networks;//graph num
    int vn = networks[gn - 1]->n_vertices;//max vertex num_get
    float score = 0.0;

    int i, j, k;
    // //cal vi, ei
    // for (i = 0; i < vn; ++i){
    //     int wi = 0;
    //     for (j = 0; j < gn; ++j){
    //         if (fmp[i][j] != -1) ++wi;
    //     }
    // }

    for (i = 0; i < vn; ++i){
        int vi, ei;
        vi = 0;
        ei = 0;
        for (j = 0; j < gn; ++j)
            if (fmp[i][j] != -1) ++vi;

        int * u = (int*) malloc(gn * sizeof(int)); //user, TODO: use bitset later
        memset(u, 0, sizeof(u));
        for (j = 0; j < vn; ++j){
            if (j == i) continue;
            //node i j
            for (k = 0; k < gn; ++k){
                if (emp[i][j][k]) 
                    u[k] = u[k] | (1 << j);
            }
        }
        int* cnt = (int*) malloc(gn * sizeof(int));
        for (j = 0; j < gn; ++j) cnt[j] = 1;
        //find same user
        for (j = 0; j < gn; ++j){
            if (!cnt[j]) continue;
            for (int k = j + 1; k < gn; ++k){
                if (!cnt[k]) continue; //impossible
                if (u[j] == u[k]){
                    ++cnt[j];
                    cnt[k] = 0;
                }
            }
        }
        // printf("=======debug u=======\n");
        // printf("=======debug u=======\n");
        // printf("=======debug u=======\n");
        // printf("=======debug u=======\n");
        // printf("=======debug u=======\n");
        // for (j = 0; j < gn; ++j){
        //     if (u[j] == 0)
        //         printf("%d %d\n", cnt[j], u[j]);
        // }
        // printf("-------\n");
        
        //users to clusters
        int c_num = 0; //cluster cnt
        int* c_u = (int*) malloc(gn * sizeof(int));
        int* c_cnt = (int*) malloc(gn * sizeof(int));
        for (j = 0; j < gn; ++j){
            if (cnt[j]) { //u[j] can be 0
                c_cnt[c_num] = cnt[j];
                c_u[c_num++] = u[j];
            }
        }

        //  for (j = 0; j < c_num; ++j){
        //     if (c_u[j] == 0)
        //         printf("%d %d\n", c_cnt[j], c_u[j]);
        // }
        // printf("-------\n");

        float* p = (float*) malloc(c_num * sizeof(float));
        for (j = 0; j < c_num; ++j){
            p[j] = float(c_cnt[j]) / float(gn);//!!!!!

            int t = 0;//edge num
            for (k = 0; k < gn; ++k){
                if (c_u[j] & (1 << k)) ++t;
            }
            ei += c_cnt[j] * t;
        }
        // printf("p: ");
        // for (j = 0; j < c_num; ++j)
        //     printf("%f ", p[j]);
        // printf("\n");

        float local_score = 0.0;
        for (j = 0; j < c_num; ++j){
            float in_log = 0.0, sim = 0.0;
            for (k = 0; k < c_num; ++k){
                sim = cal_similarity(c_u[j], c_u[k], gn);
                in_log += p[k] * sim;
                
                // printf("pk: %f, sim: %f, in_log: %f\n", p[k], sim, in_log);
            }
            local_score += p[j] * log(in_log) * (-1.0);
        }
        // printf("ei: %d, vi: %d, ls: %f\n", ei, vi, local_score);
        score += ei * vi * local_score;

        free(u);
        free(cnt);
        free(c_u);
        free(c_cnt);
        free(p);
    }
    return score;
}


void alignment_compute(struct alignment* a, struct carrier* rel,
                       compute_aux_space *caux) {
    //print info to check alignment
    struct graph ** networks = a->networks;
    int graph_n = a->n_networks;
    struct graph* n0 = networks[0];
    int vn = n0->n_vertices;
    int i, j, k;
    // printf("=======debug edge=======\n");
    // for (i = 0; i < vn; ++i){
    //     for (j = i + 1; j < vn; ++j){
    //         if (graph_is_edge(n0, i, j))
    //             printf("%d %d\n", i, j);
    //     }
    // }


    struct multipermutation* mp = a->mp;
    struct multipermutation* invmp = a->invmp;

    // printf("=======debug perm=======\n");
    // for (i = 0; i < graph_n - 1; ++i){
    //     printf("%d:", i + 1); //graph 0 has no perm
    //     struct permutation * p = mp->perms[i];
    //     int d = p->degree;
    //     int* seq = p->sequence;
    //     for (j = 0; j < d; ++j)
    //         printf("%d ", seq[j]);
    //     printf ("\n");
    // }


    //invmp not usable
    // printf("-------\n");
    // for (i = 1; i < graph_n; ++i){
    //     printf("%d:", i);
    //     struct permutation * p = invmp->perms[i];
    //     int d = p->degree;
    //     int* seq = p->sequence;
    //     for (j = 0; j < d; ++j)
    //         printf("%d ", seq[j]);
    //     printf ("\n");
    // }

    int ** fmp = alignment_fusion_node(a);
    int gn = a->n_networks;
    int f_vn = networks[gn - 1]->n_vertices;

    // printf("-------fusion node-------\n");
    // for (i = 0; i < f_vn; ++i){
    //     printf("%d:", i);
    //     for (j = 0; j < gn; ++j){
    //         printf("%d ", fmp[i][j]);
    //     }
    //     printf("\n");
    // }

    int *** emp = alignment_fusion_edges(a, fmp);

    // printf("-------fusion edge-------\n");
    // for (i = 0; i < f_vn; ++i){
    //     for (j = 0; j < f_vn; ++j){
    //         printf("%d, %d: ", i, j);
    //         for (k = 0; k < gn; ++k){
    //             if (emp[i][j][k]) printf("%d ", k);
    //         }
    //         printf("\n");
    //     }
    // }


    float score = cal_my_score(a, fmp, emp);
    a->my_score = score; // need 1 / score

    // printf("-------fusion score-------\n");
    // printf("score: %f\n", score);


    delete_fmp(a, fmp);
    delete_emp(a, emp);


    alignment_composite_graph(a,caux);

    float es;
    if (rel->rel==2) es = alignment_partial_substructure_score(a,caux); // S3
    else if (rel->rel==1) es = alignment_cluster_interaction_quality(a,caux); // CIQ
    else es = -1;

//    printf("S3 %f CIQ %f\n",
//           alignment_partial_substructure_score(a,caux),
//           alignment_cluster_interaction_quality(a,caux));

    a->edge_score = es;
    a->node_score = 0.0;
    // a->my_score = 0.0;

    /* add node score here */
    if (rel->use_alpha) {
        int i;
        struct multipermutation* invmp = a->invmp;
        struct multipermutation *mp = a->mp;
        for (i=0;i<mp->k-1;i++) inverse(invmp->perms[i],mp->perms[i]);
        a->node_score = alignment_nodescore_compute(a,rel);
        a->score = (a->edge_score)*(rel->alpha) + a->node_score*(1-rel->alpha);
    }
    else {
        a->score = a->edge_score;
    }

//    printf("alpha: %f, score: %f\n", rel->alpha, a->score);
//    printf("dom: %d, rge: %d, pdeg: %d\n", a->networks[0]->n_vertices,
//           a->networks[1]->n_vertices, a->perm->degree);

    a->score = (1.0 / (a->my_score + 1.0));

    // if (base_my_score == 0.0){
    //     a->score = 1.0 / (a->my_score + 1.0);
    // }
    // else{
    //     if (a->my_score > base_my_score){
    //         a->score = 0.0;
    //     }
    //     else{
    //         a->score = 1.0 - a->my_score / base_my_score;
    //     }
    // }
    // a->score = sqrt(a->score);

    // a->edge_score = a->score;

	a->is_computed = 1;
}

float alignment_nodescore_compute(struct alignment *a, struct carrier *rel) {
    struct graph **gs = a->networks;
    int k = a->n_networks;
    int nk = gs[k-1]->n_vertices;
    int nl,l;
    float **cmpdata = rel->cmpdata;

    float sum = 0;

    int *phi = (int*)malloc(k*sizeof(int));

    //struct permutation *perminv;
    //perminv = permutation_calloc(nk);

    int c,istart,ix;
    // create phi
    for(c=0; c<nk; c++) {
        phi[k-1] = c;
        nl = nk;
        istart = 0;
        for (l=k-2; l>=0; l--) {
            if (phi[l+1]<0) {
                break;
                //phi[l] = -1;
            }
            else {
                //perminv->degree = nl;
                //inverse(perminv,a->mp->perms[l]);
                nl = gs[l]->n_vertices;
                ix = a->invmp->perms[l]->sequence[phi[l+1]];
                if (ix>=nl) {
                    istart = l+1;
                    //phi[l] = -1;
                    break;
                }
                else {
                    phi[l] = ix;
                }
            }
        }

        float sumc = 0;
        int i,j;
        for (i=istart; i<k; i++) {
            for(j=i+1; j<k; j++) {
                sumc += cmpdata[i*k+j][ phi[i]*gs[j]->n_vertices + phi[j] ];
            }
        }
        if (istart<k-1) sum += sumc / ((k-istart)*(k-istart-1)/2);
    }

    
    //permutation_delete(perminv);
    free(phi);
    return (sum/nk);
}

struct compute_aux_space* compute_aux_space_malloc(struct graph **gs, int ng) {
    struct compute_aux_space* caux = (struct compute_aux_space*)
        malloc(sizeof(*caux));
    int l;
    int nnz=0;
    for(l=0;l<ng;l++) nnz += gs[l]->n_edges;
    int nk = gs[ng-1]->n_vertices;
    caux->spA = (int*)malloc(nnz*sizeof(int)); // final values
    caux->spIA = (int*)malloc((nk+1)*sizeof(int)); // ind first elt of each row
    caux->spJA = (int*)malloc(nnz*sizeof(int)); // final col inds
    caux->spJR = (int*)malloc(nnz*sizeof(int)); // row inds
    caux->spJC = (int*)malloc(nnz*sizeof(int)); // col inds
    caux->spINZ = (int*)malloc(nk*sizeof(int)); // nnz of each row
    caux->spWI = (int*)malloc(nk*sizeof(int)); // for the accum. could be done w/ spINZ
    caux->v_weights = (int*)malloc(nk*sizeof(int));
    caux->perm0 = permutation_calloc(nk);
    caux->permtmp = permutation_calloc(nk);    
    
//     (caux->cg).resize(gs[ng-1]->n_vertices,gs[ng-1]->n_vertices);
//     int mtotal=0;
//     for(l=0;l<ng;l++) mtotal += gs[l]->n_edges;    
//     (caux->cg_triplets).resize(mtotal);
//     caux->v_weights = (int*)malloc(gs[ng-1]->n_vertices*sizeof(int)); // vector of "cluster sizes"
//     if(caux->v_weights==NULL) {mg_error("Allocation error."); mg_quit(EXIT_FAILURE);}                
    return(caux);
}

void compute_aux_space_delete(struct compute_aux_space* caux) {
    free(caux->spA);
    free(caux->spIA);
    free(caux->spJA);
    free(caux->spJR);
    free(caux->spJC);
    free(caux->spINZ);
    free(caux->spWI);
    free(caux->v_weights);
    permutation_delete(caux->perm0);
    permutation_delete(caux->permtmp);        
    free(caux);
}


// #define INSPECT_COMPOSITE_GRAPH

// // Creates a sparse matrix, which describes the composite graph of the alignment
// // The edge weights are the number of edges between the nodes
// // The node weights are the "cluster sizes"
// // Creates the composite graph in O(m) time and O(m) space
// // where m is the total number of edges in all the networks
// void alignment_composite_graph(struct alignment *a,
//                                compute_aux_space *caux) {
//     SpMat& composite_graph = caux->cg;
//     std::vector<EigTriplet> &cg_triplets = caux->cg_triplets;    
//     int *v_weights = caux->v_weights;
//     struct graph** gs = a->networks;
//     int k = a->n_networks;
//     int nk = gs[k-1]->n_vertices;
//     int l, nl;
//     int i,j;
// 
// //    int mtotal=0;
// //    for(l=0;l<k;l++) mtotal += gs[l]->n_edges;
//     int u=0;
// 
//     int v1,v2;
//     for(i=0;i<nk;i++) v_weights[i] = 1;
//     for(j=0;j<gs[k-1]->n_edges;j++) {
//         v1 = gs[k-1]->edge_set[j]->vertex1;
//         v2 = gs[k-1]->edge_set[j]->vertex2;
//         if (v1<v2) cg_triplets[u] = EigTriplet(v1,v2,1);
//         else cg_triplets[u] = EigTriplet(v2,v1,1);
//         u++;
//     }
// 
//     struct permutation* perm0, *permtmp;;
//     perm0 = permutation_calloc(nk);
//     permtmp = permutation_calloc(nk);
//     for (l=k-2; l>=0; l--) {
//         nl = gs[l]->n_vertices;
//         for(i=0;i<nl;i++) {
//             permtmp->sequence[i] = evaluate(perm0,evaluate(a->mp->perms[l],i));
//         }
//         for(i=0;i<nl;i++) perm0->sequence[i] = permtmp->sequence[i];
// 
//         for(i=0;i<nl;i++) v_weights[evaluate(perm0,i)]++;
//         for(j=0;j<gs[l]->n_edges;j++) {
//             v1 = evaluate(perm0,gs[l]->edge_set[j]->vertex1);
//             v2 = evaluate(perm0,gs[l]->edge_set[j]->vertex2);
//             if (v1<v2) cg_triplets[u] = EigTriplet(v1,v2,1);
//             else cg_triplets[u] = EigTriplet(v2,v1,1);
//             u++;
//         }
//     }
//     permutation_delete(perm0);
//     permutation_delete(permtmp);
//     //composite_graph.resize(nk,nk);
//     composite_graph.setZero();    
//     composite_graph.setFromTriplets(cg_triplets.begin(),cg_triplets.end());
// 
// #ifdef INSPECT_COMPOSITE_GRAPH    
//         printf("---\n");
//         alignment_write(a, NULL);
//         //permutation_print(a->mp->perms[0]);
//         std::cout << composite_graph << std::endl;
//         printf("cluster sizes: ");
//         for(i=0;i<nk;i++) {
//             printf("%d ",v_weights[i]);
//         }
//         printf("\n");
// #endif
// }
// 
// // Conserved interaction score, first described in the BEAMS paper
// // Except, this is for one-to-one alignment
// float alignment_conserved_interaction_score(SpMat G, int *v_weights) {
//     float cis = 0;
//     int i;
//     int u,v;
//     for (i=0;i<G.outerSize();i++) {
//         for(Eigen::SparseMatrix<int>::InnerIterator it(G,i); it; ++it) {
//             u = v_weights[it.row()];
//             v = v_weights[it.col()];
//             if (it.value() > 1 && u > 1 && v > 1) cis += (float)it.value() / (float)std::min(u,v);
//         }
//     }
//     return cis;
// }
// 
// float alignment_partial_substructure_score(SpMat G, int *v_weights) {
//     float cis = 0;
//     int unique_edges = 0;
//     int i;
//     int r,w;
//     for (i=0;i<G.outerSize();i++) {
//         for(Eigen::SparseMatrix<int>::InnerIterator it(G,i); it; ++it) {
//             w = std::min(v_weights[it.row()],v_weights[it.col()]);
//             r = it.value();
// #ifdef INSPECT_COMPOSITE_GRAPH
//                 printf("row %d col %d vrow %d vcol %d w %d r %d\n",
//                        it.row(),it.col(),v_weights[it.row()],v_weights[it.col()],w,r);
// #endif
//             if (w > 1) {
//                 if (r>0) unique_edges++; // r>0 is required b/c (0,0) gets added regardless; idk why
//                 if (r>1) cis += (float)r / (float)w;
//             }
//         }
//     }
//     return (cis/(float)unique_edges);
// }
