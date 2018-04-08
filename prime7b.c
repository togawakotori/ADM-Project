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

#define VERSION 4//-v

int NumberOfTest(int);
int RecoverNumber(int);
void SayHello(int,int,int,int);
int FindNext(int);

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
 
n=NumberOfTest(n);

int low_value  = RecoverNumber(BLOCK_LOW(id,p,n));//interval for one process 
int high_value = RecoverNumber(BLOCK_HIGH(id,p,n));

SayHello(id,p,low_value,high_value);

if (RecoverNumber(BLOCK_LOW(1,p,n))<(int)sqrt((double)n0)){//Size of Process 0 < sqrt(n0) ?
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

prime=11;
do {
    //determine first (local) 
    if (prime*prime>low_value) first = prime*prime-low_value; //eg.prime=5, low_value=21 , first=4 
    else {
        if (low_value%prime==0) first = 0;//eg.prime=5, low_value=30, first=0
        else first = prime-(low_value%prime);//eg.prime=5, low_value=31, first=4
    }
   while ((((first+low_value)%2==0)||((first+low_value)%3==0))||(((first+low_value)%5==0)||((first+low_value)%7==0))) first+=prime;// to promise "first" is an correct number
 //  printf("PRIME %d LOW %d FIRST %d\n",prime,low_value,first);
   for (i=low_value+first;i<high_value+1;i+=FindNext((i/prime)%210)*prime){marked[NumberOfTest(i)-NumberOfTest(low_value)]=1;//not prime 

//printf("%d %d\n",i, NumberOfTest(i)-NumberOfTest(low_value));
  }
   if (!id){while (marked[++index]);prime=RecoverNumber(index);//printf("%d\n",prime);
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
if (n0<=6) global_count = -1;//special case
if (n0<=4) global_count = -2; 
if (n0<=2) global_count = -3; 
if (n0<=1) global_count = -4; 

global_count=global_count+4;//count 2,3,5,7
printf("%d PRIMES ARE <= %d\n",global_count,n0);fflush(stdout);
printf("EXECUTION TIME (s): %10.6f\n",elapsed_time);fflush(stdout);
}

 MPI_Finalize();
 return 0;
}


int add[]={-1,0,0,0,0,0,0,0,0,0,0,1,1,2,2,2,2,3,3,4,4,4,4,5,5,5,5,5,5,6,6,7,7,7,7,7,7,8,8,8,8,9,9,10,10,10,10,11,11,11,11,11,11,12,12,12,12,12,12,13,13,14,14,14,14,14,14,15,15,15,15,16,16,17,17,17,17,17,17,18,18,18,18,19,19,19,19,19,19,
20,20,20,20,20,20,20,20,21,21,21,21,22,22,23,23,23,23,24,24,25,25,25,25,26,26,26,26,26,26,26,26,27,27,27,27,27,27,28,28,28,28,29,29,29,29,29,29,30,30,31,31,31,31,32,32,32,32,32,32,33,33,34,34,34,34,34,34,35,35,35,35,35,35,
36,36,36,36,37,37,38,38,38,38,39,39,39,39,39,39,40,40,41,41,41,41,41,41,42,42,42,42,43,43,44,44,44,44,45,45,46,46,46,46,46,46,46,46,46,46,47}; 
int mask[]={11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,121,127,131,137,139,143,149,151,157,163,167,169,173,179,181,187,191,193,197,199,209,211};

int NumberOfTest(int n){return (48*(n/210)+add[n%210]);}
int RecoverNumber(int n){return ((n/48)*210+mask[n%48]);}
int FindNext(int n){return (mask[add[n]]-n);}
void SayHello(int id, int p,int low_value, int high_value){
  int len;
  char name[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(name,&len);
  printf("PROCESS %d/%d INTERVAL %d~%d ON %s\n",id,p,low_value,high_value,name);
  fflush(stdout);
}
