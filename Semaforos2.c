#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <errno.h>
#include<unistd.h>

void * productor(void*);
void * consumidor();
void SumSem(int);
void RestSem(int);
int sems;
char abc[10]="ola";
int producciones;
int num_hilos=6;
char seccion[3][6]={{"0"},{"0"}};


void main(){
	printf("Numero de producciones:");
	scanf("%d",&producciones);
	int indice=0;//Posicion del arreglo abc a escribir
	key_t semKey;
	if(semKey = ftok("/tmp", 1)== (key_t)-1){//Creacion de llaves
		printf("Error al crear la llave\n");
		exit(1);
	}
	
	if(sems= semget(semKey, 4, IPC_CREAT | IPC_EXCL| 666 )==-1){//Creacion de semaforos
		printf("Error al crear el semaforo\n");
		exit(1);
	}
	if(semctl(sems, 0, SETVAL, 2)){
		perror("Error: ");
	}
	semctl(sems, 1, SETVAL, 0);
	semctl(sems, 2, SETVAL, 1);
	semctl(sems, 3, SETVAL, 1);
	
	pthread_t thread[num_hilos];//Creacion de hilos
	

	int uno=0, dos=1, tres=2;
	thread[0]=(pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&thread[0], NULL,productor,(void *)&uno);
	thread[1]=(pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&thread[1], NULL,consumidor,(void *)NULL);
	thread[2]=(pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&thread[2], NULL,productor,(void *)&dos);
	thread[3]=(pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&thread[3], NULL,consumidor,(void *)NULL);
	thread[4]=(pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&thread[4], NULL,productor,(void *)&tres);
	thread[5]=(pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&thread[5], NULL,consumidor,(void *)NULL);
	
	
		pthread_join(thread[0], NULL);
		pthread_join(thread[2], NULL);
		pthread_join(thread[4], NULL);
		pthread_join(thread[1], NULL);
		pthread_join(thread[3], NULL);
		pthread_join(thread[5], NULL);

		sleep(1);

}

void * productor(void *arg){
	int *posicion_arreglo=(int *)arg;
	for(int j=0;j<producciones;){
		RestSem(0);
			
				if(seccion[0][0]==48){
					
					RestSem(2);
					seccion[0][0]=49;//bandera seccion critica1
					seccion[0][1]=abc[*posicion_arreglo];
					printf("Produccion %i:%c\n", j,seccion[0][1]);
					SumSem(2);
				
				}
				else
					if(seccion[1][0]==48){
					RestSem(3);
					seccion[1][0]=49;//bandera seccion critica2
					seccion[0][1]=abc[*posicion_arreglo];
					printf("Produccion %i:%c\n", j,seccion[0][1]);
					SumSem(3);
					
			}
			SumSem(1);
		
	j++;
	}
	return (void*)1;
}

void * consumidor(){
	for(int j=0;j<producciones;){
		RestSem(1);
				if(seccion[0][0]==49){
				
				RestSem(2);
				seccion[0][0]=48;//bandera seccion critica1
				printf("Consumidor %i:%s\n",j,seccion[0]);
				SumSem(2);
				
			}
				else
					if(seccion[1][0]==49){
			
				RestSem(3);
				seccion[1][0]=48;//bandera seccion critica2
				printf("Consumidor %i:%s\n",j,seccion[0]);
				SumSem(3);
				
			
			}
			SumSem(0);

		j++;
	}

	return (void*)1;
}

void SumSem(int ind_sem){
	struct sembuf op;
	op.sem_num = ind_sem; // al indice del semaforo a ustar
	op.sem_op =1;
	op.sem_flg=SEM_UNDO;
	semop(sems,&op, 1);
	
}
void RestSem(int ind_sem){
	
	struct sembuf op;
	op.sem_num = ind_sem; // al indice del semaforo a ustar
	op.sem_op =-1;
	op.sem_flg=SEM_UNDO;
	semop(sems,&op, 1);
	
}
