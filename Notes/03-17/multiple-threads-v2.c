/* multiple-threads-v2.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>



/* this function is called by each child thread */
void* whattodo(void* arg);



int main(int argc, char** argv){
  srand(time(NULL)* getpid());

  if (argc!=2){
    fprintf( stderr, "ERROR: invalid arguments\n" );
    fprintf( stderr, "USAGE: %s <#-of-child-threads>\n", *(argv+0) );
    return EXIT_FAILURE;
  }

  int children = atoi(*(argv+1));

  pthread_t* tid = calloc(children, sizeof(pthread_t));

  int i, rc;

  /* create the child threads */
  for(i=0 ; i<children ; i++){
    #if 0
    int t = 10 + ( rand() % 21 );  /* [10,30] seconds */
    printf( "MAIN: next child thread will nap for %d seconds\n", t );
    rc = pthread_create( tid + i, NULL, whattodo, &t );
    #else
    int* t = malloc(sizeof(int));
    *t = 10+(rand()%21);  /* [10,30] seconds */
    printf("MAIN: next child thread will nap for %d seconds\n", *t);
    rc = pthread_create(tid+i, NULL, whattodo, t);
    #endif
    if (rc!=0) {fprintf( stderr, "pthread_create() failed (%d)\n", rc );}
  }
    
  /* wait for the child threads to complete/terminate */
  for (i=0 ; i<children ; i++){
    #if 0
    pthread_join(*(tid+i), NULL);  /* BLOCKING CALL */
    printf("MAIN: joined a child thread\n");
    #else
    int* x;
    pthread_join(*(tid+i), (void**)&x);    /* BLOCKING CALL */
    printf("MAIN: joined a child thread that returned %d\n", *x);
    free(x);
    #endif
  }

  free(tid);

  printf("MAIN: all child threads successfully joined\n");

  return EXIT_SUCCESS;
}


void* whattodo(void* arg){
  int t = *(int*)arg;
  free(arg);

  printf("THREAD: I'm going to nap for %d seconds...\n", t);
  sleep(t);
  printf("THREAD: I'm awake!\n");

  /* dynamically allocate memory to hold return value from this thread */
  int* return_value = malloc(sizeof(int));
  *return_value = t;

  /* terminate this child thread and return the return_value pointer */
  //pthread_exit( return_value );
  return return_value;
  /* the above return will call pthread_exit()... */
}



