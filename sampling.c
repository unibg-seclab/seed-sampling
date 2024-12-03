#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#include "io_functions.h"
#include "stats.h"
#include "types.h"
#include "utils.h"

struct config {
	size_t page_size; // bytes
	size_t seed_pages;
	size_t entropy_pages;
	char *fname;
	long reps;
};

void print_stats(struct stat_list *sl) {

	printf("\tpeak:\t%llu [ns] \n", sl->peak);
	printf("\tmean:\t%.2f [ns] \n", sl->mean);
	printf("\tstd:\t%.2f [ns] \n", sl->std);
}

void estimate(struct config *cfg, struct stat_list *sl, double multiplier) {

	size_t tmp_buf_size = 11;
	char tmp_buf[tmp_buf_size];
	pprint_size(tmp_buf, tmp_buf_size, cfg->page_size * cfg->seed_pages * multiplier);
	printf("\tto extract %s", tmp_buf);
	pprint_size(tmp_buf, tmp_buf_size, cfg->page_size * cfg->entropy_pages * multiplier);
	printf(" from %s you need", tmp_buf);
	printf(" %.3f±%.3f [s]\n", sl->mean * multiplier / 1e9, sl->std / 1e9);
}

int parse_size_t(size_t *out, char *in) {
	if (sscanf(in, "%zu", out) == 1) {
		return 0;
	} else {
		return 1;
	}
}

// Parses the parameters. Returns 1 in case parameters are invalid, 0
// on success.
int parse(struct config *params, int argc, char **argv) {

	if (argc < 6){
		printf("Missing arguments: fname[char *], page_size[size_t], seed_pages[size_t], entropy_pages[size_t], reps[size_t]\n");
		return 1;
	}
	params->fname = argv[1];
	
	if (parse_size_t(&params->page_size, argv[2])) {
		printf("Invalid page_size\n");
		return 1;
	}
	if (parse_size_t(&params->seed_pages, argv[3])) {
		printf("Invalid number of seed_pages\n");
		return 1;
	}
	if (parse_size_t(&params->entropy_pages, argv[4]) || params->entropy_pages < params->seed_pages){
		printf("Invalid number of entropy_pages\n");
		return 1;
	}

	params->reps = strtol(argv[5], NULL, 10);
	if (params->reps < 1) {
		printf("Invalid number of repetitions, must be positive\n");
		return 1;
	}

	printf("[i] info\n");
	printf("\tfname:\t\t%s\n", params->fname);
	printf("\tpage_size:\t%zu [B]\n", params->page_size);
	printf("\tseed_pages:\t%zu\n", params->seed_pages);
	printf("\tentropy_pages:\t%zu\n", params->entropy_pages);
	printf("\treps:\t\t%zu\n", params->reps);

	return 0;
}

int main(int argc, char **argv) {

	// program status
	int STATUS = 0;
	// input params
	struct config params;
	// stats
	unsigned long long *obs = NULL;
	struct stat_list sl;
	// human readable entropy size print
	FILE *f = NULL;
	size_t tmp_buf_size = 11;
	char tmp_buf[tmp_buf_size];
	// seed buffer
	byte *seed = NULL;
        //  sampling indexes
	byte *indexes = NULL;
	size_t indexes_size = 0;

	// parse parameters
	STATUS = parse(&params, argc, argv);
	if (STATUS == 1){
		goto clean;
	}
	// allocate space for the observations
	obs = malloc(sizeof(unsigned long long) * params.reps);
	// set the correct multiplier
	double multiplier = 1024 * 1024 * 1024 / ((double)params.seed_pages * params.page_size);
	

	// print the approx. seed size
	pprint_size(tmp_buf, tmp_buf_size, params.seed_pages*params.page_size);
	printf("\tseed_size:\t%s (approx.)\n", tmp_buf);
	
	// open the entropy file and print its size
	f = fopen(params.fname, "r");
	if (f == NULL){
		printf("[err] entropy file not found\n");
		STATUS = 1;
		goto clean;
	}
	pprint_size(tmp_buf, tmp_buf_size, get_file_size(f));
	printf("\tentropy_size:\t%s (approx.)\n", tmp_buf);

	// allocate space for the seed
	seed = malloc(params.page_size*params.seed_pages);	

        /* measure the latency of the kernel+hardware */
	printf("[t] single_fseek\n");
	for (int rep = 0; rep < params.reps; rep++){
		obs[rep] = MEASURE({
				if (-1 == single_fseek(f))
					goto clean;
			});
		compute_stats(&sl, obs, params.reps);
	}
	print_stats(&sl);
	printf("[t] single_fread\n");
	for (int rep = 0; rep < params.reps; rep++){
		obs[rep] = MEASURE({
				if (-1 == single_fread(f, seed, params.page_size))
					goto clean;
			});
		compute_stats(&sl, obs, params.reps);
	}
	print_stats(&sl);
	

	/* measure the time required to create the seed with a linear scan*/
	pprint_size(tmp_buf, tmp_buf_size, params.page_size);	
	printf("[t] scan %zu pages of %s (reading: %2.5f%%)\n",
	       params.seed_pages,
	       tmp_buf,
	       ((double)params.seed_pages * 100)/params.entropy_pages);
	for (int rep = 0; rep < params.reps; rep++){
		obs[rep] = MEASURE({
				if (-1 == scan(f, seed, params.page_size, params.seed_pages))
					goto clean;
			});
		compute_stats(&sl, obs, params.reps);
	}
	print_stats(&sl);
	estimate(&params, &sl, multiplier);	

	/* measure the time required to create the seed with random reads */
	pprint_size(tmp_buf, tmp_buf_size, params.page_size);
	printf("[t] random_read %zu pages of %s (sampling: %2.5f%%)\n",
	       params.seed_pages,
	       tmp_buf,
	       ((double)params.seed_pages * 100)/params.entropy_pages);
	// allocate random indexes (each index is a 4-bytes bytestring)
	indexes_size = sizeof(unsigned int) * params.seed_pages;
	indexes = get_random_memory(indexes_size);
	if (indexes == NULL){
		STATUS = 1;
		goto clean;
	}
	// random reads
	for (int rep = 0; rep < params.reps; rep++){
		obs[rep] = MEASURE({
				if (-1 == random_read(f, seed, indexes, params.page_size, params.seed_pages))
					goto clean;
			});
		compute_stats(&sl, obs, params.reps);
	}
	print_stats(&sl);
	estimate(&params, &sl, multiplier);
	
	
clean:
	if (seed != NULL){
		explicit_bzero(seed, params.page_size*params.seed_pages);		
		free(seed);
	}
	if (indexes != NULL){
		explicit_bzero(indexes, indexes_size);		
		free(indexes);
	}
	if (f != NULL)
		fclose(f);
	if (obs != NULL)
		free(obs);
	
	return STATUS;
}
