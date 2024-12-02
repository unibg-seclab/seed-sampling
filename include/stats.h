#ifndef STATS_H
#define STATS_H

#include "types.h"

struct stat_list {
	double mean;
	double std;
	unsigned long long peak;
};

unsigned long long peak(unsigned long long *obs, unsigned int n);

double mean(unsigned long long *obs, unsigned int n);

double std(unsigned long long *obs, unsigned int n);

void compute_stats(struct stat_list *sl, unsigned long long *obs, unsigned int n);

#endif
