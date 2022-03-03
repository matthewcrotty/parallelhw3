#include "clockcycle.h"
#include <stdio.h>
#include <mpi.h>

int MPI_P2P_Reduce(const void* send_buffer, void* receive_buffer, int count, MPI_Datatype datatype, MPI_Op operation, int root, MPI_Comm communicator){
    printf("Rank %d\n", root);
    if(root == 0){
        for(int i = 1; i < count+1; i++){
            // Somehow this function needs to run in parallel.
            MPI_P2P_Reduce(send_buffer, receive_buffer, count, datatype, operation, i, communicator);
        }
    } else {
        // Loop, where each iteration increase stride by x2
        // Send from 1 stride away
        // Receive from every stride+1
        // Until last, then return to rank 0
        return 1;
    }
    return 1;
}

int main(){

// 1 billion longs
// Somehow initzalise this data so each rank receives it.
long long int* input_data;
long long int* receive_data;

unsigned long long start_time = clock_now();
MPI_P2P_Reduce(&input_data, &receive_data, 4, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
unsigned long long end_time = clock_now();

printf("Completed in %llu cycles\n", (end_time - start_time));
}
