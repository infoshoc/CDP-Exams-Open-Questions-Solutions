#include <stdio.h>
#include <omp.h>
#include <stdbool.h>
#include <assert.h>

#include "disk.h"
#include "queue.h"

int main() {
    struct queue q;
    int number_producers_finished = 0;
#pragma omp parallel
    {
        
#pragma omp single
        {
            init_queue(&q);
#pragma omp flush (q)
        }
//implicit #pragma omp barrier
        
        int thread_id = omp_get_thread_num();
        int number_of_threads = omp_get_num_threads();
        bool is_consumer = thread_id < number_of_threads / 2;
        int number_producers = number_of_threads - number_of_threads / 2;
        void *v = NULL;
        
        while (number_producers_finished < number_producers)
        {
            if (is_consumer) {
                while (number_producers_finished < number_producers && is_queue_empty(&q));
                
                //printf("Consumer %d is out of busy wait\n", thread_id);
            } else {
                while (is_queue_full(&q));
#pragma omp critical
                {
                    v = disk_read();
                    
                    if (NULL == v)
                    {
                        ++number_producers_finished;
                    }
                }
                
                if (NULL == v) {
                    break;
                }
            }
            
            
#pragma omp critical
            {
                bool result = true;
                
                if (is_consumer) {
                    if (!is_queue_empty(&q)) {
                        result = dequeue(&q, &v);
                    }
                }
                else {
                    if (!is_queue_full(&q)) {
                        result = enqueue(&q, v);
                    }
                }
                
                assert(result);
#pragma omp flush (q)
                //printf("Flushed by %d [%p]=q{.head=%d, .tail=%d\n", thread_id, &q, q.head, q.tail);
            }
            
            if (is_consumer) {
                process_block(v);
            }
        }
    }
    return 0;
}
