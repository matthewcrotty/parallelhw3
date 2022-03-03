#include "clockcycle.h"
#include <stdio.h>
#include <mpi.h>

int MPI_P2P_Reduce(const void* send_buffer, void* receive_buffer, int count, MPI_Datatype datatype, MPI_Op operation, int root, MPI_Comm communicator){
    int my_rank;
    MPI_Comm_rank(communicator, &my_rank);
    int world_size;
    MPI_Comm_size(communicator, &world_size);
    //printf("in p2p Rank %d\n", my_rank);

    // compute sum when implemented
    long long sum = 0;

    for(int stride = 1; stride < world_size-1; stride *= 2){
        if(my_rank % (2*stride) != 0){
            printf("%d sends to %d\n", my_rank, my_rank-stride);
            MPI_Request request;
            MPI_Isend(send_buffer, 1, datatype, my_rank-stride, 0, communicator, &request);
            break;
        } else{
            printf("%d receives from %d\n", my_rank, my_rank+stride);
            MPI_Request request;
            long long data;
            MPI_Irecv(&data, 1, datatype, my_rank+stride, 0, communicator, &request);
        }
    }

    return 1;
}

int main(int argc, char **argv){
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    printf("Size %d, rank %d\n", world_size, world_rank);
    if(world_rank == 0){

    }
    // 1 billion longs
    // Somehow initzalise this data so each rank receives it.
    long long int* input_data;
    long long int* receive_data;

    unsigned long long start_time = clock_now();
    MPI_P2P_Reduce(&input_data, &receive_data, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    unsigned long long end_time = clock_now();

    printf("Completed in %llu cycles\n", (end_time - start_time));
    MPI_Finalize();
}
