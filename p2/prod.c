#include <stdio.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PRODUCERS 4
#define CONSUMERS 3
#define CRITICAL_SECTIONS 4
#define SEMAPHORES_N CRITICAL_SECTIONS * 2
#define TOTAL_THREADS CONSUMERS + PRODUCERS
#define PRODUCTIONS 10

// Array of critical sections
char sections[CRITICAL_SECTIONS][10];
// Array of semaphores
int semaphores;
// Array of productions
// This needs to be modified if changing the 
// number of productions or producers
const char *productions[PRODUCERS] = {
    "AAAAAAAAAA",
    "BBBBBBBBBB",
    "CCCCCCCCCC",
    "DDDDDDDDDD"};

void *produce(void *arg)
{
    int index = *(int *)arg;
    printf("producer: %d ", index);
    printf("a copiar: %s\n", productions[index]);
    strcpy(sections[index], productions[index]);
    printf("copie: %s\n", sections[index]);
    printf("producer end\n");
    pthread_exit(NULL);
}
void *consume(void *arg)
{
    printf("consumer\n");
    sleep(1);
    printf("consumer end\n");
    pthread_exit(NULL);
}

int main()
{
    // Array of indexes for accessing the productions array
    // so each production thread produces different strings
    int * indexes = (int *)malloc(sizeof(int) * PRODUCERS);
    for (int i = 0; i < PRODUCERS; i++)
    {
        indexes[i] = i;
    }
    // Creating key for the semaphores
    key_t sem_key;
    if ((sem_key = ftok("/bin/ls", 'k')) == -1)
    {
        printf("ERROR: Could not create key\n");
        exit(1);
    }
    // Creating two semaphores for each critical section
    if ((semaphores = semget(sem_key, SEMAPHORES_N, IPC_CREAT | 0600)) == -1)
    {
        printf("ERROR: Could not create %d semaphores\n", SEMAPHORES_N);
        exit(1);
    }
    // Initializing each semaphore
    for (int i = 0; i < SEMAPHORES_N; i++)
    {
        // The first semaphores are for the producer
        if (i < SEMAPHORES_N / 2)
        {
            semctl(semaphores, i, SETVAL, 1);
        }
        // The rest of the semaphores are for the consumers
        else
        {
            semctl(semaphores, i, SETVAL, 0);
        }
    }
    // The threads that will produce and consume values from and out of the
    // critical sections (arrays)
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * TOTAL_THREADS);
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        // The first semaphores are for the producer
        if (i < PRODUCERS)
        {
            pthread_create(&threads[i], NULL, produce,(void *) &indexes[i]);
        }
        // The rest of the semaphores are for the consumers
        else
        {
            pthread_create(&threads[i], NULL, consume, NULL);
        }
    }
    // Waiting for all threads to finish their execution
    for (int i = TOTAL_THREADS - 1; i >= 0; i--)
    {
        printf("%d\n", i);
        pthread_join(threads[i], NULL);
        printf("joined:%d\n", i);
    }
    // for (int i = 0; i < TOTAL_THREADS; i++)
    // {
    //     printf("%d\n", i);
    //     pthread_join(threads[i], NULL);
    //     printf("joined:%d\n", i);
    // }
    semctl(semaphores, 0, IPC_RMID, 0);
    for (int i = 0; i < CRITICAL_SECTIONS; i++)
    {
        printf("%d: ", i);
        printf("%s\n", sections[i]);
    }
    
    return 0;
}