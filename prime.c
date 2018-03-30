#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_LOW(id,p,n) (id*n)/p
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id+1),p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id+1),p,n)-BLOCK_LOW(id,p,n))
#define BLOCK_OWNER(index,p,n) (p*(index+1)-1)/n 
#define MIN(a,b) (a<b?a:b)
#define VERSION 1//-v

int main(int argc, char* argv[]){

double elapsed_time;
int local_count,global_count;
int first;//local
int i,id,index;
char* marked;
int n,p,proc0_size,prime,size;

MPI_Init(&argc, &argv);
MPI_Barrier(MPI_COMM_WORLD);
elapsed_time = -MPI_Wtime();
MPI_Comm_rank(MPI_COMM_WORLD,&id);
MPI_Comm_size(MPI_COMM_WORLD,&p);

if (argc!=2){
   if (!id) {printf("PLEASE INPUT COMMAND LINE: %s <number> \n",argv[0]);fflush(stdout);}
   MPI_Finalize();
   exit(1);
}
n = atoi(argv[1]);

  int len;
  char name[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(name,&len);
  printf("Hello world from process %d of %d on %s\n",id,p,name);
  fflush(stdout);


int low_value = 2+BLOCK_LOW(id,p,n-1);//interval for one process
int high_value = 2+BLOCK_HIGH(id,p,n-1);
//printf("INTERVAL %d~%d\n",low_value,high_value);
 
if ((2+BLOCK_LOW(1,p,n-1))*(2+BLOCK_LOW(1,p,n-1))<n){//BLOCK_LOW(1,p,n-1) Size of Process 0
   if (!id) {printf("TOO MANY PROCESS\n");fflush(stdout);}
   MPI_Finalize();
   exit(1);
}

//printf("BLOCK SIZE %d\n",(int)BLOCK_SIZE(id,p,n-1));
marked = (char*) malloc((int)BLOCK_SIZE(id,p,n-1));
if (marked == NULL){
   printf("CANNOT ALLOCATE ENOUGH MEMORY\n");fflush(stdout);
   MPI_Finalize();
   exit(1); 
}
for (i=0;i<BLOCK_SIZE(id,p,n-1);i++) marked[i]=0;

if (!id) index=0;// Only Process 0 uses index

prime=2;
do {
    //determine first (local) 
    if (prime*prime>low_value) first = prime*prime-low_value; //eg.prime=5, low_value=21 , first=4 
    else {
        if (low_value%prime==0) first = 0;//eg.prime=5, low_value=30, first=0
        else first = prime-(low_value%prime);//eg.prime=5, low_value=31, first=4
    }
   
   for (i=first;i<BLOCK_SIZE(id,p,n-1);i+=prime) marked[i]=1;//not prime

   if (!id){while (marked[++index]);prime=index+2;}//Process 0 find next prime
   MPI_Bcast(&prime,1,MPI_INT,0,MPI_COMM_WORLD);//and Broadcast to all processes

}while (prime*prime<=n);

local_count=0;
for (i=0;i<BLOCK_SIZE(id,p,n-1);i++) if (!marked[i]) local_count++;

MPI_Reduce(&local_count,&global_count,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);

elapsed_time+=MPI_Wtime();
if (!id){

printf("%d PRIMES ARE <= %d\n",global_count,n);fflush(stdout);
printf("EXECUTION TIME (s): %10.6f\n",elapsed_time);fflush(stdout);
}
 

 MPI_Finalize();
 return 0;
}
