#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <omp.h>

#include "sort.h"
#include "mergesort_parallel.h"

static int *mergesort_parallel_aux(int *a, int size, int number_of_threads) {
    if (number_of_threads <= 1 || size <= 1)
    {
        return sort(a, size);
    }
    
    int left_size = size / 2;
    int right_size = size - left_size;
    int left_number_of_threads = number_of_threads / 2;
    int right_number_of_threads = number_of_threads  - left_number_of_threads;
    int *left_pointer;
    int *right_pointer;
    
#pragma omp parallel sections
    {
#pragma omp section
        {
            left_pointer = mergesort_parallel_aux(a, left_size, left_number_of_threads);
        }
#pragma omp section
        {
            right_pointer = mergesort_parallel_aux(a + left_size, right_size, right_number_of_threads);
        }
    }
    
    return merge(left_pointer, left_size, right_pointer, right_size);
}

int *mergesort_parallel(int *a, int size) {
    int *result;
    //assert(1 == __builtin_popcount(size));
    
#pragma omp parallel
    {
        int number_of_threads = omp_get_num_threads();
        
        //assert(1 == __builtin_popcount(number_of_threads));
        omp_set_nested(true);
        
        result = mergesort_parallel_aux(a, size, number_of_threads);
    }
    
    return result;
}
