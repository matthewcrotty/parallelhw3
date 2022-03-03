#include "clockcycle.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int MPI_P2P_Reduce(const void* send_buffer, void* receive_buffer, int count, MPI_Datatype datatype, MPI_Op operation, int root, MPI_Comm communicator){
    int my_rank;
    MPI_Comm_rank(communicator, &my_rank);
    int world_size;
    MPI_Comm_size(communicator, &world_size);

    // compute sum when implemented
    long long int sum = *(long long int*)send_buffer;

    // boolean to prevent deadlocks in loop
    int still_working = 1;
    for(int stride = 1; stride < world_size-1; stride *= 2){
        MPI_Barrier(MPI_COMM_WORLD); // Ensure data dependence saftey
        if(still_working == 0){
            continue;
        }
        else if(my_rank % (2*stride) != 0 ){
            MPI_Request request;
            MPI_Isend(&sum, 1, datatype, my_rank-stride, 0, communicator, &request);
            still_working = 0;
        } else{
            MPI_Request request;
            long long int data = 0;
            MPI_Irecv(&data, 1, datatype, my_rank+stride, 0, communicator, &request);
            MPI_Wait(&request, MPI_STATUS_IGNORE);
            sum += data;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // Write output in root rank
    if(my_rank == root){
        *(long long int*)receive_buffer = sum;
    }

    return 1;
}

int main(int argc, char **argv){

    long long int* input_data;
    long long int receive_data = 0;
    unsigned int array_size = 1 << 30;
    int num_elements;
    int clock_frequency = 512000000;

    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    long long int local_sum;
    unsigned long long start_time;
    unsigned long long end_time;
    double time_in_secs;

    // Determine how many elements are in each block
    if(world_rank == world_size-1){
        num_elements = array_size/world_size + array_size % (world_size);
    } else {
        num_elements = array_size/world_size;
    }

    // Initialize all the blocks of data
    input_data = (long long int*)malloc(sizeof(long long int) * num_elements);
    for(int i = 0; i < num_elements; i++){
        input_data[i] = i + (num_elements * world_rank);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Timing P2P including summation
    start_time = clock_now();
    local_sum = 0;
    for(int i = 0; i < num_elements; i++){
        local_sum += input_data[i];
    }
    MPI_P2P_Reduce(&local_sum, &receive_data, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    end_time = clock_now();

    // Output results
    if(world_rank == 0){
        time_in_secs = ((double)(end_time - start_time)) / clock_frequency;
        printf("%llu %f\n", receive_data, time_in_secs);
    }

    // Timing MPI_Reduce including summation
    long long int result = 0;
    start_time = clock_now();
    local_sum = 0;
    for(int i = 0; i < num_elements; i++){
        local_sum += input_data[i];
    }
    MPI_Reduce(&local_sum, &result, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    end_time = clock_now();

    // Output stats
    if(world_rank == 0){
        time_in_secs = ((double)(end_time - start_time)) / clock_frequency;
        printf("%llu %f\n", receive_data, time_in_secs);
    }

    free(input_data);

    MPI_Finalize();
}
