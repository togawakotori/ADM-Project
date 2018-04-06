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
void SayHello(int,int);

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
//exclude 2,3
//6k+1,6k+5
//
//5,7,11,13,17,19,23,25,29
//0,1,2, 3, 4, 5, 6, 7, 8
n=NumberOfTest(n);

//printf("N=%d\n",RecoverNumber(n-1));

//SayHello(id,p);

int low_value  = RecoverNumber(BLOCK_LOW(id,p,n));//interval for one process 
int high_value = RecoverNumber(BLOCK_HIGH(id,p,n));
printf("PROCESS %d/%d INTERVAL %d~%d\n",id,p,low_value,high_value);
 
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

prime=5;
do {
    //determine first (local) 
    if (prime*prime>low_value) first = prime*prime-low_value; //eg.prime=5, low_value=21 , first=4 
    else {
        if (low_value%prime==0) first = 0;//eg.prime=5, low_value=30, first=0
        else first = prime-(low_value%prime);//eg.prime=5, low_value=31, first=4
    }
   while (((first+low_value)%2==0)||((first+low_value)%3==0)) first+=prime;// to promise "first" is an correct number
 //  printf("PRIME %d LOW %d FIRST %d\n",prime,low_value,first);
   for (i=low_value+first;i<high_value+1;i+=((((i/prime)%6==1)?4:2)*prime)){marked[NumberOfTest(i)-NumberOfTest(low_value)]=1;//not prime

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
if (n0<=1) global_count = -2;//treat the special case, n<=1
if (n0==2) global_count = -1;//treat the special case, n=2
global_count=global_count+2;//count 2,3
printf("%d PRIMES ARE <= %d\n",global_count,n0);fflush(stdout);
printf("EXECUTION TIME (s): %10.6f\n",elapsed_time);fflush(stdout);
}
 

 MPI_Finalize();
 return 0;
}
int NumberOfTest(int n){
  switch (n%6){
    case 0:return (2*(n/6)-1);break;
    case 1:return (2*((n-1)/6));break;//NumberOfTest(n-1)+1;
    case 2:return (2*((n-2)/6));break;//NumberOfTest(n-2)+1;
    case 3:return (2*((n-3)/6));break;//NumberOfTest(n-3)+1;
    case 4:return (2*((n-4)/6));break;//NumberOfTest(n-4)+1;
    case 5:return (2*((n-5)/6)+1);break;//NumberOfTest(n-5)+2;
  }
}
int RecoverNumber(int n){
if (n%2==0) return (n/2)*6+5;
       else return ((n-1)/2)*6+7;//RecoverNumber(n-1)+2;           
}
void SayHello(int id, int p){
  int len;
  char name[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(name,&len);
  printf("Hello world from process %d of %d on %s\n",id,p,name);
  fflush(stdout);
}
