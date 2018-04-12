#include <stdio.h>
#include <malloc.h>
#include <mpi.h>
#include <math.h>// To compile with math.h use: mpicc prime.c -o prime -lm
#include <stdlib.h>


#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a<b?b:a)
#define BLOCK_LOW(id,p,n) MAX((id*n)/p,0)//this expression replaces (id*n)/p to fix a serious bug: while p=1, BLOCK_LOW=-1.
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id+1),p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id+1),p,n)-BLOCK_LOW(id,p,n))
#define BLOCK_OWNER(index,p,n) (p*(index+1)-1)/n 

#define VERSION 1//-v

//#define DEBUG 1

void SayHello(int,int,int,int);
int malloc2dfloat(float ***,int,int);
int free2dfloat(float ***);


int main(int argc, char* argv[])
{
double elapsed_time;
int id,p;

int m_a=0,n_a=0,m_b=0,n_b=0,i,j;

MPI_Init(&argc, &argv);
MPI_Barrier(MPI_COMM_WORLD);
elapsed_time = -MPI_Wtime();
MPI_Comm_rank(MPI_COMM_WORLD,&id);
MPI_Comm_size(MPI_COMM_WORLD,&p);


FILE *fin;
if (!id){fin=fopen("m.txt","r");fscanf(fin,"%d %d\n",&m_a,&n_a);}

MPI_Bcast(&m_a,1,MPI_INT,0,MPI_COMM_WORLD);
MPI_Bcast(&n_a,1,MPI_INT,0,MPI_COMM_WORLD);
MPI_Barrier(MPI_COMM_WORLD);

int low_value  = BLOCK_LOW(id,p,m_a);//interval for one process 
int high_value = BLOCK_HIGH(id,p,m_a);
SayHello(id,p,low_value,high_value);

float **a;malloc2dfloat(&a,m_a,n_a);
float **a_local;malloc2dfloat(&a_local,BLOCK_SIZE(id,p,m_a),n_a);
 
#ifdef DEBUG
printf("INFO A %d %d %d\n",id,m_a,n_a);
printf("ALLOCATE PROCESS %d SPACE %d\n",id,sizeof(float)*BLOCK_SIZE(id,p,m_a)*n_a);
#endif 


if(!id){
for (i=0;i<m_a;i++)
  for (j=0;j<n_a;j++)
   {fscanf(fin,"%f",&(a[i][j]));}

#ifdef DEBUG
for (i=0;i<m_a;i++){for (j=0;j<n_a;j++){printf("%f ", a[i][j]);}printf("\n");}//print matrix A
#endif 
}


if (!id){
a_local=a;
for (i=1;i<p;i++) MPI_Send(&(a[low_value][0]),BLOCK_SIZE(i,p,m_a)*n_a,MPI_FLOAT,i,0,MPI_COMM_WORLD);
}
else {
 MPI_Status status;
 MPI_Recv(&(a_local[0][0]),BLOCK_SIZE(id,p,m_a)*n_a,MPI_FLOAT,0,0,MPI_COMM_WORLD,&status);
}
//MPI_Bcast(&(a[0][0]), m_a*n_a,MPI_FLOAT,0,MPI_COMM_WORLD);
//MPI_Barrier(MPI_COMM_WORLD);


#ifdef DEBUG
for (i=0;i<BLOCK_SIZE(id,p,m_a);i++){for (j=0;j<n_a;j++){printf("%f ", a_local[i][j]);}printf("Watashi %d\n",id);}//print matrix A_local
#endif

if (!id){fscanf(fin,"\n"); fscanf(fin,"%d %d\n",&m_b,&n_b);}

MPI_Bcast(&m_b,1,MPI_INT,0,MPI_COMM_WORLD);
MPI_Bcast(&n_b,1,MPI_INT,0,MPI_COMM_WORLD);
//MPI_Barrier(MPI_COMM_WORLD);

float **b;malloc2dfloat(&b,m_b,n_b);
float **c;malloc2dfloat(&c,m_a,n_b);
float **c_local;malloc2dfloat(&c_local,BLOCK_SIZE(id,p,m_a),n_b);

#ifdef DEBUG
printf("INFO B %d %d %d\n",id,m_b,n_b);
#endif

if (n_a!=m_b) { printf("ERROR!\n");fclose(fin);}
else 
{ 
if(!id){for (i=0;i<m_b;i++)for (j=0;j<n_b;j++)fscanf(fin,"%f",&(b[i][j]));fclose(fin);}

MPI_Bcast(&(b[0][0]), m_b*n_b,MPI_FLOAT,0,MPI_COMM_WORLD);
//MPI_Barrier(MPI_COMM_WORLD);

for (i=0;i<BLOCK_SIZE(id,p,m_a);i++)
for (j=0;j<n_b;j++){
int k=0;
float produit=0;
for (k=0;k<n_a;k++) produit+=a_local[i][k]*b[k][j];
c_local[i][j]=produit;
}
//
#ifdef DEBUG
printf("INFO C %d %d %d\n",id,m_a,n_b);
#endif
if (!id){
 MPI_Status status;
for (i=0;i<BLOCK_SIZE(0,p,m_a);i++) for (j=0;j<n_b;j++) c[i][j]=c_local[i][j];
#ifdef DEBUG
printf("PROCESS 0 COPY C_LOCAL TO C\n");
#endif
for (i=1;i<p;i++)
 MPI_Recv(&(c[BLOCK_LOW(i,p,m_a)][0]),BLOCK_SIZE(i,p,m_a)*n_b,MPI_FLOAT,i,0,MPI_COMM_WORLD,&status);
}
else {
MPI_Send(&(c_local[0][0]),BLOCK_SIZE(id,p,m_a)*n_b,MPI_FLOAT,0,0,MPI_COMM_WORLD);
}
#ifdef DEBUG
printf("END PROCESS %d\n",id);
#endif
}

if (!id) for (i=0;i<m_a;i++){for (j=0;j<n_b;j++){printf("%f ", c[i][j]);}printf("\n");}
elapsed_time+=MPI_Wtime();
if (!id) {printf("EXECUTION TIME (s): %10.6f\n",elapsed_time);fflush(stdout);}

 free2dfloat(&a);free2dfloat(&b);free2dfloat(&c);free2dfloat(&a_local);free2dfloat(&c_local);
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


int malloc2dfloat(float ***array, int n, int m) {    
    float *p = (float *)malloc(n*m*sizeof(float));
    if (!p) return -1;

    (*array) = (float **)malloc(n*sizeof(float*));
    if (!(*array)) {
       free(p);
       return -1;
    }

    for (int i=0; i<n; i++) 
       (*array)[i] = &(p[i*m]);

    return 0;
}

int free2dfloat(float ***array) {free(&((*array)[0][0]));free(*array);return 0;}


