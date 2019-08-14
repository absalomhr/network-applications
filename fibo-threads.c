#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
	int value;
	pthread_t * thread_id;

	int * res_arr;
	int size;
} node;

void * fibo_hilos (void * arg){
	node * a = (node *) arg;
	int value = a -> value;
	long * thread_id = a -> thread_id;
	printf("Yo soy el hilo: %ld, ", * thread_id);
	printf("mi valor: %d\n", value);
	
	int * res_arr = (int *)malloc(sizeof(int)*2);
	res_arr[0] = 0;
	if (value == 0){
		a -> size = 1;
		a -> res_arr = res_arr;
		pthread_exit(NULL);
	}
	res_arr[1] = 1;
	int prim = 0, seg = 1, size = 2, ind1 = 0, ind2 = 1, nuevo;
	
	while (1){
		nuevo = res_arr[ind1] + res_arr[ind2];
		if (nuevo > value){
			break;
		} else {
			res_arr =(int *) realloc(res_arr, sizeof(int)*size + 1);
			res_arr[size] = nuevo;

			size++;

			ind1++; ind2++;

			prim = seg;
			seg = nuevo;
		}
	}
	
	a -> size = size;
	a -> res_arr = res_arr;

	pthread_exit(NULL);
}

int main () {
	int n, d;
	pthread_t * hilos;
	node * args;
	printf("Escribe cuantos hilos quieres: \n");
	scanf("%d", &n);
	printf("\n");
	fflush(stdin);

	hilos = (pthread_t *)malloc(sizeof(pthread_t)*n);
	args = (node *)malloc(sizeof(node)*n);

	printf("Introduce los valores para cada hilo\n");
	for (int i = 0; i < n; i++){
		scanf("%d", &d);
		fflush(stdin);
		args [i].value = d;
		args [i].thread_id = &hilos[i];
	}
	printf("\n");
	for (int i = 0; i < n; i++){
		pthread_create(args[i].thread_id, NULL, fibo_hilos, (void *)&args[i]);
	}


	for (int i = 0; i < n; i++){
		pthread_join(* args[i].thread_id, NULL);
		printf("\n");
		printf("Arreglo: %d,", i + 1);
		printf(" del hilo: %ld\n", * (long *)args[i].thread_id);
		for(int j = 0; j < args[i].size; j++){
			printf("%d ", args[i].res_arr[j]);
		}
		printf("\n");
	}

	return 0;
}
