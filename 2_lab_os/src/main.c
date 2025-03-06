#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct {
    int *array;
    int start;
    int length;
    int dir; //1-по возрастанию, 0-по убыванию
    int max_threads;
    pthread_mutex_t *mutex;
    int *current_threads;
} ThData;


void swap(int *x1, int *x2) {
    int x3 = *x1;
    *x1 = *x2;
    *x2 = x3;
}


void bitonic_merge(int *array, int start, int length, int dir) {
    if (length <= 1) return;
    int half = length / 2;
    
    for (int i = start; i < start + half; i++) {
        if ((dir == 1 && array[i] > array[i + half]) ||
            (dir == 0 && array[i] < array[i + half])) {
            swap(&array[i], &array[i + half]);
        }
    }

    bitonic_merge(array, start, half, dir);
    bitonic_merge(array, start + half, half, dir);
}


void *bitonic_sort(void *arr) {
    //sleep(1);
    ThData *data = (ThData*)arr;

    if (data->length <= 1) return NULL;

    int half = data->length / 2;
    pthread_t threads[2] = {0};
    ThData data_[2];

    for (int i = 0; i < 2; i++) {
        data_[i] = (ThData) {
            .array = data->array,
            .start = data->start + i * half,
            .length = half,
            .dir = (i == 0) ? 1 : 0,
            .max_threads = data->max_threads,
            .mutex = data->mutex,
            .current_threads = data->current_threads
        };

        pthread_mutex_lock(data->mutex);
        if (*data->current_threads < data->max_threads) {
            (*data->current_threads)++;
            pthread_mutex_unlock(data->mutex);

            pthread_create(&threads[i], NULL, bitonic_sort, &data_[i]);
        } else {
            pthread_mutex_unlock(data->mutex);
            bitonic_sort(&data_[i]);
        }
    }
    for (int i = 0; i < 2; i++) {
        if (threads[i]) {
            pthread_join(threads[i], NULL);

            pthread_mutex_lock(data->mutex);
            (*data->current_threads)--;
            pthread_mutex_unlock(data->mutex);
        }
    }

    bitonic_merge(data->array, data->start, data->length, data->dir);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Use: 'name' 'size' 'max_threads'\n");
        return 1;
    }
    int size_ = atoi(argv[1]);
    int max_threads = atoi(argv[2]);

    if (size_ <= 0 || max_threads <= 0) {
        printf("Error - size and max_threads must be positive\n");
        return 1;
    }

    int *array = malloc(size_ * sizeof(int));
    for (int i = 0; i < size_; i++) {
        array[i] = rand() % 100;
    }
    printf("Random array:\n");
    for (int i = 0; i < size_; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    int current_threads = 0;

    ThData data = {
        .array = array,
        .start = 0,
        .length = size_,
        .dir = 1,
        .max_threads = max_threads,
        .mutex = &mutex,
        .current_threads = &current_threads
    };

    bitonic_sort(&data);

    printf("Sorted array:\n");
    for (int i = 0; i < size_; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    free(array);
    return 0;
}