#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "genetic.h"

void mg_quit(int status) {
    exit(status);
}
void mg_error(const char *fmt, ...) {
    va_list arg;
    va_start(arg,fmt);
    vfprintf(stderr,fmt,arg);
    va_end(arg);
    fprintf(stderr,"\n");
    exit(EXIT_FAILURE);
}

void show_help(void) {
    printf("Usage: magna [options]\n\n");
    printf("where options are:\n");
    printf("  -G <file name>\n");
    printf("\tnetwork list file\n");
    //printf("  -i <file name>\n");
    //printf("\tinitial population (default: random)\n");
    printf("  -o <value>\n");
    printf("\toutput file\n");
    printf("  -m <value>\n");
    printf("\tedge conservation measure (currently only CIQ)\n");
    printf("  -d <file name>\n");
    printf("\tnode comparison data file list\n");
    printf("  -a <value>\n");
    printf("\talpha parameter; weight between edge and node conservation (default: 1.0; only edge conservation)\n");
    printf("  -p <value>\n");
    printf("\tpopulation size\n");
    printf("  -e <value>\n");
    printf("\tpercentage elite members (default: 0.5)\n");
    printf("  -n <value>\n");
    printf("\tnumber of generations\n");
    printf("  -f <value>\n");
    printf("\tfrequency of output (default: 1)\n");
    printf("  -t <value>\n");
    printf("\tnumber of threads to use\n");
    printf("  -h\n");
    printf("\tshow this help text\n");
}

int main(int argc, char** argv) {
	
    // Must be specified
    char* graphs_file_name = NULL;		// G
    char* output_file_prefix = NULL;	// o
    char* optimizing_measure = NULL;	// m
    int pop_size = -1;					// p
    int n_gen = -1;						// n
    int n_threads = 1;                  // t
	
    // Optional -- default values
    char* init_pop_file = NULL;			// i
    float pop_elite_ratio = 0.5;		// e
    int freq = 1;						// f
    float alpha = 1;                      // a
    int use_alpha = 0;
    char *node_sim_list_file = NULL;      // d
	
    char opt;

    if (argc==1) {
        show_help();
        exit(1);
    }
	
    // Retrieve command line arguments
    while ((opt = getopt(argc, argv, "G:i:o:m:d:a:p:e:n:f:t:h")) != -1) {
        switch (opt) {
        case 'G':
            graphs_file_name = optarg;
            break;
        case 'i':
            init_pop_file = optarg;
            break;
        case 'o':
            output_file_prefix = optarg;
            break;
        case 'm':
            optimizing_measure = optarg;
            break;
        case 'd':
            node_sim_list_file = optarg;
            break;
        case 'a':
            alpha = atof(optarg);
            use_alpha = 1;
            break;
        case 'p':
            pop_size = atoi(optarg);
            break;
        case 'e':
            pop_elite_ratio = atof(optarg);
            break;
        case 'n':
            n_gen = atoi(optarg);
            break;
        case 'f':
            freq = atoi(optarg);
            break;
        case 't':
            n_threads = atoi(optarg);
            break;
        case 'h':
            show_help();
            exit(1);
            break;
        }
    }
	
    int optimizing_measure_code;
    int pop_elite;
	
    // Checks
    if (graphs_file_name == NULL) {
        fprintf(stderr, "Network list file undefined.\n");
        exit(1);
    }
    if (output_file_prefix == NULL) {
        fprintf(stderr, "Output file prefix undefined (-o).\n");
        exit(1);
    }
    if (optimizing_measure == NULL) {
        fprintf(stderr, "Optimizing measure undefined.\n");
        exit(1);
    }
    if (strcmp(optimizing_measure, "EC") == 0)
        optimizing_measure_code = 0;
    else if (strcmp(optimizing_measure, "CIQ") == 0)
        optimizing_measure_code = 1;
    else if (strcmp(optimizing_measure, "S3") == 0)
        optimizing_measure_code = 2;
    else {
        fprintf(stderr, "Unknown measure: %s\n", optimizing_measure);
        exit(1);
    }
    if (alpha < 0 || alpha > 1) {
        fprintf(stderr, "edge-node weight value, alpha, needs to be between 0 and 1\n");
        exit(1);
    }
    if (use_alpha && node_sim_list_file == NULL) {
        fprintf(stderr, "Node comparison data file list needs to be specified using the -d option\n");
        exit(1);
    }
    if (pop_size <= 0) {
        fprintf(stderr, "Population size must be specified as a positive integer.\n");
        exit(1);
    }
    if (n_gen < 0) {
        fprintf(stderr, "Number of generations must be specified a nonnegative integer.\n");
        exit(1);
    }
    if ((0 <= pop_elite_ratio) && (pop_elite_ratio <= 1))
        pop_elite = pop_elite_ratio * pop_size;
    else {
        fprintf(stderr, "The ratio of elite members must be a real number between 0 and 1.\n");
        exit(1);
    }
    if ((freq > n_gen) || (freq <= 0)) {
        fprintf(stderr, "Frequency of output must be a positive integer no more than the number of generations.\n");
        exit(1);
    }
    else if (n_gen % freq != 0)
        printf("You probably want the frequency of output to divide the number of generations.\n");
    if (n_threads < 1) {
        fprintf(stderr, "Number of threads must be a positive integer.\n");
        exit(1);
    }
	
    // Call code
    run_simulation(graphs_file_name, init_pop_file,
                   output_file_prefix, optimizing_measure_code,
                   pop_size, pop_elite, n_gen, freq,
                   alpha, node_sim_list_file, n_threads);
	
    return 0;
}
