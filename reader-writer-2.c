#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>

#define READERS 10
#define WRITERS 5

struct sched_param  param;
struct sched_param  main_param;
pthread_mutex_t     read_mutex      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     write_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     readTry_mutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     resource_mutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t      attribute_writer;
pthread_attr_t      attribute_reader;
pthread_t           reader_thread[READERS];
pthread_t           writer_thread[WRITERS];
static int          readcount;
static int          writecount;
static int          value;

void* writers();
void* readers();

void proc_rt(int prio){
    int min  = sched_get_priority_min(SCHED_RR);
    int max  = sched_get_priority_max(SCHED_RR);
    prio = prio > min ? prio : prio < max ? max : prio;
    main_param.sched_priority = prio;
    sched_setscheduler(pthread_self(), SCHED_RR, &main_param);
}

int main(){
    int ret;
    printf("Readers-writers problem solve algorithm - writers-preference\n");

    proc_rt(50);

    pthread_attr_init(&attribute_writer);
    pthread_attr_init(&attribute_reader);
    ret = pthread_attr_getschedparam(&attribute_reader, &param);
    param.sched_priority++;
    ret = pthread_attr_setschedparam(&attribute_writer, &param);

    for (int i = 0; i < WRITERS; i++)
        pthread_create(&writer_thread[i], &attribute_writer, writers, NULL);
    for (int i = 0; i < READERS; i++)
        pthread_create(&reader_thread[i], &attribute_reader, readers, NULL);
    
    for (int i = 0; i < WRITERS; i++)
        pthread_join(writer_thread[i], NULL);
    for (int i = 0; i < READERS; i++)
        pthread_join(reader_thread[i], NULL);
    
    return 0;
}

void* readers(){
    pthread_mutex_lock(&readTry_mutex);
    pthread_mutex_lock(&read_mutex);
    readcount++;
    if (readcount == 1)
        pthread_mutex_lock(&resource_mutex);
    pthread_mutex_unlock(&read_mutex);
    pthread_mutex_unlock(&readTry_mutex);

    printf("\033[35;1m I'm reader number: %d\033[0m\n", readcount);
    sleep(1);

    pthread_mutex_lock(&read_mutex);
    readcount--;
    if (readcount == 0)
        pthread_mutex_unlock(&resource_mutex);
    pthread_mutex_unlock(&read_mutex);

    pthread_exit(0);
}

void* writers(){
    pthread_mutex_lock(&write_mutex);
    writecount++;
    if (writecount == 1)
        pthread_mutex_lock(&readTry_mutex);
    pthread_mutex_unlock(&write_mutex);
    
    pthread_mutex_lock(&resource_mutex);
    printf("\033[31;1m I'm writer number: %d\033[0m\n", writecount);
    pthread_mutex_unlock(&resource_mutex);
    sleep(1);

    pthread_mutex_lock(&write_mutex);
    writecount--;
    if (writecount == 0)
        pthread_mutex_unlock(&readTry_mutex);
    pthread_mutex_unlock(&write_mutex);

    pthread_exit(0);
}
