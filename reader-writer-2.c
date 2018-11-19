/*
*
*   Authors Steven Faulkner, Kyle Barrington, Amir Aziz
*           Amir Seifpour, Calvin Chan
*
*   COP 4600 Final Project: Producer COnsumer program with 
*                           with emphasis given to the writers.
*
*
*   Complie: gcc reader-writer-2.c -lpthread -o main
*   Run: ./main (number of readers) (number of writers)
*
*       @number of readers: an integer number representing the number
*                            of reader threads to be created.
*   
*       @number of writers: an integer number representing the number 
*                            of writer threads to be created.
*/


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define true 1
#define false 0


int Readers_In_Queue = 0;
int Writers_In_Queue = 0;
int Readers_In_Library = 0;
int Writers_In_Library = 0;
pthread_mutex_t variableMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t libraryMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t entryCond = PTHREAD_COND_INITIALIZER;
unsigned long uSecSleep = 1500000;
double duration;


int perror_exit(char* errorStr) 
{
/*	Helper function for perror 
 *	handles the error message and exits 
 *	program on fault
 */

    perror(errorStr);
    exit(EXIT_FAILURE);
}



void *readerFunction(void* arg) {
/*
*
*   Start routine for reader threads runs an infinite loop
*   
*    @void*  arg: passed input argument is thread number.
*
*
*/

    //clock initialization
    clock_t  begin;

    begin = clock();

    while (true) 
    {

	/*locks mutex, adds reader to queue
	 * then unlocks mutex
	 */

	
        pthread_mutex_lock(&variableMutex);
           
        Readers_In_Queue++;
           
        printf("ReaderQueue: %d WriterQueue: %d [in: R:%d W:%d]\n",
                Readers_In_Queue, Writers_In_Queue, Readers_In_Library, Writers_In_Library);
       
    
        pthread_mutex_unlock(&variableMutex);



         
        pthread_mutex_lock(&libraryMutex);
        pthread_mutex_lock(&variableMutex);
                
                 /* Writeres are given highest priority
		  * if any writers are waiting, lock the library 
		  * to keep readers from entering 
		  */
            while (Writers_In_Queue || Writers_In_Library) 
            {   
                
                    pthread_mutex_unlock(&variableMutex);
                    pthread_cond_wait(&entryCond, &libraryMutex);
                    pthread_mutex_lock(&variableMutex);
            }

            Readers_In_Queue--;
            Readers_In_Library++;
                
            printf("ReaderQueue: %d WriterQueue: %d [in: R:%d W:%d]\n",
                    Readers_In_Queue, Writers_In_Queue, Readers_In_Library, Writers_In_Library);
            
        	
        pthread_mutex_unlock(&variableMutex);
        pthread_mutex_unlock(&libraryMutex);
        pthread_cond_broadcast(&entryCond);

        // Simulate reading from file/data from  buffer 
        usleep(rand() % uSecSleep);

        pthread_mutex_lock(&libraryMutex);
           
        pthread_mutex_lock(&variableMutex);
               
        Readers_In_Library--;
                
            printf("ReaderQueue: %d WriterQueue: %d [in: R:%d W:%d]\n",
                    Readers_In_Queue, Writers_In_Queue, Readers_In_Library, Writers_In_Library);
           
	    pthread_mutex_unlock(&variableMutex);
       
        pthread_mutex_unlock(&libraryMutex);
        pthread_cond_broadcast(&entryCond);

        // Sleep and enter the queue again 
        usleep(rand() % uSecSleep);


	//each thread will run for 10 seconds 
	//before breaking out of loop and killing thread
	if ((double)(clock() - begin) >= duration)
	{
		break;
	}
	else
	{
		continue;
	}
    }

    pthread_exit(kill);
}

void *writerFunction(void* arg){

/*
 *	Start routine for writer threads runs an infinite loop
 *
 *	@void* arg: passed input argument is thread number
 *
 *
 */
    //clock innitalization	
    clock_t begin;

    begin = clock();

    while (true) 
    {
        

	/* lock mutex, add writer to queue
	 * then unlock mutex
	 */
        pthread_mutex_lock(&variableMutex);
           
        Writers_In_Queue++;
            
        printf("ReaderQueue: %d WriterQueue: %d [in: R:%d W:%d]\n",
                Readers_In_Queue, Writers_In_Queue, Readers_In_Library, Writers_In_Library);
       
   
       pthread_mutex_unlock(&variableMutex);

        

        pthread_mutex_lock(&libraryMutex);
           
        pthread_mutex_lock(&variableMutex);
                
            while (Readers_In_Library || Writers_In_Library) 
            {
                
                    pthread_mutex_unlock(&variableMutex);
                    pthread_cond_wait(&entryCond, &libraryMutex);
                    pthread_mutex_lock(&variableMutex);
            }
               
            Writers_In_Queue--;
            Writers_In_Library++;
           

            printf("ReaderQueue: %d WriterQueue: %d [in: R:%d W:%d]\n",
                    Readers_In_Queue, Writers_In_Queue, Readers_In_Library, Writers_In_Library);
           
        /* We dont brodcast an unblock of our conditional here 
	 * since other writers might be waiting to enter the library
	 */
        pthread_mutex_unlock(&variableMutex);
        pthread_mutex_unlock(&libraryMutex);

        //Simulate writing from file /data from buffer 
        usleep(rand() % uSecSleep);

        pthread_mutex_lock(&libraryMutex);

      	pthread_mutex_lock(&variableMutex);
                
        Writers_In_Library--;
           

        printf("ReaderQueue: %d WriterQueue: %d [in: R:%d W:%d]\n",
                Readers_In_Queue, Writers_In_Queue, Readers_In_Library, Writers_In_Library);
            
        
        pthread_mutex_unlock(&variableMutex);
        
        pthread_mutex_unlock(&libraryMutex);
        pthread_cond_broadcast(&entryCond);

        // Sleep and enter the queue again 
        usleep(rand() % uSecSleep);


	//each threads will only run for 10 seconds 
	// before breaking out of loop and killing thread
	if ((double)(clock() - begin) >= duration)
	{
		break;
	}
	else
	{
		continue;
	}	
			

    }

    pthread_exit(kill);
}

int main(int argc, char** argv) {
   
    printf("\n**********Readers-Writers with writers priority*********\n");
    
    int userReaderCount, userWriterCount;
    

    /*
    *   check for runtime arguments, prompt user for
    *   parameters if they return null.
    *
    */
    if ((argv[1] == NULL) || (argv[2]) == NULL || (argv[3] == NULL ))
    {
        printf("Number of readers = ");
        if (scanf("%d", &userReaderCount) == EOF) 
		perror_exit("scanf");
        
	printf("Number of writers = ");
        if (scanf("%d", &userWriterCount) == EOF) 
		perror_exit("scanf");
        printf("Duration of each thread = ");
	if( scanf("%lf", &duration) == EOF)
		perror_exit("scanf");

	printf("Starting program\n\n");
        
	sleep(1);

    } 
    else 
    {
        
        userReaderCount = atoi(argv[1]);
        userWriterCount = atoi(argv[2]);
        
	printf("Number of Readers = %d\n", userReaderCount);
        printf("Number of Writers = %d\n", userWriterCount);
        printf("Starting program\n\n");
        
	sleep(1);
    }
    
    //random seed
    srand(time(NULL));
    
    //allocation and initialization for read/write threads
    pthread_t *readerThread = calloc(userReaderCount, sizeof(pthread_t));
    pthread_t *writerThread = calloc(userWriterCount, sizeof(pthread_t));
    
    // cast between void * and long are same size - int throws warnings
    long i = 0;

    printf("ReaderQueue: %d WriterQueue: %d [in: R:%d W:%d]\n",
            Readers_In_Queue, Writers_In_Queue, Readers_In_Library, Writers_In_Library);
   
    
    //create reader threads for total number 
    // of readers
    for (i = 0; i < userReaderCount; ++i) 
    {
        if (pthread_create(&readerThread[i], NULL, readerFunction, (void*)i)) 
        {
            perror_exit("Error while creating reader thread (pthread_create)");
        }
	
    }
   
    // create writer threads for total number 
    // of writers
    for (i = 0; i < userWriterCount; ++i) 
    {
        if (pthread_create(&writerThread[i], NULL, writerFunction, (void*)i)) 
        {
            perror_exit("Error while creating writer thread (pthread_create)");
        }
    }

   //wait for reader threads to terminate
    for (i = 0; i < userReaderCount; ++i) 
    {
    
        if (pthread_join(readerThread[i], NULL)) 
        {  
            perror_exit("Error while waiting for reader thread termination (pthread_join)");
        }

	
    }
   
   free(readerThread);
   
    //wait for writer threads to terminate
    for (i = 0; i < userWriterCount; ++i) 
    {
        if (pthread_join(writerThread[i], NULL)) 
        {
            perror_exit("Error while waiting for writer thread termination (pthread_join)");
        }


    }
    

    // clean up
    free(writerThread);
    pthread_cond_destroy(&entryCond);
    pthread_mutex_destroy(&variableMutex);
    pthread_mutex_destroy(&libraryMutex);
}
