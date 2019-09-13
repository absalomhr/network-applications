
/*
 * =====================================================================================
 *
 *       Filename:  ejemplo.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/23/2019 11:50:42 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define PRODUCTOR1 0
#define PRODUCTOR2 2
#define CONSUMIDOR1 1 
#define CONSUMIDOR2 3 

#define NUM_PRODUCTORES 3
#define NUM_CONSUMIDORES 3

#define TAM_BUFFER 11

int semid;	
char seccion_critica[2][TAM_BUFFER];

const int semaforos_productores[] = {0,2};
const int semafores_consumidores[] = {1,3};

const int num_producciones_x_productor = 10;
const int num_secciones = 2;
const int tam_operacion = 1;
struct sembuf operacion;


void *hilo_productor(void *arg);


void *hilo_consumidor(void *arg);


void cerramos_semaforo(int semaforo);
void abrimos_semaforo(int semaforo);

int main(int argc,char *argv[])
{
	key_t llave;
	pthread_t productores[NUM_PRODUCTORES];
	int productores_id[] = {0,1,2};
	pthread_t consumidores[NUM_CONSUMIDORES];
	int consumidores_id[] = {0,1,2};
	
	llave = ftok(argv[0],'k');
	if((semid = semget(llave,4,IPC_CREAT | 0600)) == -1) 
	{
		printf(" no se puede crear a los semáforos\n");
		exit(-1);
	}

	//inicializamos a los semáforos
	semctl(semid,PRODUCTOR1,SETVAL,1); // abrimos semáforo PRODUCTOR
	semctl(semid,PRODUCTOR2,SETVAL,1); // abrimos semáforo PRODUCTOR
	semctl(semid,CONSUMIDOR1,SETVAL,0); //cerramos semáforo CONSUMIDOR
	semctl(semid,CONSUMIDOR2,SETVAL,0); //cerramos semáforo CONSUMIDOR


	for(int i = 0; i < NUM_PRODUCTORES; i++)
	{
		pthread_create(&productores[i],NULL,hilo_productor,(void *)&productores_id[i]); 
		pthread_create(&consumidores[i],NULL,hilo_consumidor,(void *)&consumidores_id[i]);
	}

	

	for(int i = 0; i < NUM_CONSUMIDORES; i++)
	{
		pthread_join(productores[i],NULL);
		pthread_join(consumidores[i],NULL);
	}

	semctl(semid,0,IPC_RMID,0);
	return 0;
}

void *hilo_productor(void *args)
{
	char produccion[TAM_BUFFER];
	int indice_seccion;
	int  produjo;
	int num_producciones = 0;
	switch(*((int *)args))
	{
		case 0:
			strcpy(produccion,"hola mundo");
			break;
		case 1:
			strcpy(produccion,"2222222222");
			break;
		case 2:
			strcpy(produccion,"3333333333");
			break;
	}

	for(int i = 0; i < num_producciones_x_productor; i++)
	{
		indice_seccion = 0;
		//produjo = FALSE;
		while(TRUE)
		{
			if(indice_seccion >= num_secciones)
				indice_seccion = 0;
			if((semctl(semid,semaforos_productores[indice_seccion],GETVAL,0)) != 0)
			{
				//cierro el semaforo productor
				cerramos_semaforo(semaforos_productores[indice_seccion]);
				printf("\n");
				//printf("se cierra semaforo productor: %d\n",semaforos_productores[indice_seccion]);
				//zona critica
				strcpy(seccion_critica[indice_seccion],produccion);
				printf("No Productor:%d , No Production:%d,  Value:%s, Total produtcions:%d\n",*((int *)args),indice_seccion,produccion,++num_producciones);
				//salimos de zona critica
				//abrimos semaforo consumidor
				abrimos_semaforo(semafores_consumidores[indice_seccion]);
				printf("\n");
				//printf("se abre semaforo consumidro indice: %d\n",semafores_consumidores[indice_seccion]);
				break;
			}
			indice_seccion++;
		}
	}
	pthread_exit(NULL);
}

void *hilo_consumidor(void *args)
{
	char buffer[TAM_BUFFER];
	int indice_seccion;
	int num_consumos = 0;
	for(int i = 0; i < num_producciones_x_productor; i++)
	{
//		consumio = FALSE;
		indice_seccion = 0;
		while(TRUE)
		{
			if(indice_seccion >= num_secciones)
				indice_seccion = 0;
			if((semctl(semid,semafores_consumidores[indice_seccion],GETVAL,0))!= 0)
			{
				//cerramos_semaforo CONSUMIDOR
				cerramos_semaforo(semafores_consumidores[indice_seccion]); 
				printf("\n");
				//printf("se cierra semaforo consumidro indice: %d\n",semafores_consumidores[indice_seccion]);
				//zona critica
				strcpy(buffer,seccion_critica[indice_seccion]);
				printf("\tsoy el consumidor:%d, consumo: %s , en seccion: %d ,numero de consumos: %d\n",
						*((int *)args),buffer,indice_seccion,++num_consumos);
				//salimos de zona critica
				//abrimos_semaforo PRODUCTOR
				abrimos_semaforo(semaforos_productores[indice_seccion]);
				printf("\n");
				//printf("se abre  semaforo productor: %d\n",semaforos_productores[indice_seccion]);
				break;
			}
			indice_seccion++;
		}
	}
	pthread_exit(NULL);
}

void cerramos_semaforo(int semaforo)
{
	operacion.sem_num = semaforo;
	operacion.sem_op = -1;
	operacion.sem_flg = 0;
	semop(semid,&operacion,tam_operacion);
}

void abrimos_semaforo(int semaforo)
{
	operacion.sem_num = semaforo;
	operacion.sem_op = 1;
	operacion.sem_flg = 0;
	semop(semid,&operacion,tam_operacion);
}

