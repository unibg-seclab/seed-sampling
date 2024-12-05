#include <math.h>

#include "stats.h"
#include "types.h"

unsigned long long peak(unsigned long long *obs, unsigned int n) {

        unsigned long long p = 0;
        for (unsigned int i = 0; i < n; i++)
                if (obs[i] > p)
                        p = obs[i];
        return p;
}

double mean(unsigned long long *obs, unsigned int n) {

        double m = 0;
        for (unsigned int i = 0; i < n; i++)
                m += (double)obs[i] / n;
        return m;
}

double std(unsigned long long *obs, unsigned int n) {

        if (n == 1)
                return 0.0;
        double m   = mean(obs, n);
        double std = 0;
        for (unsigned int i = 0; i < n; i++)
                std += pow((double)obs[i] - m, 2) / n;
        return sqrt(std);
}

void compute_stats(struct stat_list *sl, unsigned long long *obs, unsigned int n) {

        sl->peak = peak(obs, n);
        sl->mean = mean(obs, n);
        sl->std  = std(obs, n);
}
