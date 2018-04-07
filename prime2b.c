#include <mpi.h>
#include <math.h>// To compile with math.h use: mpicc prime.c -o prime -lm
#include <stdio.h>
#include <stdlib.h>

#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a<b?b:a)
#define BLOCK_LOW(id,p,n) MAX((id*n)/p,0)//this expression replaces (id*n)/p to fix a serious bug: while p=1, BLOCK_LOW=-1.
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id+1),p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id+1),p,n)-BLOCK_LOW(id,p,n))
#define BLOCK_OWNER(index,p,n) (p*(index+1)-1)/n 

#define VERSION 3//-v

void SayHello(int,int,int,int);

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
n = atoi(argv[1]);//input the number n
int n0= n;// backup of n
// exclude 2 
// the result of n=2k is the same as the one of n=2k-1
// we should examine 3,5,...,2k-1
//              corr 0,1,...,k-2 
if (n%2==0) {n=(n-1)/2;}
       else {n=n/2;}
 



int low_value = 3+2*BLOCK_LOW(id,p,n);//interval for one process, here n is the number of prime from 3 to n0;
int high_value = 3+2*BLOCK_HIGH(id,p,n);

SayHello(id,p,low_value,high_value);

//if ((2+BLOCK_LOW(1,p,n-1))*(2+BLOCK_LOW(1,p,n-1))<n){
if (3+2*BLOCK_LOW(1,p,n)<(int)sqrt((double)n0)){
//BLOCK_LOW(1,p,n-1) Size of Process 0
   if (!id) {printf("TOO MANY PROCESS\n");fflush(stdout);}
   MPI_Finalize();
   exit(1);
}

//printf("BLOCK SIZE %d\n",(int)BLOCK_SIZE(id,p,n-1));
marked = (char*) malloc((int)BLOCK_SIZE(id,p,n));
if (marked == NULL){
   printf("CANNOT ALLOCATE ENOUGH MEMORY\n");fflush(stdout);
   MPI_Finalize();
   exit(1); 
}
for (i=0;i<BLOCK_SIZE(id,p,n);i++) marked[i]=0;

if (!id) index=0;// Only Process 0 uses index

prime=3;
do {
    //determine first (local) 
    if (prime*prime>low_value) first = prime*prime-low_value; //eg.prime=5, low_value=21 , first=4 
    else {
        if (low_value%prime==0) first = 0;//eg.prime=5, low_value=30, first=0
        else first = prime-(low_value%prime);//eg.prime=5, low_value=31, first=4
    }
   if ((first+low_value)%2==0) first=first+prime;// to promise "first" is an odd number
 //  printf("PRIME %d LOW %d FIRST %d\n",prime,low_value,first);
   for (i=first;i<2*BLOCK_SIZE(id,p,n);i+=2*prime){marked[(i)/2]=1;//not prime
//3,5,7,9,11,13,15,17,19
//0,1,2,3,4, 5, 6, 7, 8
//printf("%d %d %d\n",i,i+low_value, (i+1)/2);
  }
   if (!id){while (marked[++index]);prime=2*index+3;//printf("%d\n",prime);
}//Process 0 find next prime
 
   MPI_Bcast(&prime,1,MPI_INT,0,MPI_COMM_WORLD);//and Broadcast to all processes

}while (prime*prime<=n0);


local_count=0;
//printf("Block Size %d\n",BLOCK_SIZE(id,p,n));
for (i=0;i<BLOCK_SIZE(id,p,n);i++) if (!marked[i]) local_count++;
//printf("%d\n",local_count);
MPI_Reduce(&local_count,&global_count,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);

elapsed_time+=MPI_Wtime();
if (!id){
global_count=global_count+1;//count 2
printf("%d PRIMES ARE <= %d\n",global_count,n0);fflush(stdout);
printf("EXECUTION TIME (s): %10.6f\n",elapsed_time);fflush(stdout);
}
 

 MPI_Finalize();
 return 0;
}
void SayHello(int id, int p,int low_value, int high_value){
  int len;
  char name[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(name,&len);
  printf("PROCESS %d/%d INTERVAL %d~%d ON %s\n",id,p,low_value,high_value,name);
  fflush(stdout);
}
