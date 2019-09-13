#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int  * arr; // array
int n; // size of the array 

void swap (int ind1, int ind2){
	int temp = arr[ind2];
	arr[ind2] = arr[ind1];
	arr[ind1] = temp;
}

void * pseudo_sort (void * arg){
	int * indexes = (int *) arg;
	int b_index = indexes[0];
	int e_index = indexes[1];
	int pivot = arr[e_index];

	// we have one element left
	if (b_index == e_index){
		pthread_exit(NULL);
	}
	// the indexes are out of bounds
	if (b_index > e_index){
		pthread_exit(NULL);
	}

	int left = b_index, right = e_index - 1;
	while (1){
		// left
		while(1){
			if (arr[left] > pivot){
				break;
			}
			left++;
			if (left > e_index){
				
				left--;
				break;
			}
		}
		// right
		while(1){
			if (arr[right] <= pivot){
				break;
			}
			right--;
			if (right < b_index){
				right++;
				break;
			}
		}
		if (left >= right){
			swap(left, e_index);
			break;
		}
		swap(left, right);
		left++; right--;
	}

	printf("Im the thread: %ld\n", (long)pthread_self());
	printf("This is my resulting array:\n");
	for(int i = b_index; i <= e_index; i++){
		printf("%d ", arr[i]);
	}
	printf("\n\n");

	// Threads
	int t1_created = -1, t2_created = -1;
	pthread_t t1, t2;
	// left thread
		int vals1 [2] = {b_index, left - 1};
		t1_created = pthread_create(&t1, NULL, pseudo_sort, (void *)vals1);	
	// right thread
		int vals2 [2] = {left + 1, e_index};
		t2_created = pthread_create(&t2, NULL, pseudo_sort, (void *)vals2);

	if (t1_created == 0){
		pthread_join(t1, NULL);
	}
	if (t2_created == 0){
		pthread_join(t2, NULL);
	}
	
	if (!((b_index == 0) && (e_index == n - 1)))
		pthread_exit(NULL);
}

int main () {
	
	srand(time(NULL));

	printf("Array size: \n");
	scanf("%d", &n);
	fflush(stdin);

	arr = (int *)malloc(sizeof(int)*n); // allocating memory for the array

	for (int i = 0; i < n; i++){
		arr[i] = rand() % 100 + 1; // random number between 1 and 100
	}

	printf("BEFORE:\n\n");
	for (int i = 0; i < n; i++){
		printf("%d ", arr[i]);
	}
	printf("\n\n");
	int args [2] = {0, n - 1};
	pseudo_sort((void *)args);
	printf("\nAFTER:\n\n");
	for (int i = 0; i < n; i++){
		printf("%d ", arr[i]);
	}printf("\n");

	// is the array sorted?
	/*for (int i = 0; i < n - 1; i++){
		if(arr[i] > arr[i+1])
			printf("ERROR");
			break;
	}*/
}