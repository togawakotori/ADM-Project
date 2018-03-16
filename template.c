#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]){

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    //im back!
    
    MPI_Finalize();
    return 0;
}


