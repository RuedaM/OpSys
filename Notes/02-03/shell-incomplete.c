/* shell-incomplete.c */

/* TO DO: add the ability to pipe commands together, e.g.,
 *
 *    bash$ ps -ef | grep goldsd
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <ctype.h>
 #include <string.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 
 #define MAXLINE 1024
 
 int main()
 {
   char * line = calloc( MAXLINE, sizeof( char ) );
 
   if ( line == NULL )
   {
     perror( "calloc() failed" );
     return EXIT_FAILURE;
   }
 
   int rc;
   int nbg = 0;   /* number of background processes */
 
   while ( 1 )
   {
     while ( nbg > 0 )
     {
       int status;
       pid_t child_pid = waitpid( -1, &status, WNOHANG );  /* NON-BLOCKING */
                              /*  ^^
                               * wait for any of my child processes
                               *  to terminate...
                               */
 
       if ( child_pid == -1 )
       {
         perror( "waitpid() failed" );
         return EXIT_FAILURE;
       }
 
       if ( child_pid == 0 ) break;
 
       if ( child_pid > 0 )
       {
         nbg--;
         printf( "<background process terminated " );
 
         if ( WIFSIGNALED( status ) )
         {
           printf( "abnormally>\n" );
         }
         else if ( WIFEXITED( status ) )
         {
           printf( "with exit status %d>\n", WEXITSTATUS( status ) );
         }
       }
     }
 
     /* Display the shell prompt and read in a line of input from the user */
     printf( "$ " );
 
     if ( fgets( line, MAXLINE, stdin ) == NULL )
     {
       fprintf( stderr, "fgets() failed\n" );
       break;
     }
 
     /* TO DO: make this code more robust in handling spaces in the input */
 
     /* Remove trailing '\n' and/or space characters */
     int len = strlen( line );
     while ( len > 0 && isspace( *(line+len-1) ) )
     {
       *(line+len-1) = '\0';
       len--;
     }
     if ( len == 0 ) continue;
 
 #ifdef DEBUG_MODE
     printf( "line: \"%s\"\n", line );
 #endif
 
     /* Handle the special shell commands, i.e., exit and cd */
     if ( strcmp( line, "exit" ) == 0 ) break;
 
     if ( strncmp( line, "cd", 2 ) == 0 )
     {
       char * location = NULL;
       if ( len == 2 ) location = getenv( "HOME" ); else location = line + 3;
       rc = chdir( location );
       if ( rc == -1 ) fprintf( stderr, "cd: %s: No such file or directory\n", location );
       continue;
     }
 
 
     /* Determine if the command is to be run in the background */
     int background = *(line+len-1) == '&';
 
     if ( background )
     {
       nbg++;
       do { *(line+len-1) = '\0'; len--; } while ( isspace( *(line+len-1) ) );
     }
 
 
     /* Identify how many arguments this command has, though we
      *  assume exactly and only one space char as delimiter
      */
     int anyargs = 1;
     char * space = strchr( line, ' ' );
     if ( space == NULL ) { space = line + strlen( line ); anyargs = 0; }
     *space = '\0';
 
     if ( background )
     {
       printf( "<running background process \"%s\">\n", line );
     }
 
     /* Create a child process to execute the command */
     pid_t p = fork();
 
     if ( p == -1 ) { perror( "fork() failed" ); return EXIT_FAILURE; }
 
     if ( p > 0 )  /* PARENT process -- shell */
     {
       if ( !background )  /* run the command in the foreground... */
       {
         pid_t child_pid = waitpid( p, NULL, 0 );
 
         if ( child_pid == -1 )
         {
           perror( "waitpid() failed" );
           return EXIT_FAILURE;
         }
 
         /* TO DO: expand by copy-and-pasting WIFSIGNALED() etc. */
 
 
 
       }
     }
     else  /* CHILD process */
     {
       int argc = 1;
 
       /* Determine argument count, assuming exactly one space as delimiter */
       if ( anyargs )
       {
         for ( char * ptr = space + 1 ; ptr ; argc++ )
         {
           ptr = strchr( ptr, ' ' );
           if ( ptr ) ptr++;
         }
       }
 
 
       /* Construct argument vector, assuming exactly one space as delimiter */
       char ** argv = calloc( argc + 1, sizeof( char * ) );
       *(argv+0) = calloc( strlen( line ) + 1, sizeof( char ) );
       if ( *(argv+0) == NULL ) { perror( "calloc() failed" ); return EXIT_FAILURE; }
       strcpy( *(argv+0), line );
 
       for ( int i = 1 ; i < argc ; i++ )
       {
         char * argument = space + 1;
         space = strchr( argument, ' ' );
         if ( space ) *space = '\0';
         *(argv+i) = calloc( strlen( argument ) + 1, sizeof( char ) );
         if ( *(argv+i) == NULL ) { perror( "calloc() failed" ); return EXIT_FAILURE; }
         strcpy( *(argv+i), argument );
       }
 
 
       execvp( line, argv );
       perror( "execvp() failed" );
       return EXIT_FAILURE;
     }
   }
 
   free( line );
 
   return EXIT_SUCCESS;
 }
 