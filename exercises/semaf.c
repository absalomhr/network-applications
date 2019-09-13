#include<stdio.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<pthread.h>
#include<stdlib.h>

#include<unistd.h>

#include<string.h>

#define TOTAL_THREAD 4
#define TOTAL_SEMAFOROS 4
#define TOTAL_PRODUCCIONES 10000

int consumos = 0;

char section[TOTAL_SEMAFOROS][10];
int sems;

void openSem(int sem_id){
	struct sembuf op;
	op.sem_num = sem_id;
	op.sem_op = 1;
	op.sem_flg = SEM_UNDO;
	semop(sems, &op, 1);
}
void closeSem(int sem_id){
	struct sembuf op;
	op.sem_num = sem_id;
	op.sem_op = -1;
	op.sem_flg = SEM_UNDO;
	semop(sems, &op, 1);
}
void * productor(void){
	char producir [] = "hola";
	int producciones = 0;
	while(producciones < TOTAL_PRODUCCIONES){
		for (int i = 0; i < TOTAL_SEMAFOROS; i += 2){
			if(semctl(sems, i, GETVAL) != 0){
				closeSem(i);
				strcpy(section[i], producir);
				printf(" # Produccion '%s' en %d por %ld [%d]\n",section[i], i, pthread_self(), ++producciones);
				openSem(i + 1);
				break;
			}
		}
	}
}
void * consumidor(void){
	while(consumos < TOTAL_PRODUCCIONES * (TOTAL_THREAD / 2)){
		for (int i = 1; i < TOTAL_SEMAFOROS; i += 2){
			if(semctl(sems, i, GETVAL) != 0){
				closeSem(i);
				closeSem(TOTAL_SEMAFOROS); // consumos
				if(consumos < TOTAL_PRODUCCIONES * (TOTAL_THREAD / 2)){
					printf(" >> Consumo '%s' leído en %d por %ld [%d]\n",section[i - 1],  i - 1, pthread_self(), ++consumos);
				}
				openSem(TOTAL_SEMAFOROS);
				openSem(i - 1);	
				break;
			}
		}
	}

	semctl(sems, 0, IPC_RMID, 0);
}
int main(int argc, char const *argv[]){
	//int sems;
	key_t semKey;
	if((semKey=ftok("/bin/ls",'k'))==-1){
		printf("Error creando la llave prro\n");
		exit(1);
	}
	if((sems=semget(semKey,TOTAL_SEMAFOROS + 1,IPC_CREAT|0600))==-1){
		printf("Error al crear los semaforos prro\n");
		exit(1);
	}

	// Inicializaciones
	for(int i = 0; i < TOTAL_SEMAFOROS; i++){
		semctl(sems,i,SETVAL,1);
		semctl(sems,++i,SETVAL,0);
	}

	semctl(sems,TOTAL_SEMAFOROS,SETVAL,1); // contador de consumos

	pthread_t * threads = (pthread_t *)malloc(sizeof(pthread_t)*TOTAL_THREAD);
	
	for(int i = 0; i < TOTAL_THREAD; i++){
		pthread_create(&threads[i], NULL, (void *)productor, NULL);
		pthread_create(&threads[++i], NULL, (void *)consumidor, NULL);
	}

	for(int i = 0; i < TOTAL_THREAD; i++)
		pthread_join(threads[i],NULL);
	
	semctl(sems, 0, IPC_RMID, 0);

	return 0;
}