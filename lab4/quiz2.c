//Cho biết chương trình sau thực thi thì kết quả như thế nào? Điền tất cá các giải thích và nộp lại file .c

/* Giải thích 1 
    Khai bao
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct 
 {
   double      *a;
   double      *b;
   double     sum; 
   int     veclen; 
 } DOTDATA;

/* giải thích 2 */

#define MAXTHRDS 8
#define VECLEN 100
DOTDATA dotstr; 
pthread_t callThd[MAXTHRDS];
pthread_mutex_t mutexsum;

/*
Giải thích 3
*/

void *dotprod(void *arg)
{

   /* Define and use local variables for convenience */

   int i, start, end, len ;
   long offset;
   double mysum, *x, *y;
   offset = (long)arg;
     
   len = dotstr.veclen;
   start = offset*len;
   end   = start + len;
   x = dotstr.a;
   y = dotstr.b;

   /*
  Giải thích 4
   */

   mysum = 0;
   for (i=start; i<end ; i++) 
    {
      mysum += (x[i] * y[i]);
    }

   /*
   Giải thích 5
   */
   pthread_mutex_lock (&mutexsum);
   printf("Thread %ld adding partial sum of %f to global sum of %f\n", arg, mysum, dotstr.sum);
   dotstr.sum += mysum;
   pthread_mutex_unlock (&mutexsum);

   pthread_exit((void*) 0);
}

/* 
Giải thích 6
*/

int main (int argc, char *argv[])
{
long i;
double *a, *b;
void *status;
pthread_attr_t attr;

/* Assign storage and initialize values */
a = (double*) malloc (MAXTHRDS*VECLEN*sizeof(double));
b = (double*) malloc (MAXTHRDS*VECLEN*sizeof(double));
  
for (i=0; i<VECLEN*MAXTHRDS; i++) {
  a[i]=1;
  b[i]=a[i];
  }

dotstr.veclen = VECLEN; 
dotstr.a = a; 
dotstr.b = b; 
dotstr.sum=0;

pthread_mutex_init(&mutexsum, NULL);
         
/* Create threads to perform  */
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

for(i=0;i<MAXTHRDS;i++) {
  /* Each thread works on a different set of data.
  The offset is specified by 'i'. The size of
  the data for each thread is indicated by VECLEN.
  */
  pthread_create( &callThd[i], &attr, dotprod, (void *)i); 
  }

pthread_attr_destroy(&attr);

/* Wait on the other threads */
for(i=0;i<MAXTHRDS;i++) {
  pthread_join( callThd[i], &status);
  }

/* After joining, print out the results and cleanup */
printf ("Done. Threaded version: sum =  %f \n", dotstr.sum);
free (a);
free (b);
pthread_mutex_destroy(&mutexsum);
pthread_exit(NULL);
}   