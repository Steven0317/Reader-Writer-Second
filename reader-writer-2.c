
/* 
 * Copyright 2014 Damian Terlecki.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

int readersInQueue = 0;
int writersInQueue = 0;
int readersInLibrary = 0;
int writersInLibrary = 0;
pthread_mutex_t varMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t libraryMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t entryCond = PTHREAD_COND_INITIALIZER;
unsigned long uSecSleep = 2000000;

perror_exit(char* errorStr) {
    perror(errorStr);
    exit(EXIT_FAILURE);
}

void *readerF(void* arg) {
    while (1) {
        pthread_mutex_lock(&varMutex);
            readersInQueue++;
            printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",
                    readersInQueue, writersInQueue, readersInLibrary, writersInLibrary);
        pthread_mutex_unlock(&varMutex);

        pthread_mutex_lock(&libraryMutex);
            pthread_mutex_lock(&varMutex);
                while (writersInQueue || writersInLibrary) {    // Writers have higher prioritu
                    pthread_mutex_unlock(&varMutex);
                    pthread_cond_wait(&entryCond, &libraryMutex);
                    pthread_mutex_lock(&varMutex);
                }
                readersInQueue--;
                readersInLibrary++;
                printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",
                        readersInQueue, writersInQueue, readersInLibrary, writersInLibrary);
            pthread_mutex_unlock(&varMutex);
        pthread_mutex_unlock(&libraryMutex);
        pthread_cond_broadcast(&entryCond);

        /* Reading... */
        usleep(rand() % uSecSleep);

        pthread_mutex_lock(&libraryMutex);
            pthread_mutex_lock(&varMutex);
                readersInLibrary--;
                printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",
                        readersInQueue, writersInQueue, readersInLibrary, writersInLibrary);
            pthread_mutex_unlock(&varMutex);
        pthread_mutex_unlock(&libraryMutex);
        pthread_cond_broadcast(&entryCond);

        /* Sleep and enter the queue again */
        usleep(rand() % uSecSleep);
    }
}

void *writerF(void* arg){
    while (1) {
        pthread_mutex_lock(&varMutex);
            writersInQueue++;
            printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",
                    readersInQueue, writersInQueue, readersInLibrary, writersInLibrary);
        pthread_mutex_unlock(&varMutex);

        pthread_mutex_lock(&libraryMutex);
            pthread_mutex_lock(&varMutex);
                while (readersInLibrary || writersInLibrary) {
                    pthread_mutex_unlock(&varMutex);
                    pthread_cond_wait(&entryCond, &libraryMutex);
                    pthread_mutex_lock(&varMutex);
                }
                writersInQueue--;
                writersInLibrary++;
                printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",
                        readersInQueue, writersInQueue, readersInLibrary, writersInLibrary);
            pthread_mutex_unlock(&varMutex);
        pthread_mutex_unlock(&libraryMutex);

        /* Writing... */
        usleep(rand() % uSecSleep);

        pthread_mutex_lock(&libraryMutex);
            pthread_mutex_lock(&varMutex);
                writersInLibrary--;
                printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",
                        readersInQueue, writersInQueue, readersInLibrary, writersInLibrary);
            pthread_mutex_unlock(&varMutex);
        pthread_mutex_unlock(&libraryMutex);
        pthread_cond_broadcast(&entryCond);

        /* Sleep and enter the queue again */
        usleep(rand() % uSecSleep);
    }
}

int main(int argc, char** argv) {
    printf("\nReaders - writers 2nd problem (writers priority)\n");
    int readersCount, writersCount;
    if ((argv[1] == NULL) || (argv[2]) == NULL ) {
        printf("Number of readers > ");
        if (scanf("%d", &readersCount) == EOF) perror_exit("scanf");
        printf("Number of writers > ");
        if (scanf("%d", &writersCount) == EOF) perror_exit("scanf");
        printf("Starting in 1 sec...\n\n");
        sleep(1);
    } else {
        readersCount = atoi(argv[1]);
        writersCount = atoi(argv[2]);
        printf("Number of Readers = %d\n", readersCount);
        printf("Number of Writers = %d\n", writersCount);
        printf("Starting in 2 sec...\n\n");
        sleep(2);
    }
    srand(time(NULL));
    pthread_t *readerThread = calloc(readersCount, sizeof(pthread_t));
    pthread_t *writerThread = calloc(writersCount, sizeof(pthread_t));
    long i = 0;
    printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",
            readersInQueue, writersInQueue, readersInLibrary, writersInLibrary);
    for (i = 0; i < readersCount; ++i) {
        if (pthread_create(&readerThread[i], NULL, readerF, (void*)i)) {
            perror_exit("Error while creating reader thread (pthread_create)");
        }
    }
    for (i = 0; i < writersCount; ++i) {
        if (pthread_create(&writerThread[i], NULL, writerF, (void*)i)) {
            perror_exit("Error while creating writer thread (pthread_create)");
        }
    }
    for (i = 0; i < readersCount; ++i) {
        if (pthread_join(readerThread[i], NULL)) {  // wait for thread finish which will not happen
            perror_exit("Error while waiting for reader thread termination (pthread_join)");
        }
    }
    free(readerThread);
    for (i = 0; i < writersCount; ++i) {
        if (pthread_join(writerThread[i], NULL)) {
            perror_exit("Error while waiting for writer thread termination (pthread_join)");
        }
    }
    free(writerThread);
    pthread_cond_destroy(&entryCond);
    pthread_mutex_destroy(&varMutex);
    pthread_mutex_destroy(&libraryMutex);
}