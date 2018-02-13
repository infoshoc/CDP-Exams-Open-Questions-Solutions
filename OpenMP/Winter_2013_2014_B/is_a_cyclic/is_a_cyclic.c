#include <stdbool.h>
#include <stdio.h>
#include "is_a_cyclic.h"

bool is_a_cyclic(void)
{
    bool result = true;
    
#pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < N; ++i) {
//Read result from memory
#pragma omp flush (result)
        if (!result) {
            continue;
        }
        
        int number_of_visited_elements = 0;
        int j = i;
        
        do
        {
            ++number_of_visited_elements;
            j = (j + a[j]) % N;
//Read result from memory
#pragma omp flush (result)
        }
        while (number_of_visited_elements <= N && result && i != j);
        
        if (N < number_of_visited_elements)
        {
            printf("Problem on index:%d\n", i);
            
            result = false;
//Write result to memory
#pragma omp flush(result)
        }
    }
    
    return result;
}
