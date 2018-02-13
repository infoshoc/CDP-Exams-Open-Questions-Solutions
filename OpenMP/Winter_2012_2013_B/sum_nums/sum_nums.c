#include <omp.h>
#include <limits.h>

#include "sum_nums.h"
#include "common.h"


void sum_nums(unsigned int *a, unsigned int *b, unsigned int *result, int n)
{
    unsigned int was_carry_to_result[2][n];
    bool *was_carry_to_thread_number_to_will_be_carry[2];
    bool *thread_number_to_was_actually_carry;
    
#pragma omp parallel
    {
        int number_of_threads = omp_get_num_threads();
        int thread_number = omp_get_thread_num();
        int elements_per_thread = (n + thread_number - 1) / number_of_threads;
        
#pragma omp single
        {
            for (int was_carry = 0; was_carry < 2; ++was_carry)
            {
                was_carry_to_thread_number_to_will_be_carry[was_carry] = (bool*)malloc(number_of_threads * sizeof(*was_carry_to_thread_number_to_will_be_carry));
            }
            
            thread_number_to_was_actually_carry = (bool*)malloc(number_of_threads * sizeof(thread_number_to_was_actually_carry));
        }
//implicit barrier and flush
        
        int begin_element = min(n, elements_per_thread * thread_number);
        int end_element = min(n, begin_element + elements_per_thread);
        
        /*O(n/p)*/
        for (int was_carry = 0; was_carry < 2; ++was_carry)
        {
            long long carry = was_carry;
            
            for (int element = begin_element; element < end_element; ++element)
            {
                carry += (long long) a[element] + b[element];
                
                was_carry_to_result[was_carry][element] = carry % INT_MAX;
                carry /= INT_MAX;
            }
            
            was_carry_to_thread_number_to_will_be_carry[was_carry][thread_number] = carry;
        }
        
#pragma omp single
        {
            /*O(p)*/
            thread_number_to_was_actually_carry[0] = false;
            
            for (int thread = 1; thread < number_of_threads; ++thread)
            {
                thread_number_to_was_actually_carry[thread] = was_carry_to_thread_number_to_will_be_carry[thread_number_to_was_actually_carry[thread - 1]][thread - 1];
            }
            
            result[n] = was_carry_to_thread_number_to_will_be_carry[thread_number_to_was_actually_carry[number_of_threads - 1]][number_of_threads - 1];
        }
//implicit barrier and flush
        
        /*O(n/p)*/
        for (int element = begin_element; element < end_element; ++element)
        {
            result[element] = was_carry_to_result[thread_number_to_was_actually_carry[thread_number]][element];
        }
    }
    
    free(thread_number_to_was_actually_carry);
    
    for (int was_carry = 0; was_carry < 2; ++was_carry)
    {
        free(was_carry_to_thread_number_to_will_be_carry[was_carry]);
    }
}
