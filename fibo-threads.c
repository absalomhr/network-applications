#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
	This program receives a number of threads to be created and
	a value for each thread.
	Then each thread will generate and store in an array the fibonacci
	sequence up to the received value or the inmediate previous value
	in the sequence. The main program will print each generated array.
*/

/* Structure sent to each thread as argument */
typedef struct {
	int value; // value sent to the thread
	pthread_t * thread_id; // the thread id

	int * res_arr; // address where the generated array will be stored
	int size; // size of the generated array
} Node;

/*
	Function that each thread executes. It will generate
	values from the fibonacci sequence up to the value passed
	to the thread.
 */
void * fibonacci_thread (void * arg){
	Node * node = (Node *) arg;
	int value = node -> value;
	long * thread_id = node -> thread_id;
	printf("I'm the thread: %ld, ", * thread_id);
	printf("I received the value: %d\n", value);

	// Case where the passed values is negative
	if (value < 0){
		node -> res_arr = NULL;
		pthread_exit(NULL);
	}
	int * res_arr = (int *)malloc(sizeof(int)*2);
	res_arr[0] = 0;
	if (value == 0){
		node -> size = 1;
		node -> res_arr = res_arr;
		pthread_exit(NULL);
	}
	res_arr[1] = 1;
	// Generating the fibonacci sequence
	int size = 2, f_index = 0, s_index = 1, new_v;
	while (1) {
		new_v = res_arr[f_index] + res_arr[s_index];
		// If the number in the sequence is greater than
		// the limit value, we stop generating
		if (new_v > value){
			break;
		}
		else {
			res_arr = (int *)realloc(res_arr, sizeof(int)*size + 1);
			res_arr[size] = new_v;
			size++; f_index++; s_index++;
		}
	}
	node -> size = size;
	node -> res_arr = res_arr;
	pthread_exit(NULL);
}


int main () {
	int n, d;
	pthread_t * threads;
	Node * args; // Array of arguments for each thread
	printf("How many threads do you want?: \n");
	scanf("%d", &n);
	printf("\n");
	fflush(stdin);

	threads = (pthread_t *)malloc(sizeof(pthread_t)*n);
	args = (Node *)malloc(sizeof(Node)*n);

	printf("Enter the value for each thread\n");
	for (int i = 0; i < n; i++){
		scanf("%d", &d);
		fflush(stdin);
		args [i].value = d;
		args [i].thread_id = &threads[i];
	}
	printf("\n");
	// Creating the threads
	for (int i = 0; i < n; i++){
		pthread_create(args[i].thread_id, NULL, fibonacci_thread, (void *)&args[i]);
	}
	// Waiting for each thread to finish its execution
	// and printing the resulting array
	for (int i = 0; i < n; i++){
		pthread_join(* args[i].thread_id, NULL);
		printf("\n");
		printf("Array: %d,", i + 1);
		printf(" from thread: %ld\n", * (long *)args[i].thread_id);
		for(int j = 0; j < args[i].size; j++){
			printf("%d ", args[i].res_arr[j]);
		}
		printf("\n");
	}

	return 0;
}
