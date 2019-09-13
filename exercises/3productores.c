#include <stdio.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <stdlib.h>

#define Total_threads 2
int section = 0,sems;
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
void * tenedor(void){
	for (int i = 0; i < 50; ++i){
		closeSem(0);
		printf("El valor de la seccion es: %d\n",section++);
		openSem(1);	
	}
}
void * tenedor2(void){
	for (int i = 0; i < 50; ++i){
		closeSem(1);
		printf("El valor obtenido es: %d\n",section);
		openSem(0);
	}
}
int main(int argc, char const *argv[]){
	//int sems;
	key_t semKey;
	if((semKey=ftok("/bin/ls",1))==-1){
		printf("Error creando la llave prro\n");
		exit(1);
	}
	if((sems=semget(semKey,2,IPC_CREAT|IPC_EXCL|666))==-1){
		printf("Error al crear los semaforos prro\n");
		exit(1);
	}
	if(semctl(sems,0,SETVAL,1)==-1){
		printf("Error inicializando los semaforos prro\n");
		exit(1);
	}
	pthread_t * threads = (pthread_t *)malloc(sizeof(pthread_t)*Total_threads);
	pthread_create(&threads[0], NULL, (void *)tenedor, NULL);
	pthread_create(&threads[1], NULL, (void *)tenedor2, NULL);
	pthread_join(threads[0],NULL);
	pthread_join(threads[1],NULL);
	return 0;
}