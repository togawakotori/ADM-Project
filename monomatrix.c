#include <stdio.h>
#include <malloc.h>

FILE *fin;

int main()
{
int m_a,n_a,m_b,n_b,i,j;

fin=fopen("m.txt","r");
fscanf(fin,"%d %d\n",&m_a,&n_a);
double* a=malloc(sizeof(double)*m_a*n_a);
for (i=0;i<m_a;i++)
for (j=0;j<n_a;j++){
fscanf(fin,"%lf",a+n_a*i*sizeof(double)+j*sizeof(double));
}
//fclose(fin);


for (i=0;i<m_a;i++){
for (j=0;j<n_a;j++){
printf("%f ", *(a+n_a*i*sizeof(double)+j*sizeof(double)));
}
printf("\n");
}

fscanf(fin,"%d %d\n",&m_b,&n_b);
double* b=malloc(sizeof(double)*m_b*n_b);
for (i=0;i<m_b;i++)
for (j=0;j<n_b;j++){
fscanf(fin,"%lf",b+n_b*i*sizeof(double)+j*sizeof(double));
}
fclose(fin);

for (i=0;i<m_b;i++){
for (j=0;j<n_b;j++){
printf("%f ", *(b+n_b*i*sizeof(double)+j*sizeof(double)));
}
printf("\n");
}

if (n_a!=m_b) printf("ERROR!");
else 
{
double* c=malloc(sizeof(double)*m_a*n_b);
for (i=0;i<m_a;i++)
for (j=0;j<n_b;j++){
int k=0;
double produit=0;
for (k=0;k<n_a;k++) produit+=*(a+n_a*i*sizeof(double)+k*sizeof(double))* *(b+n_b*k*sizeof(double)+j*sizeof(double));
*(c+n_b*i*sizeof(double)+j*sizeof(double))=produit;
}
for (i=0;i<m_a;i++){
for (j=0;j<n_b;j++){
printf("%f ", *(c+n_b*i*sizeof(double)+j*sizeof(double)));
}
printf("\n");
}

}

}
