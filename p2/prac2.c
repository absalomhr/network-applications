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
#define TRUE 1

// Array of critical sections
char sections[CRITICAL_SECTIONS][10];
// Array of semaphores
int semaphores;
// All of the consumptions to be made by the consumer threads
int total_consumptions = PRODUCERS * PRODUCTIONS;

void open_semaphore(int sem_id)
{
    struct sembuf op;
    op.sem_num = sem_id;
    op.sem_op = 1;
    op.sem_flg = SEM_UNDO;
    semop(semaphores, &op, 1);
}
void close_semaphore(int sem_id)
{
    struct sembuf op;
    op.sem_num = sem_id;
    op.sem_op = -1;
    op.sem_flg = SEM_UNDO;
    semop(semaphores, &op, 1);
}
void *produce(void *arg)
{
    int option = *(int *)arg;
    char s[10];
    // This need to be modified according to the number of producers
    switch (option)
    {
    case 0:
        strcpy(s, "AAAAAAAAA");
        break;
    case 1:
        strcpy(s, "BBBBBBBBB");
        break;
    case 2:
        strcpy(s, "CCCCCCCCC");
        break;
    case 3:
        strcpy(s, "DDDDDDDDD");
        break;
    }
    int section_index = 0;
    for (int i = 0; i < PRODUCTIONS; i++)
    {
        while (TRUE)
        {
            if (section_index >= CRITICAL_SECTIONS - 1)
                section_index = 0;
            if (semctl(semaphores, section_index, GETVAL) != 0)
            {
                // Closing the producer semaphore
                close_semaphore(section_index);
                strcpy(sections[section_index], s);
                // Opening the consumer semaphore
                open_semaphore(section_index + CRITICAL_SECTIONS);
                break;
            }
            section_index++;
        }
    }
    pthread_exit(NULL);
}
void *consume(void *arg)
{
    printf("consuming %ld\n", pthread_self());
    int section_index = CRITICAL_SECTIONS;
    char s[10];

    // There is work to do or not
    int flag = 0;
    while (flag == 0)
    {
        while (TRUE)
        {
            if (semctl(semaphores, SEMAPHORES_N, GETVAL) != 0)
            {
                close_semaphore(SEMAPHORES_N);
                if (total_consumptions <= 0){
                    flag = -1;
                }
                open_semaphore(SEMAPHORES_N);
                break;
            }
        }
        if (flag != 0)
        {
            break;
        }
        if (section_index >= (CRITICAL_SECTIONS * 2) - 1)
            section_index = CRITICAL_SECTIONS;
        if (semctl(semaphores, section_index, GETVAL) != 0)
        {
            // Closing the consumer semaphore
            close_semaphore(section_index);
            strcpy(s, sections[section_index - CRITICAL_SECTIONS]);
            printf("c %ld: consumed: %s\n", pthread_self(), s);
            while (TRUE)
            {
                if (semctl(semaphores, SEMAPHORES_N, GETVAL) != 0)
                {
                    close_semaphore(SEMAPHORES_N);
                    total_consumptions--;
                    open_semaphore(SEMAPHORES_N);
                    break;
                }
            }
            // Opening the producer semaphore
            open_semaphore(section_index - CRITICAL_SECTIONS);
        }
        section_index++;
    }
    printf("c: %ld OUT OF FOR\n", pthread_self());
    pthread_exit(NULL);
}

int main()
{
    // Array of indexes for accessing the productions array
    // so each production thread produces different strings
    const int indexes[4] = {0, 1, 2, 3};
    // Creating key for the semaphores
    key_t sem_key;
    if ((sem_key = ftok("/bin/ls", '1')) == -1)
    {
        printf("ERROR: Could not create key\n");
        exit(1);
    }
    // Creating two semaphores for each critical section plus one more for total consumptions
    if ((semaphores = semget(sem_key, SEMAPHORES_N + 1, IPC_CREAT | 0600)) == -1)
    {
        printf("ERROR: Could not create %d semaphores\n", SEMAPHORES_N);
        exit(1);
    }
    // Initializing each semaphore
    for (int i = 0; i < SEMAPHORES_N; i++)
    {
        // The first semaphores are for the producers
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
    // Semaphore for the total consumptions
    semctl(semaphores, SEMAPHORES_N, SETVAL, 1);
    // The threads that will produce and consume values from and out of the
    // critical sections (arrays)
    pthread_t *prod_threads = (pthread_t *)malloc(sizeof(pthread_t) * PRODUCERS);
    pthread_t *cons_threads = (pthread_t *)malloc(sizeof(pthread_t) * CONSUMERS);
    // Creating and starting the execution of each array
    for (int i = 0; i < PRODUCERS; i++)
    {
        pthread_create(&prod_threads[i], NULL, produce, (void *)&indexes[i]);
    }
    for (int i = 0; i < CONSUMERS; i++)
    {
        pthread_create(&cons_threads[i], NULL, consume, NULL);
    }
    // Waiting for each thread to end its execution
    for (int i = 0; i < CONSUMERS; i++)
    {
        pthread_join(cons_threads[i], NULL);
    }
    for (int i = 0; i < PRODUCERS; i++)
    {
        pthread_join(prod_threads[i], NULL);
    }
    semctl(semaphores, 0, IPC_RMID, 0);
    return 0;
}