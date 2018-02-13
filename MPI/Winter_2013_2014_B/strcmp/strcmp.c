#include <stdlib.h>
#include <stdio.h>

#include "mpi.h"
#include "strcmp.h"

#define RESULT_TAG (23)
#define TERMINATE_TAG (42)

int p_strcmp(const char *str1, const char *str2) {
    int current_process_rank;
    int number_of_processes;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &current_process_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);
    
    int send_receive_count[number_of_processes];
    int displacements[number_of_processes];
    
    /*O(p)*/
    for (int i = 0; i < number_of_processes; ++i) {
        displacements[i] =
            0 == i ?
                0 :
                send_receive_count[i - 1] + displacements[i - 1];
        send_receive_count[i] =
            0 == i ?
                0 :
                N / (number_of_processes - 1) + (number_of_processes == i + 1 ? N % number_of_processes - 1 : 0);
    }
    
    if (MANAGER_RANK != current_process_rank) {
        str1 = (char*)malloc(send_receive_count[current_process_rank] * sizeof(*str1));
        str2 = (char*)malloc(send_receive_count[current_process_rank] * sizeof(*str2));
    }
    
    MPI_Scatterv((void*)str1, send_receive_count, displacements, MPI_CHAR,
                 (void*)str1, send_receive_count[current_process_rank], MPI_CHAR,
                 MANAGER_RANK, MPI_COMM_WORLD);
    
    MPI_Scatterv((void*)str2, send_receive_count, displacements, MPI_CHAR,
                 (void*)str2, send_receive_count[current_process_rank], MPI_CHAR,
                 MANAGER_RANK, MPI_COMM_WORLD);
    
    signed char result = 0;
    
    if (MANAGER_RANK == current_process_rank) {
        /*O(p)*/
        for (int rank_to_listen = 1; rank_to_listen < number_of_processes; ++rank_to_listen) {
            
            MPI_Recv(&result, 1, MPI_BYTE,
                     rank_to_listen, RESULT_TAG, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            
            if (result) {
                //Can be moved to slave
                for (int rank_to_notify = rank_to_listen + 1; rank_to_notify < number_of_processes; ++rank_to_notify) {
                    MPI_Request request;
                    
                    MPI_Issend(NULL, 0, MPI_BYTE,
                               rank_to_notify, TERMINATE_TAG, MPI_COMM_WORLD,
                               &request);
                }
                
                break;
            }
        }
    }
    else {
        enum {
            TERMINATE_REQUEST,
            SEND_REQUEST,
            
            REQUESTS_NUMBER
        };
        
        MPI_Request requests[REQUESTS_NUMBER];
        
        MPI_Irecv(NULL, 0, MPI_BYTE,
                  MANAGER_RANK, TERMINATE_TAG, MPI_COMM_WORLD,
                  &requests[TERMINATE_REQUEST]);
        
        /*O(N/p)*/
        for (int i = 0; i < send_receive_count[current_process_rank]; ++i) {
            int flag;
            
            if (str1[i] < str2[i]) {
                result = -1;
                //printf("Rank: %d: is %d: because %s[%d]=%c vs %s[%d]=%c\n", current_process_rank, result, str1, i, str1[i], str2, i, str2[i]);
                
                break;
            }
            else if (str1[i] > str2[i]) {
                result = +1;
                //printf("Rank: %d: is %d: because %s[%d]=%c vs %s[%d]=%c\n", current_process_rank, result, str1, i, str1[i], str2, i, str2[i]);
                
                break;
            }
            
            MPI_Test(&requests[TERMINATE_REQUEST], &flag, MPI_STATUS_IGNORE);
            
            if (flag) {
                break;
            }
        }
        
        MPI_Issend(&result, 1, MPI_BYTE, MANAGER_RANK,
                   RESULT_TAG, MPI_COMM_WORLD,
                   &requests[SEND_REQUEST]);
        
        int index;
        
        MPI_Waitany(REQUESTS_NUMBER, requests, &index, MPI_STATUS_IGNORE);
        
        free((void*)str2);
        free((void*)str1);
    }
    
    return result;
}
