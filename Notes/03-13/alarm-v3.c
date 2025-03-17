/* alarm-v3.c -- multi-threaded solution */

/* TO DO: fix any other memory leaks in this code...for each thread! */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAXLINE 80

typedef struct
{
  int seconds;
  char msg[MAXLINE];
} alarm_t;

/* function executed by each thread */
void * alarm_thread_function( void * arg )
{
  alarm_t * alarm = (alarm_t *)arg;

  pthread_t mytid = pthread_self();   /* get my thread ID */

  /* pthread_detach() enables pthreads to reclaim the
   *  thread resources when this thread terminates,
   *   i.e., do not leave a "zombie" thread behind...
   *
   * we therefore do not need to call pthread_join()
   */
  pthread_detach( mytid );

  printf( "Alarm set for %d seconds\n", alarm->seconds );
  sleep( alarm->seconds );
  fprintf( stderr, "ALARM (%d): %s\n", alarm->seconds, alarm->msg );

  free( alarm );

  return NULL;
}

int main()
{
  const int maxline = MAXLINE;

  char * format_string = calloc( maxline, sizeof( char ) );
  sprintf( format_string, "%%d %%79[^\n]" );

  char * line = calloc( maxline, sizeof( char ) );
  char * msg = calloc( maxline, sizeof( char ) );

  while ( 1 )
  {
    int seconds;

    printf( "Set alarm (<sec> <msg>) -- 0 to exit: " );

    if ( fgets( line, maxline - 1, stdin ) == NULL ) return EXIT_FAILURE;

    if ( strlen( line ) <= 1 ) continue;   /* skip blank lines */

    if ( sscanf( line, format_string, &seconds, msg ) < 2 || seconds < 0 )
    {
      if ( seconds == 0 ) break;
      fprintf( stderr, "ERROR: invalid alarm request\n" );
    }
    else
    {
      pthread_t tid;  /* thread ID */

#if 0
      /* create the next alarm */
      alarm_t alarm;
      alarm.seconds = seconds;
      strcpy( alarm.msg, msg );
#else
      /* dynamically allocate memory for the next alarm thread */
      alarm_t * alarm = malloc( sizeof( alarm_t ) );
      alarm->seconds = seconds;
      strcpy( alarm->msg, msg );
#endif

      int rc = pthread_create( &tid, NULL, alarm_thread_function, alarm );

      if ( rc != 0 )
      {
        fprintf( stderr, "ERROR: pthread_create() failed (%d): %s\n",
                 rc, strerror( rc ) );
        return EXIT_FAILURE;
      }

      usleep( 1000 );
      *msg = '\0';
    }
  }

  free( format_string );
  free( msg );
  free( line );

  return EXIT_SUCCESS;
}