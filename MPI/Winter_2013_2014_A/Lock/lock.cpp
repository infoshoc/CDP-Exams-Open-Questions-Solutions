//
//  lock.cpp
//  hw3
//
//  Created by Volodymyr Polosukhin on 12/02/2018.
//  Copyright © 2018 infoshoc. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "lock.hpp"
#include "mpi.h"
#include "macros.h"


#define MPI_LOCK_TAG (42)
#define MPI_UNLOCK_TAG (23)

static bool initialize_unlock_requests();

static bool unlock_reqest_sent[LOCKS_NUMBER];
static MPI_Request unlock_requests[LOCKS_NUMBER];
static bool unlock_requests_initialized = initialize_unlock_requests();

static bool initialize_unlock_requests()
{
    memset(unlock_reqest_sent, false, sizeof(unlock_requests));
    
    return true;
}

void dlock(int i)
{
    MPI_Request lock_request;
    
    MPI_ASSERT(0 <= i && i < LOCKS_NUMBER);
    
    if (unlock_reqest_sent[i])
    {
        MPI_Wait(unlock_requests + i, MPI_STATUS_IGNORE);
        unlock_reqest_sent[i] = false;
    }
    
    MPI_Irecv(NULL, 0, MPI_BYTE, LOCK_MANAGER,
              MPI_LOCK_TAG, MPI_COMM_WORLD,
              &lock_request);
    
    MPI_Ssend(&i, 1, MPI_INT,
              LOCK_MANAGER, MPI_LOCK_TAG, MPI_COMM_WORLD);
    
    MPI_Wait(&lock_request, MPI_STATUS_IGNORE);
}

void dunlock(int i)
{
    unlock_reqest_sent[i] = true;
    
    MPI_Issend(&i, 1, MPI_INT, LOCK_MANAGER,
               MPI_UNLOCK_TAG, MPI_COMM_WORLD, &unlock_requests[i]);
}

void lock_manager()
{
    int number_of_threads;
    int thread_holding_lock[LOCKS_NUMBER];
    
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_threads);
    
    bool is_thread_waiting_for_lock[number_of_threads][LOCKS_NUMBER];
    
    memset(is_thread_waiting_for_lock, false, sizeof(**is_thread_waiting_for_lock) * number_of_threads * LOCKS_NUMBER);
    memset(thread_holding_lock, LOCK_MANAGER, sizeof(thread_holding_lock));
    
    while (true) {
        MPI_Request request;
        MPI_Status status;
        int i;
        
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&i, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        switch (status.MPI_TAG) {
            case MPI_LOCK_TAG:
            {
                if (status.MPI_SOURCE == thread_holding_lock[i] || LOCK_MANAGER == thread_holding_lock[i])
                {
                    thread_holding_lock[i] = status.MPI_SOURCE;
                    
                    MPI_Irsend(NULL, 0, MPI_BYTE,
                               status.MPI_SOURCE, MPI_LOCK_TAG, MPI_COMM_WORLD,
                               &request);
                }
                else
                {
                    is_thread_waiting_for_lock[status.MPI_SOURCE][i] = true;
                }
                
                break;
            }
            
            case MPI_UNLOCK_TAG:
            {
                MPI_ASSERT(thread_holding_lock[i] == status.MPI_SOURCE);
                
                thread_holding_lock[i] = LOCK_MANAGER;
                
                for (int rank = 0; rank < number_of_threads; ++rank) {
                    if (is_thread_waiting_for_lock[rank][i]) {
                        is_thread_waiting_for_lock[rank][i] = false;
                        thread_holding_lock[i] = status.MPI_SOURCE;
                        
                        MPI_Irsend(NULL, 0, MPI_BYTE,
                                   rank, MPI_LOCK_TAG, MPI_COMM_WORLD,
                                   &request);
                    }
                }
                
                break;
            }
                
            default:
                break;
        }
    }
}
