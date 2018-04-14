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

#define VERSION 4//-v
#define DEBUG 1//DEBUG MODE

void mono(float **, int,int, float **,int,int);
int malloc2dfloat(float ***,int,int);
int free2dfloat(float ***);

int main(int argc, char* argv[])
{
FILE *fin;
double elapsed_time;
int id,p;
int m_a=0,n_a=0,m_b=0,n_b=0,i,j;

//INIITIALIZE
MPI_Init(&argc, &argv);
if (argc!=2){
   if (!id) {printf("PLEASE INPUT COMMAND LINE: %s <FILE> \n",argv[0]);fflush(stdout);}
   MPI_Finalize();
   exit(1);
}

MPI_Barrier(MPI_COMM_WORLD);
elapsed_time = -MPI_Wtime();

MPI_Comm_rank(MPI_COMM_WORLD,&id);
if (!id) printf("VERSION %d\n",VERSION);

MPI_Comm_size(MPI_COMM_WORLD,&p);
int p_sqrt=(int)sqrt(p);
 
//PROCESS 0 READ DATA

if (!id){fin=fopen(argv[1],"r");fscanf(fin,"%d %d\n",&m_a,&n_a);}

MPI_Bcast(&m_a,1,MPI_INT,0,MPI_COMM_WORLD);
MPI_Bcast(&n_a,1,MPI_INT,0,MPI_COMM_WORLD);
MPI_Barrier(MPI_COMM_WORLD); 

if (MIN(m_a,n_a)<p_sqrt) {  
   if (!id) {printf("FATAL ERROR: TOO MANY PROCESS\n");fflush(stdout);fclose(fin);}
   MPI_Finalize();
   exit(2);
}


float **a;malloc2dfloat(&a,m_a,n_a);
float **a_chip;malloc2dfloat(&a_chip,BLOCK_SIZE((int)id/p_sqrt,p_sqrt,m_a),BLOCK_SIZE(id%p_sqrt,p_sqrt,n_a));//LOCAL

float **a_buffer[p];//Storage a_chip[p/p_sqrt][p%p_sqrt] 
if (!id)
for (i=0;i<p;i++)
malloc2dfloat(&a_buffer[i],BLOCK_SIZE((int)i/p_sqrt,p_sqrt,m_a),BLOCK_SIZE(i%p_sqrt,p_sqrt,n_a));
 
if(!id) for (i=0;i<m_a;i++) for (j=0;j<n_a;j++)
{
fscanf(fin,"%f",&(a[i][j]));
a_buffer[BLOCK_OWNER(i,p_sqrt,m_a)*p_sqrt+BLOCK_OWNER(j,p_sqrt,n_a)][i-BLOCK_LOW(BLOCK_OWNER(i,p_sqrt,m_a),p_sqrt,m_a)][j-BLOCK_LOW(BLOCK_OWNER(j,p_sqrt,n_a),p_sqrt,n_a)]=a[i][j];  
}


if (!id){fscanf(fin,"\n"); fscanf(fin,"%d %d\n",&m_b,&n_b);}

MPI_Bcast(&m_b,1,MPI_INT,0,MPI_COMM_WORLD);
MPI_Bcast(&n_b,1,MPI_INT,0,MPI_COMM_WORLD);
MPI_Barrier(MPI_COMM_WORLD);
 
if (n_a!=m_b) {  
   if (!id) {printf("FATAL ERROR: N_A!=M_B\n");fflush(stdout);fclose(fin);}
   MPI_Finalize();
   exit(3);
}
if (MIN(MIN(m_a,n_a),MIN(m_b,n_b))<p_sqrt) {  
   if (!id) {printf("\nFATAL ERROR: TOO MANY PROCESS\n");fflush(stdout);fclose(fin);}
   MPI_Finalize();
   exit(2);
}

float **b;malloc2dfloat(&b,m_b,n_b);
float **b_chip;malloc2dfloat(&b_chip,BLOCK_SIZE((int)id/p_sqrt,p_sqrt,m_b),BLOCK_SIZE(id%p_sqrt,p_sqrt,n_b));
float **b_buffer[p];

if (!id) for (i=0;i<p;i++) malloc2dfloat(&b_buffer[i],BLOCK_SIZE((int)i/p_sqrt,p_sqrt,m_b),BLOCK_SIZE(i%p_sqrt,p_sqrt,n_b));//ALLOCATE BUFFER

if(!id) for (i=0;i<m_b;i++) for (j=0;j<n_b;j++)
{fscanf(fin,"%f",&(b[i][j]));
b_buffer[BLOCK_OWNER(i,p_sqrt,m_b)*p_sqrt+BLOCK_OWNER(j,p_sqrt,n_b)][i-BLOCK_LOW(BLOCK_OWNER(i,p_sqrt,m_b),p_sqrt,m_b)][j-BLOCK_LOW(BLOCK_OWNER(j,p_sqrt,n_b),p_sqrt,n_b)]=b[i][j];}

if(!id) fclose(fin);

float **c;malloc2dfloat(&c,m_a,n_b);
float **c_chip;malloc2dfloat(&b_chip,BLOCK_SIZE((int)id/p_sqrt,p_sqrt,m_a),BLOCK_SIZE(id%p_sqrt,p_sqrt,n_b));
float **c_buffer[p];

if (!id) for (i=0;i<p;i++) malloc2dfloat(&c_buffer[i],BLOCK_SIZE((int)i/p_sqrt,p_sqrt,m_a),BLOCK_SIZE(i%p_sqrt,p_sqrt,n_b));//ALLOCATE BUFFER

//DISTRIBUTE
if (!id){
a_chip=a_buffer[0];
for (i=1;i<p;i++) MPI_Send(&(a_buffer[i][0][0]),BLOCK_SIZE((int)i/p_sqrt,p_sqrt,m_a)*BLOCK_SIZE(i%p_sqrt,p_sqrt,n_a),MPI_FLOAT,i,0,MPI_COMM_WORLD);
}
else {
 MPI_Status status;
 MPI_Recv(&a_chip[0][0],BLOCK_SIZE((int)id/p_sqrt,p_sqrt,m_a)*BLOCK_SIZE(id%p_sqrt,p_sqrt,n_a),MPI_FLOAT,0,0,MPI_COMM_WORLD,&status);
}

if (!id){
b_chip=b_buffer[0];
for (i=1;i<p;i++) MPI_Send(&(b_buffer[i][0][0]),BLOCK_SIZE((int)i/p_sqrt,p_sqrt,m_b)*BLOCK_SIZE(i%p_sqrt,p_sqrt,n_b),MPI_FLOAT,i,0,MPI_COMM_WORLD);
}
else {
 MPI_Status status;
 MPI_Recv(&b_chip[0][0],BLOCK_SIZE((int)id/p_sqrt,p_sqrt,m_b)*BLOCK_SIZE(id%p_sqrt,p_sqrt,n_b),MPI_FLOAT,0,0,MPI_COMM_WORLD,&status);
}

MPI_Barrier(MPI_COMM_WORLD);

//EXCHANGE
printf("EXCHANGE READY %d\n",id);
//NOW BLOCK C_I,J has BLOCK A_I,J BLOCK B_I,J
for (k=1;k<=p_sqrt;k++){//ROUND k
//SEND a_chip, b_chip to NEXT

int send_m_a=BLOCK_SIZE((int)id/p_sqrt,p_sqrt,m_a);
int send_n_a=BLOCK_SIZE((id%p_sqrt + (k-1) * (int)(id/p_sqrt))%p_sqrt,p_sqrt,n_a);
int recv_m_a=BLOCK_SIZE((int)id/p_sqrt,p_sqrt,m_a);
int recv_n_a=BLOCK_SIZE((id%p_sqrt + (k) * (int)(id/p_sqrt))%p_sqrt,p_sqrt,n_a);
int arrv=;//to        id/p_sqrt,(id+id/p_sqrt)%p_sqrt
int vien=;//from      id/p_sqrt,(id+id/p_sqrt)%p_sqrt

float **recv_buf_a;malloc2dfloat(&recv_buf_a, recv_m_a,recv_n_a);

float **recv_buf_b;malloc2dfloat(&recv_buf_b,BLOCK_SIZE(((int)id/p_sqrt + k * (id%p_sqrt))%p_sqrt,p_sqrt,m_b),BLOCK_SIZE((id%p_sqrt,p_sqrt,n_b));

MPI_Status status;
MPI_Send(&a_chip[0][0],send_m_a*send_n_a,MPI_FLOAT,to?,0,MPI_COMM_WORLD);
MPI_Recv(&recv_buf_a[0][0],recv_m_a*recv_n_a,MPI_FLOAT,from?,0,MPI_COMM_WORLD,&status);

//k^th step C[.,.] need A BLOCK
//(int)id/p_sqrt 
//(id%p_sqrt + k * (int)(id/p_sqrt)) % p_sqrt
//B BLOCk
//((int)id/p_sqrt + k * (id%p_sqrt)) % p_sqrt
//id%p_sqrt

free2dfloat(&a_chip);
malloc2dfloat(&a_chip,BLOCK_SIZE((int)id/p_sqrt,p_sqrt,m_a),BLOCK_SIZE((id%p_sqrt + (k) * (int)(id/p_sqrt))%p_sqrt,p_sqrt,n_a));
a_chip=recv_buf_a;


MPI_Barrier(MPI_COMM_WORLD);
}


//TERMINATE
elapsed_time+=MPI_Wtime();

if (!id) {printf("EXECUTION TIME (s): %10.6f\n",elapsed_time);fflush(stdout);}

MPI_Finalize();

#ifdef DEBUG
if (!id) {printf("\nMONO\n");mono(a,m_a,n_a,b,m_b,n_b);}
#endif
 
 return 0;
}

void mono(float **a, int m_a,int n_a, float **b,int m_b,int n_b)
{
int i,j,k;
float **c_mono;
malloc2dfloat(&c_mono,m_a,n_b);
for (i=0;i<m_a;i++)
  for (j=0;j<n_b;j++)
     c_mono[i][j]=0;
for (i=0;i<m_a;i++)
  for (j=0;j<n_b;j++)
    for (k=0;k<n_a;k++)
     c_mono[i][j]+=a[i][k]*b[k][j];
for (i=0;i<m_a;i++){for (j=0;j<n_b;j++){printf("%f ", c_mono[i][j]);}printf("\n");}
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



