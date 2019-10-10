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
#define CRITICAL_SECTIONS 7
#define SEMAPHORES_N CRITICAL_SECTIONS * 2
#define TOTAL_THREADS CONSUMERS + PRODUCERS
#define PRODUCTIONS 10000
#define TRUE 1

// Array of critical sections
char sections[CRITICAL_SECTIONS][10];
// Array of semaphores
int semaphores;
// All of the consumptions to be made by the consumer threads
int total_consumptions = PRODUCERS * PRODUCTIONS;
// File pointers array
FILE *files[PRODUCERS];
// Opening a semaphore (reducing its value by 1)
void open_semaphore(int sem_id)
{
    struct sembuf op;
    op.sem_num = sem_id;
    op.sem_op = 1;
    op.sem_flg = SEM_UNDO;
    semop(semaphores, &op, 1);
}
// Closing a semaphore (adding to its value by 1)
void close_semaphore(int sem_id)
{
    struct sembuf op;
    op.sem_num = sem_id;
    op.sem_op = -1;
    op.sem_flg = SEM_UNDO;
    semop(semaphores, &op, 1);
}
// Function for each producer thread
void *produce(void *arg)
{
    // What type of production will you make
    int option = *(int *)arg;
    // Variable for storing its production
    char s[10];
    // This needs to be modified according to the number of producers
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
    // Starting index for the producers in the semaphores array
    int section_index = (SEMAPHORES_N / 2);
    for (int i = 0; i < PRODUCTIONS; i++)
    {
        while (TRUE)
        {
            // We only need to close producers semaphores
            if (section_index >= (SEMAPHORES_N / 2) + (SEMAPHORES_N / 2))
                section_index = SEMAPHORES_N / 2;
            // If the section is open, we write to the section
            if (semctl(semaphores, section_index, GETVAL) != 0)
            {
                // Closing the producer semaphore
                close_semaphore(section_index);
                strcpy(sections[section_index - (SEMAPHORES_N / 2)], s);
                // Opening the consumer semaphore
                open_semaphore(section_index - (SEMAPHORES_N / 2));
                break;
            }
            // If the section is closed we'll try to write to another section
            else
                section_index++;
        }
    }
    pthread_exit(NULL);
}
// Function for each consumer
void *consume(void *arg)
{
    // Starting indes for consumer semaphores in the semaphore array
    int section_index = 0;
    // Variable for storing the consumption
    char s[10];
    // Counting if we already did all the necesary consumptions
    int consumption_flag_index = SEMAPHORES_N + PRODUCERS;
    // Variable that stores the index of the file semaphore that we will write to
    int file_index;
    // Flag that watches whether we did all the consumptions or not
    int flag = 0;
    while (flag == 0)
    {
        // Starting index for the file semaphores in the semaphore array
        file_index = SEMAPHORES_N;
        // Checking if we already did all the consumptions
        while (TRUE)
        {
            if (semctl(semaphores, consumption_flag_index, GETVAL) != 0)
            {
                close_semaphore(consumption_flag_index);
                if (total_consumptions <= 0)
                {
                    // All the consumptions are done
                    flag = -1;
                }
                open_semaphore(consumption_flag_index);
                break;
            }
        }
        // All the consumptions are done, nothing left to do
        if (flag != 0)
        {
            break;
        }
        // We only gonna close consumer semaphores
        if (section_index >= SEMAPHORES_N / 2)
            section_index = 0;
        // If the section is open, we read the value stored there
        if (semctl(semaphores, section_index, GETVAL) != 0)
        {
            // Closing the consumer semaphore
            close_semaphore(section_index);
            // Retrieving the value
            strcpy(s, sections[section_index]);
            // We already read the value, we can decrease the number of consumptions left
            while (TRUE)
            {
                if (semctl(semaphores, consumption_flag_index, GETVAL) != 0)
                {
                    close_semaphore(consumption_flag_index);
                    total_consumptions--;
                    open_semaphore(consumption_flag_index);
                    break;
                }
            }
            // Opening the producer semaphore because we already copied the value that the section had
            open_semaphore(section_index + (SEMAPHORES_N / 2));
            // Depending of what we read, we will write to a corresponding file
            if (strcmp(s, "BBBBBBBBB") == 0)
            {
                file_index++;
            }
            else if (strcmp(s, "CCCCCCCCC") == 0)
            {
                file_index += 2;
            }
            else if (strcmp(s, "DDDDDDDDD") == 0)
            {
                file_index += 3;
            }
            // We try to close the semaphore of the corresponding file
            while (TRUE)
            {
                // When the file opens, we will write the value
                if (semctl(semaphores, file_index, GETVAL) != 0)
                {
                    close_semaphore(file_index);
                    fprintf(files[file_index - SEMAPHORES_N], "%s\n", s);
                    open_semaphore(file_index);
                    break;
                }
            }
        }
        // If the section is closed, we try another
        else
            section_index++;
    }
    pthread_exit(NULL);
}

int main()
{
    // Array of indexes for accessing the productions array
    // so each production thread produces different strings
    const int indexes[4] = {0, 1, 2, 3};
    // Opening each file, this also needs to be modified if the numer
    // of producers changes
    files[0] = fopen("file.txt", "w");
    files[1] = fopen("file2.txt", "w");
    files[2] = fopen("file3.txt", "w");
    files[3] = fopen("file4.txt", "w");
    // Creating key for the semaphores
    key_t sem_key;
    if ((sem_key = ftok("/bin/ls", '1')) == -1)
    {
        printf("ERROR: Could not create key\n");
        exit(1);
    }
    // Creating the needed semaphores
    // [CONSUMERS_SEM, PRODUCERS_SEM, FILES, CONSUMPTIONS_FLAG]
    if ((semaphores = semget(sem_key, SEMAPHORES_N + PRODUCERS + 1, IPC_CREAT | 0600)) == -1)
    {
        printf("ERROR: Could not create %d semaphores\n", SEMAPHORES_N);
        exit(1);
    }
    // Initializing each semaphore
    for (int i = 0; i < SEMAPHORES_N + PRODUCERS + 1; i++)
    {
        // The first semaphores are for the consumers
        if (i < SEMAPHORES_N / 2)
        {
            semctl(semaphores, i, SETVAL, 0);
        }
        // The rest of the semaphores are for the consumers
        else
        {
            semctl(semaphores, i, SETVAL, 1);
        }
    }
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
    // Deleting the semaphores
    semctl(semaphores, 0, IPC_RMID, 0);
    for (int i = 0; i < PRODUCERS; i++)
    {
        fclose(files[i]);
    }
    return 0;
}