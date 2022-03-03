#include "clockcycle.h"
#include <stdio.h>
#include <mpi.h>

int MPI_P2P_Reduce(const void* send_buffer, void* receive_buffer, int count, MPI_Datatype datatype, MPI_Op operation, int root, MPI_Comm communicator){
    int my_rank;
    MPI_Comm_rank(communicator, &my_rank);
    int world_size;
    MPI_Comm_size(communicator, &world_size);

    // compute sum when implemented
    long long int sum = **(long long int**)send_buffer;

    int still_working = 1;
    for(int stride = 1; stride < world_size-1; stride *= 2){
        MPI_Barrier(MPI_COMM_WORLD);
        if(still_working == 0){
            continue;
        }
        else if(my_rank % (2*stride) != 0){
            //printf("%d sends %lld to %d\n", my_rank, sum, my_rank-stride);
            MPI_Request request;
            MPI_Isend(&sum, 1, datatype, my_rank-stride, 0, communicator, &request);
            still_working = 0;
        } else{
            MPI_Request request;
            long long int data = 0;
            MPI_Irecv(&data, 1, datatype, my_rank+stride, 0, communicator, &request);
            MPI_Wait(&request, MPI_STATUS_IGNORE);
            //printf("%d received %lld from %d\n", my_rank, data, my_rank+stride);
            sum += data;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(my_rank == 0){
        **(long long int**)receive_buffer = sum;
    }

    return 1;
}

int main(int argc, char **argv){

    long long int* input_data;
    long long int* receive_data;
    unsigned int array_size = 1 << 30;
    //unsigned int array_size = 1000;
    int num_elements;

    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if(world_rank == world_size-1){
        num_elements = array_size/world_size + array_size % (world_size);
    } else {
        num_elements = array_size/world_size;
    }
    //printf("Rank %d, num %d\n", world_rank, num_elements);

    // I read this thread and it makes sense, https://submitty.cs.rpi.edu/courses/s22/csci4320/forum/threads/47
    // (I am not that Matthew C, its a very common name) so I just instantiated
    // sections of the array, and summed it up before the P2P reduce.
    // I don't know if that is what the pdf is asking for, but the other solution
    // would be to change P2P reduce to include an arguement for the size of the
    // input buffer and to compute the sum inside the function.
    input_data = (long long int*)malloc(sizeof(long long int) * num_elements);
    for(int i = 0; i < num_elements; i++){
        input_data[i] = i + (num_elements * world_rank);
    }
    receive_data = (long long int*)malloc(sizeof(long long int));
    long long int* local_sum = (long long int*)malloc(sizeof(long long int));
    *local_sum = 0;
    for(int i = 0; i < num_elements; i++){
        *local_sum += input_data[i];
    }
    MPI_Barrier(MPI_COMM_WORLD);


    //printf("Rank %d, sum %lld\n", world_rank, *local_sum);
    unsigned long long start_time = clock_now();
    MPI_P2P_Reduce(&local_sum, &receive_data, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    unsigned long long end_time = clock_now();

    if(world_rank == 0){
        printf("Final Value = %lld\n", *receive_data);
        printf("Completed in %llu cycles\n", (end_time - start_time));
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // long long int result;
    // start_time = clock_now();
    // MPI_Reduce(&local_sum, &result, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    // end_time = clock_now();
    // MPI_Barrier(MPI_COMM_WORLD);
    // if(world_rank == 0){
    //     printf("Final Value = %lld\n", result);
    //     printf("Completed in %llu cycles\n", (end_time - start_time));
    // }

    free(input_data);
    free(receive_data);
    free(local_sum);


    MPI_Finalize();
}
