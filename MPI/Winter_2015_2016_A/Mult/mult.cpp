//
//  mult.cpp
//  hw3
//
//  Created by Volodymyr Polosukhin on 11/02/2018.
//  Copyright © 2018 infoshoc. All rights reserved.
//

#include <string.h>
#include <stdlib.h>

#include "mpi.h"

#include "fs.h"
#include "mult.h"

void mult(){
    int r, w;
    MPI_Comm_rank(MPI_COMM_WORLD, &r);
    MPI_Comm_size(MPI_COMM_WORLD, &w);
    int n;
    int *A, *B, *R;
    if (r == 0) { // Receive input from filesystem
        n = fs_read_input_size();
        A = fs_read_A();
        B = fs_read_B();
        R = (int*)malloc(sizeof(int) * n * n);
    }
    
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    int number_of_rows = n / w;
    
    if (0 != r)
    {
        A = (int*)malloc(sizeof(int) * number_of_rows * n);
        B = (int*)malloc(sizeof(int) * n * n);
        R = (int*)malloc(sizeof(int) * number_of_rows * n);
    }
    
    MPI_Scatter((void*)A, number_of_rows * n, MPI_INT,
                (void*)A, number_of_rows * n, MPI_INT,
                0, MPI_COMM_WORLD);
    
    MPI_Bcast(B, n * n, MPI_INT,
              0, MPI_COMM_WORLD);
    
    for (int row = 0; row < number_of_rows; ++row)
    {
        for (int col = 0; col < n; ++col)
        {
            R[row * n + col] = 0;
            
            for (int k = 0; k < n; ++k)
            {
                R[row * n + col] += A[row * n + k] * B[k * n + col];
            }
        }
    }
    
    MPI_Gather(R, number_of_rows * n, MPI_INT,
               R, number_of_rows * n, MPI_INT,
               0, MPI_COMM_WORLD);
    
    if (r == 0) {
        fs_write_result(R);
    }
    
    free(R);
    free(B);
    free(A);
}
