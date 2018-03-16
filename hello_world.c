#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]){
    int id,p,len;
    char name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Get_processor_name(name,&len);

    printf("Hello world from process %d of %d on %s\n",id,p,name);
    fflush(stdout);

    MPI_Finalize();
    return 0;
}


