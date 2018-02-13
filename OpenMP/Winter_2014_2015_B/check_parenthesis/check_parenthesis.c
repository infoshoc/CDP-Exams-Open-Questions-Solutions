#include <omp.h>

#include "check_parenthesis.h"

bool check_parenthesis(const char *str, int len)
{
    int *total_sum = NULL;
    int *minimal_sum = NULL;
    int threads_number;
    
#pragma omp parallel
    {
#pragma omp single
        {
            threads_number = omp_get_num_threads();
            total_sum = (int*)malloc(threads_number * sizeof(*total_sum));
            minimal_sum = (int*)malloc(threads_number * sizeof(*minimal_sum));
        }
//implicit #pragma omp barrier and #pragma omp flush
        
        int thread_id = omp_get_thread_num();
        
        total_sum[thread_id] = 0;
        minimal_sum[thread_id] = 0;
        
// splits to sequential chunks O(n/p)
#pragma omp for schedule(static)
        for (int i = 0; i < len; ++i)
        {
            int integer_value = ('(' == str[i]) - (')' == str[i]);
            
            total_sum[thread_id] += integer_value;
            
            if (total_sum[thread_id] < minimal_sum[thread_id])
            {
                minimal_sum[thread_id] = total_sum[thread_id];
            }
        }
    }
    
    int total_total_sum = 0;
    bool result = true;
    
// O(p)
    for (int j = 0; j < threads_number; ++j)
    {
        if (total_total_sum + minimal_sum[j] < 0) {
            result = false;
            
            break;
        }
        
        total_total_sum += total_sum[j];
    }
    
    result &= 0 == total_total_sum;
    
    free(total_sum);
    free(minimal_sum);
    
    return result;
}
